#ifndef GAMEFACTORY_H
#define GAMEFACTORY_H

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

#include <map>
#include <list>
#include <typeinfo>

const unsigned char ID_REFERENCE = 0x01;
const unsigned char ID_OBJECT = ID_REFERENCE << 1;
const unsigned char ID_ITEM = ID_OBJECT << 1;
const unsigned char ID_CONTAINER = ID_ITEM << 1;
const unsigned char ID_ACTOR = ID_CONTAINER << 1;
const unsigned char ID_PLAYER = ID_ACTOR << 1;

using namespace std;

class GameFactory
{
friend class Reference;
friend class Object;
friend class Item;
friend class Container;
friend class Actor;
friend class Player;

private:
    GameFactory();

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    static multimap<unsigned char, Reference*> instances;
    static list<Reference*> GetObjectTypes(unsigned char type);

public:

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    static Reference* GetObject(NetworkID id);
    static Reference* GetObject(unsigned int refID);

    static Reference* CreateInstance(unsigned char type, unsigned int refID, unsigned int baseID);
    static Reference* CreateInstance(unsigned char type, unsigned int baseID);

    static void DestroyAllInstances();
    static bool DestroyInstance(NetworkID id);
    static NetworkID DestroyInstance(Reference* reference);

};

#endif
