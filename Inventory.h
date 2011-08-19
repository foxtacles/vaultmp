#ifndef INVENTORY_H
#define INVENTORY_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <map>
#include <list>
#include <vector>
#include <stdlib.h>

#include "vaultmp.h"
#include "Data.h"
#include "Reference.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace Data;
using namespace std;

class Inventory : public Reference
{

private:
    static const char* Fallout3Items[][2];
    static const char* FalloutNVItems[][2];

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    static bool initialized;
    static int game;
    static Inventory* internal;
    static map<const char*, const char*, str_compare> Item_map;
    static map<string, string> Mod_map;
    static char* ResolveIndex(char* key, char* store = NULL);
    static Item* FindItem(list<Item*> container, map<const char*, const char*, str_compare>::iterator it);

    void AddItem(Item* item);
    Item* FindItem(map<const char*, const char*, str_compare>::iterator it);

    list<Item*> container;

public:
    Inventory();
    Inventory(string ref, string base);
    virtual ~Inventory();

    static void Initialize(int game);
    static void Cleanup();
    static void RegisterIndex(string mod, string idx);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    static Inventory* TransferInventory(Inventory* dest);
    static map<const char*, const char*, str_compare>::iterator GetItemReference(string baseID);
    static bool CreateDiff(Inventory* inv1, Inventory* inv2, Inventory* diff);
    static bool AddItem_Internal(string baseID, int count, int type = 0, float condition = 0.00, bool worn = false);
    static Parameter GetItemBaseParam(Item* item);
    static Parameter GetItemCountParam(Item* item);

    bool AddItem(string baseID, int count, int type = 0, float condition = 0.00, bool worn = false);
    bool RemoveItem(string baseID, int count);
    bool UpdateItem(string baseID, float condition, bool worn);
    bool IsEmpty();
    void FlushInventory();
    void PrintInventory();
    Inventory* Copy(Inventory* copy = NULL);
    list<Item*> GetItemList();
    list<ParamList> GetItemParamList_AddItem(bool hidden);
    list<ParamList> GetItemParamList_EquipItem(bool unequip, bool hidden);

};

#endif
