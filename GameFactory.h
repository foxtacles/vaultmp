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

    static Reference* GetObject(unsigned char type, NetworkID id);
    static Reference* GetObject(unsigned char type, unsigned int refID);
    static Reference* GetObject(ObjectNetwork object);
    static Reference* GetObject(ObjectReference object);
    static vector<Reference*> GetMultiple(const vector<ObjectNetwork>& objects);
    static vector<Reference*> GetMultiple(const vector<ObjectReference>& objects);
    static NetworkID LookupNetworkID(unsigned int refID);
    static unsigned int LookupRefID(NetworkID id);
    static unsigned char GetType(Reference* reference);
    static vector<Reference*> GetObjectTypes(unsigned char type);
    static void LeaveReference(Reference* reference);
    static void LeaveReference(const vector<Reference*>& reference);

    static NetworkID CreateInstance(unsigned char type, unsigned int refID, unsigned int baseID);
    static NetworkID CreateInstance(unsigned char type, unsigned int baseID);
    static void CreateKnownInstance(unsigned char type, NetworkID id, unsigned int refID, unsigned int baseID);
    static void CreateKnownInstance(unsigned char type, NetworkID id, unsigned int baseID);

    static void DestroyAllInstances();
    static bool DestroyInstance(NetworkID id);
    static NetworkID DestroyInstance(Reference* reference);

};

#endif
