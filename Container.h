#ifndef CONTAINER_H
#define CONTAINER_H

#define TYPECLASS
#include "GameFactory.h"

#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <cstdlib>

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
typedef list<pair<NetworkID, bool> > ContainerDiff;
typedef pair<NetworkID, map<NetworkID, list<NetworkID> > > StripCopy;

class Container : public Object
{
friend class GameFactory;
friend class Item;

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
    static unsigned char game;
    static unsigned int ResolveIndex(unsigned int baseID);
    static bool Item_sort(NetworkID id, NetworkID id2);

    list<NetworkID> container;

    StripCopy Strip() const;

    Container(const Container&);
    Container& operator=(const Container&);

protected:
    Container(unsigned int refID, unsigned int baseID);
    virtual ~Container();

public:
    static void Initialize(unsigned char game);
    static void Cleanup();
    static void RegisterIndex(unsigned char real, unsigned char idx = 0x00);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    void AddItem(NetworkID id);
    void RemoveItem(NetworkID id);
    ContainerDiff Compare(NetworkID id) const;

    bool IsEmpty() const;
    void PrintContainer() const;
    void FlushContainer();
    const list<NetworkID>& GetItemList() const;

    NetworkID Copy() const;

};

#endif
