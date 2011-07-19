#ifndef INVENTORY_H
#define INVENTORY_H

#define VAULTMP_DEBUG

#include <windows.h>
#include <map>
#include <list>

#include "Data.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace Data;
using namespace std;

class Inventory {

      private:
            static const char* Fallout3Items[][2];
            static const char* FalloutNVItems[][2];

            #ifdef VAULTMP_DEBUG
            static Debug* debug;
            #endif

            static bool initialized;
            static bool NewVegas;

            static map<const char*, const char*, str_compare> Item_map;
            static map<string, string> Mod_map;

            static char* ResolveIndex(char* key, char* store = NULL);

            void AddItem(Item* item);
            void RemoveItem(Item* item);
            Item* FindItem(map<const char*, const char*, str_compare>::iterator it);
            static Item* FindItem(list<Item*> container, map<const char*, const char*, str_compare>::iterator it);

            list<Item*> container;
            static Inventory* internal;

      public:
            Inventory();
            ~Inventory();

            static void Initialize(bool NewVegas);
            static void Cleanup();
            static void RegisterIndex(string mod, string idx);

            #ifdef VAULTMP_DEBUG
            static void SetDebugHandler(Debug* debug);
            #endif

            static Inventory* TransferInventory();
            static bool CreateDiff(Inventory* inv1, Inventory* inv2, Inventory* diff);

            static bool AddItem_Internal(string baseID, int count, int type = 0, float condition = 0.00, bool worn = false);
            bool AddItem(string baseID, int count, int type = 0, float condition = 0.00, bool worn = false);
            bool RemoveItem(string baseID, int count);
            bool UpdateItem(string baseID, float condition, bool worn);

            void FlushInventory();
            bool IsEmpty();
            Inventory* Copy(Inventory* copy = NULL);
            list<Item*> GetItemList();

            void PrintInventory();

};

#endif
