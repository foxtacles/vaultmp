#ifndef GAMEFACTORY_H
#define GAMEFACTORY_H

#include <map>
#include <list>
#include <typeinfo>

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

typedef map<Reference*, unsigned char> ReferenceList;

using namespace std;

class FactoryObject;

/**
 * \brief Create, use and destroy game object instances via the GameFactory
 */

class GameFactory
{
private:
    GameFactory();

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    static CriticalSection cs;
    static ReferenceList instances;

public:

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    /**
     * \brief Obtains a lock on a Reference
     *
     * The Reference is identified by a NetworkID
     */
    static FactoryObject GetObject(NetworkID id);
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
    static vector<FactoryObject> GetMultiple(const vector<unsigned int>& objects);
    /**
     * \brief Obtains a lock on multiple References
     *
     * The References are identified by a STL vector of NetworkID. You must use this function if you want to obtain multiple locks.
     * Returns a STL vector which contains the locked References in the same ordering as the input vector.
     */
    static vector<FactoryObject> GetMultiple(const vector<NetworkID>& objects);
    /**
     * \brief Lookup a NetworkID
     */
    static NetworkID LookupNetworkID(unsigned int refID);
    /**
     * \brief Lookup a reference ID
     */
    static unsigned int LookupRefID(NetworkID id);
    /**
     * \brief Returns the type of the given Reference
     */
    static unsigned char GetType(Reference* reference);
    /**
     * \brief Obtains a lock on all References of a given type
     */
    static vector<FactoryObject> GetObjectTypes(unsigned char type);
    /**
     * \brief Invalidates a Reference held by a FactoryObject
     */
    static void LeaveReference(FactoryObject& reference);
    /**
     * \brief Creates a new instance of a given type
     */
    static NetworkID CreateInstance(unsigned char type, unsigned int refID, unsigned int baseID);
    /**
     * \brief Creates a new instance of a given type
     */
    static NetworkID CreateInstance(unsigned char type, unsigned int baseID);
    /**
     * \brief Creates a known instance of a given type
     */
    static void CreateKnownInstance(unsigned char type, NetworkID id, unsigned int refID, unsigned int baseID);
    /**
     * \brief Creates a known instance of a given type
     */
    static void CreateKnownInstance(unsigned char type, NetworkID id, unsigned int baseID);

    /**
     * \brief Destroys all instances and cleans up type classes
     */
    static void DestroyAllInstances();
    /**
     * \brief Destroys an instance
     */
    static bool DestroyInstance(NetworkID id);
    /**
     * \brief Destroys an instance which has previously been locked
     *
     * You must make sure the lock count of the given Reference equals to one
     */
    static NetworkID DestroyInstance(FactoryObject& reference);
};

/**
  * \brief Holds an instance pointer
  */

class FactoryObject
{
friend class GameFactory;

private:
    FactoryObject(Reference* reference) : reference(reference) { if (!(reference = (Reference*) reference->StartSession())) throw VaultException("Unknown object %08X", reference); };
    Reference* reference;

public:
    FactoryObject() : reference(NULL) {};
    ~FactoryObject() { if (reference) reference->EndSession(); };
    FactoryObject(FactoryObject const& p) : reference(p.reference) { if (reference) reference->StartSession(); };
    FactoryObject& operator= (FactoryObject const& p)
    {
        if (this != &p)
        {
            if (reference)
                reference->EndSession();
            reference = p.reference;
            if (reference)
                reference->StartSession();
        }
        return *this;
    };

    Reference* operator* () { return reference; }
};

/**
  * \brief Tries to cast an instance pointer
  */
template <typename T>
T* vaultcast(Reference* reference);
template <>
inline Object* vaultcast(Reference* reference) { return reinterpret_cast<Object*>(reference); };
template <>
inline Item* vaultcast(Reference* reference) { if (GameFactory::GetType(reference) & ID_ITEM) return reinterpret_cast<Item*>(reference); else return NULL; };
template <>
inline Container* vaultcast(Reference* reference) { if (GameFactory::GetType(reference) & ALL_CONTAINERS) return reinterpret_cast<Container*>(reference); else return NULL; };
template <>
inline Actor* vaultcast(Reference* reference) { if (GameFactory::GetType(reference) & ALL_ACTORS) return reinterpret_cast<Actor*>(reference); else return NULL; };
template <>
inline Player* vaultcast(Reference* reference) { if (GameFactory::GetType(reference) & ID_PLAYER) return reinterpret_cast<Player*>(reference); else return NULL; };
/**
  * \brief Tries to cast the instance pointer of a FactoryObject
  */
template <typename T>
inline T* vaultcast(FactoryObject& object) { return vaultcast<T>(*object); };

#endif
