#ifndef GAMEFACTORY_H
#define GAMEFACTORY_H

#include <map>
#include <unordered_map>
#include <list>
#include <typeinfo>

#include "RakNet.h"

#ifndef TYPECLASS
#include "Object.h"
#include "Item.h"
#include "Container.h"
#include "Actor.h"
#include "Player.h"
#else
#undef TYPECLASS
#endif

#include "Reference.h"

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

class Reference;
class Object;
class Item;
class Container;
class Actor;
class Player;

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
		static FactoryObject GetObject(RakNet::NetworkID id);
		/**
		 * \brief Obtains a lock on a Reference
		 *
		 * The Reference is identified by a reference ID
		 */
		static FactoryObject GetObject(unsigned int refID);
		/**
		 * \brief Obtains a lock on multiple References
		 *
		 * The References are identified by a STL vector of reference IDs. You must use this function if you want to obtain multiple locks.
		 * Returns a STL vector which contains the locked References in the same ordering as the input vector.
		 */
		static std::vector<FactoryObject> GetMultiple(const std::vector<unsigned int>& objects);
		/**
		 * \brief Obtains a lock on multiple References
		 *
		 * The References are identified by a STL vector of NetworkID. You must use this function if you want to obtain multiple locks.
		 * Returns a STL vector which contains the locked References in the same ordering as the input vector.
		 */
		static std::vector<FactoryObject> GetMultiple(const std::vector<RakNet::NetworkID>& objects);
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
		static std::vector<FactoryObject> GetObjectTypes(unsigned char type) noexcept;
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
		static void LeaveReference(FactoryObject& reference);
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
		static RakNet::NetworkID DestroyInstance(FactoryObject& reference);

		/**
		 * \brief Used to set the changed flag for the next network reference going to be created
		 */
		static void SetChangeFlag(bool changed);
};

/**
  * \brief Holds an instance pointer
  */

class FactoryObject
{
		friend class GameFactory;

		template <typename T>
		friend T* vaultcast(const FactoryObject& object) noexcept;

	private:
		Reference* reference;
		unsigned char type;

		FactoryObject(Reference* reference, unsigned char type) : reference(reference), type(type)
		{
			if (!(reference = reinterpret_cast<Reference*>(reference->StartSession())))
				throw VaultException("Unknown object %08X", reference);
		};

	public:
		FactoryObject() : reference(nullptr), type(0x00) {};
		~FactoryObject()
		{
			if (reference)
				reference->EndSession();
		};

		FactoryObject(const FactoryObject&) = delete;
		FactoryObject& operator=(const FactoryObject&) = delete;

		FactoryObject(FactoryObject&& p) : reference(p.reference), type(p.type)
		{
			p.reference = nullptr;
			p.type = 0x00;
		};
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
		};

		unsigned char GetType() const
		{
			return type;
		}

		Reference* operator->() const
		{
			return reference;
		}
};

/**
  * \brief Tries to cast the instance pointer of a FactoryObject
  */
template<typename T>
T* vaultcast(const FactoryObject& object) noexcept;
template<>
inline Object* vaultcast(const FactoryObject& object) noexcept
{
	return reinterpret_cast<Object*>(object.reference);
}
template<>
inline Item* vaultcast(const FactoryObject& object) noexcept
{
	if (object.type & ID_ITEM)
		return reinterpret_cast<Item*>(object.reference);
	else
		return nullptr;
}
template<>
inline Container* vaultcast(const FactoryObject& object) noexcept
{
	if (object.type & ALL_CONTAINERS)
		return reinterpret_cast<Container*>(object.reference);
	else
		return nullptr;
}
template<>
inline Actor* vaultcast(const FactoryObject& object) noexcept
{
	if (object.type & ALL_ACTORS)
		return reinterpret_cast<Actor*>(object.reference);
	else
		return nullptr;
}
template<>
inline Player* vaultcast(const FactoryObject& object) noexcept
{
	if (object.type & ID_PLAYER)
		return reinterpret_cast<Player*>(object.reference);
	else
		return nullptr;
}

#endif
