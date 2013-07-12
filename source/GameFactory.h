#ifndef GAMEFACTORY_H
#define GAMEFACTORY_H

#include <map>
#include <unordered_map>
#include <list>
#include <typeinfo>

#include "RakNet.h"

#include "Reference.h"
#include "Object.h"
#include "Item.h"
#include "Container.h"
#include "Actor.h"
#include "Player.h"
#include "Window.h"
#include "Button.h"
#include "Text.h"
#include "Edit.h"

#ifdef VAULTSERVER
#include "vaultserver/vaultserver.h"
#include "vaultserver/Database.h"
#include "vaultserver/Record.h"
#include "vaultserver/Reference.h"
#include "vaultserver/Exterior.h"
#include "vaultserver/Weapon.h"
#include "vaultserver/Race.h"
#include "vaultserver/NPC.h"
#include "vaultserver/BaseContainer.h"
#include "vaultserver/Item.h"
#include "vaultserver/Terminal.h"
#include "vaultserver/Interior.h"
#endif

#include "Expected.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

const unsigned int ID_REFERENCE        = 0x01;
const unsigned int ID_OBJECT           = ID_REFERENCE << 1;
const unsigned int ID_ITEM             = ID_OBJECT << 1;
const unsigned int ID_CONTAINER        = ID_ITEM << 1;
const unsigned int ID_ACTOR            = ID_CONTAINER << 1;
const unsigned int ID_PLAYER           = ID_ACTOR << 1;
const unsigned int ID_WINDOW           = ID_PLAYER << 1;
const unsigned int ID_BUTTON           = ID_WINDOW << 1;
const unsigned int ID_TEXT             = ID_BUTTON << 1;
const unsigned int ID_EDIT             = ID_TEXT << 1;

const unsigned int ALL_OBJECTS         = (ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER);
const unsigned int ALL_CONTAINERS      = (ID_CONTAINER | ID_ACTOR | ID_PLAYER);
const unsigned int ALL_ACTORS          = (ID_ACTOR | ID_PLAYER);
const unsigned int ALL_GUI             = (ID_WINDOW | ID_BUTTON | ID_TEXT | ID_EDIT);

template<typename T>
class FactoryObject;

/**
 * \brief Create, use and destroy game object instances via the GameFactory
 */
class GameFactory
{
	private:
		typedef std::map<std::shared_ptr<Reference>, unsigned int> ReferenceList;
		typedef std::unordered_map<RakNet::NetworkID, ReferenceList::iterator> ReferenceIndex;
		typedef std::unordered_map<unsigned int, unsigned int> ReferenceCount;
		typedef std::unordered_set<RakNet::NetworkID> ReferenceDeleted;

		GameFactory() = delete;

#ifdef VAULTMP_DEBUG
		static DebugInput<GameFactory> debug;
#endif

		static CriticalSection cs;
		static ReferenceList instances;
		static ReferenceIndex index;
		static ReferenceCount typecount;
		static ReferenceDeleted delrefs;
		static bool changed;

#ifdef VAULTSERVER
		static Database<DB::Record> dbRecords;
		static Database<DB::Reference> dbReferences;
		static Database<DB::Exterior> dbExteriors;
		static Database<DB::Weapon> dbWeapons;
		static Database<DB::Race> dbRaces;
		static Database<DB::NPC> dbNpcs;
		static Database<DB::BaseContainer> dbContainers;
		static Database<DB::Item> dbItems;
		static Database<DB::Terminal> dbTerminals;
		static Database<DB::Interior> dbInteriors;
#endif

		inline static ReferenceList::iterator GetShared(const RakNet::NetworkID& id) { return index.count(id) ? index[id] : instances.end(); }

	public:
		static void Initialize();

		/**
		 * \brief Obtains a lock on a Reference
		 *
		 * The Reference is identified by a NetworkID
		 */
		template<typename T = Object>
		static Expected<FactoryObject<T>> GetObject(RakNet::NetworkID id);
		/**
		 * \brief Obtains a lock on a Reference
		 *
		 * The Reference is identified by a reference ID
		 */
		template<typename T = Object>
		static Expected<FactoryObject<T>> GetObject(unsigned int refID);
		/**
		 * \brief Obtains a lock on multiple References
		 *
		 * The References are identified by a STL vector of reference IDs. You must use this function if you want to obtain multiple locks.
		 * Returns a STL vector which contains the locked References in the same ordering as the input vector.
		 */
		template<typename T = Object>
		static std::vector<Expected<FactoryObject<T>>> GetMultiple(const std::vector<unsigned int>& objects);
		/**
		 * \brief Obtains a lock on multiple References
		 *
		 * The References are identified by a STL vector of NetworkID. You must use this function if you want to obtain multiple locks.
		 * Returns a STL vector which contains the locked References in the same ordering as the input vector.
		 */
		template<typename T = Object>
		static std::vector<Expected<FactoryObject<T>>> GetMultiple(const std::vector<RakNet::NetworkID>& objects);
		/**
		 * \brief Lookup a NetworkID
		 */
		static RakNet::NetworkID LookupNetworkID(unsigned int refID);
		/**
		 * \brief Lookup a reference ID
		 */
		static unsigned int LookupRefID(RakNet::NetworkID id);
		/**
		 * \brief Checks if an ID has been deleted
		 */
		static bool IsDeleted(RakNet::NetworkID id);
		/**
		 * \brief Returns the type of the given NetworkID
		 */
		static unsigned int GetType(RakNet::NetworkID id) noexcept;
		/**
		 * \brief Returns the type of the given reference
		 */
		static unsigned int GetType(const Reference* reference) noexcept;
		/**
		 * \brief Returns the type of the given reference ID
		 */
		static unsigned int GetType(unsigned int refID) noexcept;
		/**
		 * \brief Obtains a lock on all References of a given type
		 */
		template<typename T = Object>
		static std::vector<FactoryObject<T>> GetObjectTypes(unsigned int type) noexcept;
		/**
		 * \brief Returns the NetworkID's of all References of a given type
		 */
		static std::vector<RakNet::NetworkID> GetIDObjectTypes(unsigned int type) noexcept;
		/**
		 * \brief Counts the amount of References of a given type
		 */
		static unsigned int GetObjectCount(unsigned int type) noexcept;
		/**
		 * \brief Invalidates a Reference held by a FactoryObject
		 */
		template<typename T>
		static void LeaveReference(FactoryObject<T>& reference);
		/**
		 * \brief Creates a new instance of a given type
		 */
		static RakNet::NetworkID CreateInstance(unsigned int type, unsigned int refID, unsigned int baseID);
		/**
		 * \brief Creates a new instance of a given type
		 */
		static RakNet::NetworkID CreateInstance(unsigned int type, unsigned int baseID);
		/**
		 * \brief Creates a known instance from a network packet
		 */
		static RakNet::NetworkID CreateKnownInstance(unsigned int type, const pDefault* packet);

		/**
		 * \brief Destroys all instances and cleans up type classes
		 */
		static void DestroyAllInstances();
		/**
		 * \brief Destroys an instance
		 */
		static bool DestroyInstance(RakNet::NetworkID id);
		/**
		 * \brief Destroys an instance which has previously been locked
		 *
		 * You must make sure the lock count of the given Reference equals to one
		 */
		template<typename T>
		static RakNet::NetworkID DestroyInstance(FactoryObject<T>& reference);

		/**
		 * \brief Used to set the changed flag for the next network reference going to be created
		 */
		static void SetChangeFlag(bool changed);
};

/**
  * \brief Holds an instance pointer
  */
template<typename T>
class FactoryObject;

template<>
class FactoryObject<Reference>
{
		friend class GameFactory;

		template<typename T, typename U>
		friend Expected<FactoryObject<T>> vaultcast(const FactoryObject<U>& object) noexcept;

	private:
		Reference* reference;
		unsigned int type;

	protected:
		FactoryObject(Reference* reference, unsigned int type) : reference(reinterpret_cast<Reference*>(reference->StartSession())), type(type) {}

		FactoryObject(const FactoryObject& p) : reference(p.reference), type(p.type)
		{
			if (reference)
				reference->StartSession();
		}
		FactoryObject& operator=(const FactoryObject& p)
		{
			if (this != &p)
			{
				if (reference)
					reference->EndSession();

				reference = p.reference;
				type = p.type;

				if (reference)
					reference->StartSession();
			}

			return *this;
		}

		FactoryObject(FactoryObject&& p) : reference(p.reference), type(p.type)
		{
			p.reference = nullptr;
			p.type = 0x00;
		}
		FactoryObject& operator=(FactoryObject&& p)
		{
			if (this != &p)
			{
				if (reference)
					reference->EndSession();

				reference = p.reference;
				type = p.type;

				p.reference = nullptr;
				p.type = 0x00;
			}

			return *this;
		}

		FactoryObject() : reference(nullptr), type(0x00) {};
		~FactoryObject()
		{
			if (reference)
				reference->EndSession();
		}

	public:
		unsigned int GetType() const { return type; }
		Reference& operator*() const { return *reference; }
		Reference* operator->() const { return reference; }
		explicit operator bool() const { return reference; }
		bool operator==(const FactoryObject& p) const { return reference && reference == p.reference; }
		bool operator!=(const FactoryObject& p) const { return !operator==(p); }

		template<typename T>
		inline bool validate(unsigned int type = 0x00000000) const;
};

#define GF_TYPE_WRAPPER(derived, base, token)                                                                                                               \
	template<> inline bool FactoryObject<Reference>::validate<derived>(unsigned int type) const { return type ? (type & token) : (this->type & token); }   \
	template<> class FactoryObject<derived> : public FactoryObject<base>                                                                                    \
	{                                                                                                                                                       \
		friend class GameFactory;                                                                                                                           \
                                                                                                                                                            \
		template<typename T, typename U>                                                                                                                    \
		friend Expected<FactoryObject<T>> vaultcast(const FactoryObject<U>& object) noexcept;                                                               \
                                                                                                                                                            \
	protected:                                                                                                                                              \
		FactoryObject(Reference* reference, unsigned int type) : FactoryObject<base>(reference, type)                                                      \
		{                                                                                                                                                   \
			if (!validate<derived>())                                                                                                                       \
				reference = nullptr;                                                                                                                        \
		}                                                                                                                                                   \
		template<typename T> FactoryObject(const FactoryObject<T>& p) : FactoryObject<base>(p) {}                                                           \
		template<typename T> FactoryObject& operator=(const FactoryObject<T>& p) { return FactoryObject<base>::operator=(p); }                              \
                                                                                                                                                            \
	public:                                                                                                                                                 \
		FactoryObject() : FactoryObject<base>() {}                                                                                                          \
		FactoryObject(const FactoryObject& p) : FactoryObject<base>(p) {}                                                                                   \
		FactoryObject& operator=(const FactoryObject&) = default;                                                                                           \
		FactoryObject(FactoryObject&& p) : FactoryObject<base>(std::move(p)) {}                                                                             \
		FactoryObject& operator=(FactoryObject&&) = default;                                                                                                \
		~FactoryObject() = default;                                                                                                                         \
                                                                                                                                                            \
		derived* operator->() const { return static_cast<derived*>(FactoryObject<Reference>::operator->()); }                                               \
		derived& operator*() const { return static_cast<derived&>(FactoryObject<Reference>::operator*()); }                                                 \
};

GF_TYPE_WRAPPER(Object, Reference, ALL_OBJECTS)
GF_TYPE_WRAPPER(Item, Object, ID_ITEM)
GF_TYPE_WRAPPER(Container, Object, ID_CONTAINER)
GF_TYPE_WRAPPER(Actor, Container, ALL_ACTORS)
GF_TYPE_WRAPPER(Player, Actor, ID_PLAYER)
GF_TYPE_WRAPPER(Window, Reference, ALL_GUI)
GF_TYPE_WRAPPER(Button, Window, ID_BUTTON)
GF_TYPE_WRAPPER(Text, Window, ID_TEXT)
GF_TYPE_WRAPPER(Edit, Window, ID_EDIT)

#undef GF_TYPE_WRAPPER

/**
  * \brief Tries to cast the instance pointer of a FactoryObject
  */
template<typename T, typename U>
inline Expected<FactoryObject<T>> vaultcast(const FactoryObject<U>& object) noexcept
{
	auto result = FactoryObject<T>(object);

	if (result.template validate<T>())
		return result;

	return VaultException("vaultcast failed");
}
template<typename T, typename U>
inline Expected<FactoryObject<T>> vaultcast(const Expected<FactoryObject<U>>& object) noexcept { return vaultcast<T>(const_cast<Expected<FactoryObject<U>>&>(object).get()); }
#endif
