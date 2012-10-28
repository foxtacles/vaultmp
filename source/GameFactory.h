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

#ifdef VAULTSERVER
#include "vaultserver/vaultserver.h"
#include "vaultserver/Database.h"
#include "vaultserver/Record.h"
#include "vaultserver/Exterior.h"
#include "vaultserver/Weapon.h"
#include "vaultserver/Race.h"
#include "vaultserver/NPC.h"
#include "vaultserver/BaseContainer.h"
#endif

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

const unsigned char ID_REFERENCE        = 0x01;
const unsigned char ID_OBJECT           = ID_REFERENCE << 1;
const unsigned char ID_ITEM             = ID_OBJECT << 1;
const unsigned char ID_CONTAINER        = ID_ITEM << 1;
const unsigned char ID_ACTOR            = ID_CONTAINER << 1;
const unsigned char ID_PLAYER           = ID_ACTOR << 1;

const unsigned char ALL_OBJECTS         = (ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER);
const unsigned char ALL_CONTAINERS      = (ID_CONTAINER | ID_ACTOR | ID_PLAYER);
const unsigned char ALL_ACTORS          = (ID_ACTOR | ID_PLAYER);

typedef std::map<std::shared_ptr<Reference>, unsigned char> ReferenceList;
typedef std::unordered_map<unsigned char, unsigned int> ReferenceCount;

template<typename T>
class FactoryObject;

/**
 * \brief Create, use and destroy game object instances via the GameFactory
 */
class GameFactory
{
	private:
		GameFactory() = delete;

#ifdef VAULTMP_DEBUG
		static DebugInput<GameFactory> debug;
#endif

		static CriticalSection cs;
		static ReferenceList instances;
		static ReferenceCount typecount;
		static unsigned char game;
		static bool changed;

#ifdef VAULTSERVER
		static Database<Record> dbRecords;
		static Database<Exterior> dbExteriors;
		static Database<Weapon> dbWeapons;
		static Database<Race> dbRaces;
		static Database<NPC> dbNpcs;
		static Database<BaseContainer> dbContainers;
#endif

		static inline ReferenceList::iterator GetShared(const Reference* reference) { return find_if(instances.begin(), instances.end(), [=](const ReferenceList::value_type& _reference) { return _reference.first.get() == reference; }); }

	public:
		static void Initialize(unsigned char game);

		/**
		 * \brief Obtains a lock on a Reference
		 *
		 * The Reference is identified by a NetworkID
		 */
		template<typename T = Object>
		static FactoryObject<T> GetObject(RakNet::NetworkID id);
		/**
		 * \brief Obtains a lock on a Reference
		 *
		 * The Reference is identified by a reference ID
		 */
		template<typename T = Object>
		static FactoryObject<T> GetObject(unsigned int refID);
		/**
		 * \brief Obtains a lock on multiple References
		 *
		 * The References are identified by a STL vector of reference IDs. You must use this function if you want to obtain multiple locks.
		 * Returns a STL vector which contains the locked References in the same ordering as the input vector.
		 */
		template<typename T = Object>
		static std::vector<FactoryObject<T>> GetMultiple(const std::vector<unsigned int>& objects);
		/**
		 * \brief Obtains a lock on multiple References
		 *
		 * The References are identified by a STL vector of NetworkID. You must use this function if you want to obtain multiple locks.
		 * Returns a STL vector which contains the locked References in the same ordering as the input vector.
		 */
		template<typename T = Object>
		static std::vector<FactoryObject<T>> GetMultiple(const std::vector<RakNet::NetworkID>& objects);
		/**
		 * \brief Lookup a NetworkID
		 */
		static RakNet::NetworkID LookupNetworkID(unsigned int refID);
		/**
		 * \brief Lookup a reference ID
		 */
		static unsigned int LookupRefID(RakNet::NetworkID id);
		/**
		 * \brief Returns the type of the given NetworkID
		 */
		static unsigned char GetType(RakNet::NetworkID id) noexcept;
		/**
		 * \brief Returns the type of the given reference
		 */
		static unsigned char GetType(const Reference* reference) noexcept;
		/**
		 * \brief Returns the type of the given reference ID
		 */
		static unsigned char GetType(unsigned int refID) noexcept;
		/**
		 * \brief Obtains a lock on all References of a given type
		 */
		template<typename T>
		static std::vector<FactoryObject<T>> GetObjectTypes(unsigned char type) noexcept;
		/**
		 * \brief Returns the NetworkID's of all References of a given type
		 */
		static std::vector<RakNet::NetworkID> GetIDObjectTypes(unsigned char type) noexcept;
		/**
		 * \brief Counts the amount of References of a given type
		 */
		static unsigned int GetObjectCount(unsigned char type) noexcept;
		/**
		 * \brief Invalidates a Reference held by a FactoryObject
		 */
		static void LeaveReference(FactoryObject<Object>& reference);
		/**
		 * \brief Creates a new instance of a given type
		 */
		static RakNet::NetworkID CreateInstance(unsigned char type, unsigned int refID, unsigned int baseID);
		/**
		 * \brief Creates a new instance of a given type
		 */
		static RakNet::NetworkID CreateInstance(unsigned char type, unsigned int baseID);
		/**
		 * \brief Creates a known instance of a given type
		 */
		static void CreateKnownInstance(unsigned char type, RakNet::NetworkID id, unsigned int refID, unsigned int baseID);
		/**
		 * \brief Creates a known instance of a given type
		 */
		static void CreateKnownInstance(unsigned char type, RakNet::NetworkID id, unsigned int baseID);
		/**
		 * \brief Creates a known instance from a network packet
		 */
		static RakNet::NetworkID CreateKnownInstance(unsigned char type, const pDefault* packet);

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
		static RakNet::NetworkID DestroyInstance(FactoryObject<Object>& reference);

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
		friend FactoryObject<T> vaultcast(const FactoryObject<U>& object, bool = true) noexcept;

	private:
		Reference* reference;
		unsigned char type;

	protected:
		FactoryObject(Reference* reference, unsigned char type) : reference(reference), type(type)
		{
			if (!(reference = reinterpret_cast<Reference*>(reference->StartSession())))
				throw VaultException("Unknown object %08X", reference);
		}

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
		unsigned char GetType() const { return type; }
		Reference* operator->() const { return reference; }
		explicit operator bool() const { return reference; }
		bool operator==(const FactoryObject& p) const { return reference && reference == p.reference; }
		bool operator!=(const FactoryObject& p) const { return !operator==(p); }

		template<typename T>
		inline bool validate(unsigned char type = 0x00) const;
};

template<> inline bool FactoryObject<Reference>::validate<Object>(unsigned char) const { return true; }
template<> class FactoryObject<Object> : public FactoryObject<Reference>
{
		friend class GameFactory;

		template<typename T, typename U>
		friend FactoryObject<T> vaultcast(const FactoryObject<U>& object, bool = true) noexcept;

	protected:
		FactoryObject(Reference* reference, unsigned char type) : FactoryObject<Reference>(reference, type) {}
		template<typename T> FactoryObject(const FactoryObject<T>& p) : FactoryObject<Reference>(p) {}
		template<typename T> FactoryObject& operator=(const FactoryObject<T>& p) { return FactoryObject<Reference>::operator=(p); }

	public:
		FactoryObject() : FactoryObject<Reference>() {}
		FactoryObject(const FactoryObject& p) : FactoryObject<Reference>(p) {}
		FactoryObject& operator=(const FactoryObject&) = default;
		FactoryObject(FactoryObject&& p) : FactoryObject<Reference>(std::move(p)) {}
		FactoryObject& operator=(FactoryObject&&) = default;
		~FactoryObject() = default;

		Object* operator->() const { return static_cast<Object*>(FactoryObject<Reference>::operator->()); }
};

template<> inline bool FactoryObject<Reference>::validate<Item>(unsigned char type) const { return type ? (type & ID_ITEM) : (this->type & ID_ITEM); }
template<> class FactoryObject<Item> : public FactoryObject<Object>
{
		friend class GameFactory;

		template<typename T, typename U>
		friend FactoryObject<T> vaultcast(const FactoryObject<U>& object, bool = true) noexcept;

	protected:
		FactoryObject(Reference* reference, unsigned char type) : FactoryObject<Object>(reference, type)
		{
			if (!validate<Item>())
				throw VaultException("Object %08X is not an Item", reference);
		}
		template<typename T> FactoryObject(const FactoryObject<T>& p) : FactoryObject<Object>(p) {}
		template<typename T> FactoryObject& operator=(const FactoryObject<T>& p) { return FactoryObject<Object>::operator=(p); }

	public:
		FactoryObject() : FactoryObject<Object>() {}
		FactoryObject(const FactoryObject& p) : FactoryObject<Object>(p) {}
		FactoryObject& operator=(const FactoryObject&) = default;
		FactoryObject(FactoryObject&& p) : FactoryObject<Object>(std::move(p)) {}
		FactoryObject& operator=(FactoryObject&&) = default;
		~FactoryObject() = default;

		Item* operator->() const { return static_cast<Item*>(FactoryObject<Reference>::operator->()); }
};

template<> inline bool FactoryObject<Reference>::validate<Container>(unsigned char type) const { return type ? (type & ALL_CONTAINERS) : (this->type & ALL_CONTAINERS); }
template<> class FactoryObject<Container> : public FactoryObject<Object>
{
		friend class GameFactory;

		template<typename T, typename U>
		friend FactoryObject<T> vaultcast(const FactoryObject<U>& object, bool = true) noexcept;

	protected:
		FactoryObject(Reference* reference, unsigned char type) : FactoryObject<Object>(reference, type)
		{
			if (!validate<Container>())
				throw VaultException("Object %08X is not a Container", reference);
		}
		template<typename T> FactoryObject(const FactoryObject<T>& p) : FactoryObject<Object>(p) {}
		template<typename T> FactoryObject& operator=(const FactoryObject<T>& p) { return FactoryObject<Object>::operator=(p); }

	public:
		FactoryObject() : FactoryObject<Object>() {}
		FactoryObject(const FactoryObject& p) : FactoryObject<Object>(p) {}
		FactoryObject& operator=(const FactoryObject&) = default;
		FactoryObject(FactoryObject&& p) : FactoryObject<Object>(std::move(p)) {}
		FactoryObject& operator=(FactoryObject&&) = default;
		~FactoryObject() = default;

		Container* operator->() const { return static_cast<Container*>(FactoryObject<Reference>::operator->()); }
};

template<> inline bool FactoryObject<Reference>::validate<Actor>(unsigned char type) const { return type ? (type & ALL_ACTORS) : (this->type & ALL_ACTORS); }
template<> class FactoryObject<Actor> : public FactoryObject<Container>
{
		friend class GameFactory;

		template<typename T, typename U>
		friend FactoryObject<T> vaultcast(const FactoryObject<U>& object, bool = true) noexcept;

	protected:
		FactoryObject(Reference* reference, unsigned char type) : FactoryObject<Container>(reference, type)
		{
			if (!validate<Actor>())
				throw VaultException("Object %08X is not an Actor", reference);
		}
		template<typename T> FactoryObject(const FactoryObject<T>& p) : FactoryObject<Container>(p) {}
		template<typename T> FactoryObject& operator=(const FactoryObject<T>& p) { return FactoryObject<Container>::operator=(p); }

	public:
		FactoryObject() : FactoryObject<Container>() {}
		FactoryObject(const FactoryObject& p) : FactoryObject<Container>(p) {}
		FactoryObject& operator=(const FactoryObject&) = default;
		FactoryObject(FactoryObject&& p) : FactoryObject<Container>(std::move(p)) {}
		FactoryObject& operator=(FactoryObject&&) = default;
		~FactoryObject() = default;

		Actor* operator->() const { return static_cast<Actor*>(FactoryObject<Reference>::operator->()); }
};

template<> inline bool FactoryObject<Reference>::validate<Player>(unsigned char type) const { return type ? (type & ID_PLAYER) : (this->type & ID_PLAYER); }
template<> class FactoryObject<Player> : public FactoryObject<Actor>
{
		friend class GameFactory;

		template<typename T, typename U>
		friend FactoryObject<T> vaultcast(const FactoryObject<U>& object, bool = true) noexcept;

	protected:
		FactoryObject(Reference* reference, unsigned char type) : FactoryObject<Actor>(reference, type)
		{
			if (!validate<Player>())
				throw VaultException("Object %08X is not a Player", reference);
		}
		template<typename T> FactoryObject(const FactoryObject<T>& p) : FactoryObject<Actor>(p) {}
		template<typename T> FactoryObject& operator=(const FactoryObject<T>& p) { return FactoryObject<Actor>::operator=(p); }

	public:
		FactoryObject() : FactoryObject<Actor>() {}
		FactoryObject(const FactoryObject& p) : FactoryObject<Actor>(p) {}
		FactoryObject& operator=(const FactoryObject&) = default;
		FactoryObject(FactoryObject&& p) : FactoryObject<Actor>(std::move(p)) {}
		FactoryObject& operator=(FactoryObject&&) = default;
		~FactoryObject() = default;

		Player* operator->() const { return static_cast<Player*>(FactoryObject<Reference>::operator->()); }
};

/**
  * \brief Tries to cast the instance pointer of a FactoryObject
  */
template<typename T, typename U>
inline FactoryObject<T> vaultcast(const FactoryObject<U>& object, bool safe = true) noexcept
{
	bool valid = object.template validate<T>();

	if (!valid)
	{
		if (safe)
#ifdef VAULTMP_DEBUG
			throw VaultException("Object %08X (%s) cannot be casted to %s", object.reference, typeid(U).name(), typeid(T).name());
#else
			throw VaultException("Object vaultcast failed (%08X)", object.reference);
#endif
		else
			return FactoryObject<T>();
	}

	return FactoryObject<T>(object);
}
#endif
