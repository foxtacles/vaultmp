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

const unsigned char ID_REFERENCE        = 0x01;
const unsigned char ID_OBJECT           = ID_REFERENCE << 1;
const unsigned char ID_ITEM             = ID_OBJECT << 1;
const unsigned char ID_CONTAINER        = ID_ITEM << 1;
const unsigned char ID_ACTOR            = ID_CONTAINER << 1;
const unsigned char ID_PLAYER           = ID_ACTOR << 1;

const unsigned char ALL_OBJECTS         = (ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER);
const unsigned char ALL_CONTAINERS      = (ID_CONTAINER | ID_ACTOR | ID_PLAYER);
const unsigned char ALL_ACTORS          = (ID_ACTOR | ID_PLAYER);

typedef pair<unsigned char, NetworkID> ObjectNetwork;
typedef pair<unsigned char, unsigned int> ObjectReference;
typedef map<Reference*, unsigned char> ReferenceList;

using namespace std;

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
     * The Reference is identified by a NetworkID and its type.
     */
    static Reference* GetObject(unsigned char type, NetworkID id);
    /**
     * \brief Obtains a lock on a Reference
     *
     * The Reference is identified by a reference ID and its type.
     */
    static Reference* GetObject(unsigned char type, unsigned int refID);
    /**
     * \brief Obtains a lock on a Reference
     *
     * The Reference is identified by a ObjectNetwork, which has the form pair<unsigned char, NetworkID>
     */
    static Reference* GetObject(ObjectNetwork object);
    /**
     * \brief Obtains a lock on a Reference
     *
     * The Reference is identified by a ObjectReference, which has the form pair<unsigned char, unsigned int>
     */
    static Reference* GetObject(ObjectReference object);
    /**
     * \brief Obtains a lock on multiple References
     *
     * The References are identified by a STL vector of ObjectNetwork. You must use this function if you want to obtain multiple locks.
     * Returns a STL vector which contains the locked References in the same ordering as the input vector.
     */
    static vector<Reference*> GetMultiple(const vector<ObjectNetwork>& objects);
    /**
     * \brief Obtains a lock on multiple References
     *
     * The References are identified by a STL vector of ObjectReference. You must use this function if you want to obtain multiple locks.
     * Returns a STL vector which contains the locked References in the same ordering as the input vector.
     */
    static vector<Reference*> GetMultiple(const vector<ObjectReference>& objects);
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
    static vector<Reference*> GetObjectTypes(unsigned char type);
    /**
     * \brief Releases a lock
     *
     * Must be called when you are finished with the Reference you obtained via GetObject.
     */
    static void LeaveReference(Reference* reference);
    /**
     * \brief Releases multiple locks
     *
     * Must be called when you are finished with multiple References you obtained via GetMultiple or GetObjectTypes
     */
    static void LeaveReference(const vector<Reference*>& reference);
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
    static NetworkID DestroyInstance(Reference* reference);

};

#endif
