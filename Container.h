#ifndef CONTAINER_H
#define CONTAINER_H

#define TYPECLASS
#include "GameFactory.h"

#include <map>
#include <list>
#include <vector>
#include <stdlib.h>

#include "vaultmp.h"
#include "Data.h"
#include "Item.h"
#include "Object.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace Data;
using namespace std;

class Item;

typedef const map<const unsigned int, const char*> ItemDatabase;
typedef map<const unsigned char, const unsigned char> IndexLookup;

class Container : public Object
{
friend class GameFactory;

private:
    static ItemDatabase Fallout3Items;
    static ItemDatabase FalloutNVItems;
    static ItemDatabase OblivionItems;

    static ItemDatabase* Items;
    static IndexLookup Mods;

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    static bool initialized;
    static int game;
    static unsigned int ResolveIndex(unsigned int baseID);

    list<Item*> container;

protected:
    Container(unsigned int refID, unsigned int baseID);
    virtual ~Container();

public:
    static void Initialize(int game);
    static void Cleanup();
    static void RegisterIndex(unsigned char real, unsigned char idx = 0x00);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    bool IsEmpty();
    void FlushContainer();
    void PrintContainer();
    list<Item*> GetItemList();

};

#endif
