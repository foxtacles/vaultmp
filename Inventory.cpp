#include "Inventory.h"
#include "Items.h"

#ifdef VAULTMP_DEBUG
Debug* Inventory::debug;
#endif

bool Inventory::initialized = false;
bool Inventory::NewVegas = false;

map<const char*, const char*, str_compare> Inventory::Item_map;
map<string, string> Inventory::Mod_map;
Inventory* Inventory::internal = NULL;

Inventory::Inventory()
{

}

Inventory::~Inventory()
{
    this->FlushInventory();
}

char* Inventory::ResolveIndex(char* key, char* store)
{
    if (*key == 0x30 && *(key + 1) == 0x30)
    {
        if (store != NULL)
            strcpy(store, key);
        return key;
    }

    char idx[] = {*key, *(key + 1), 0x00};

    map<string, string>::iterator it;
    it = Mod_map.find(string(idx));

    if (it == Mod_map.end())
        return NULL;

    if (store != NULL)
    {
        strcpy(store, key);
        it->second.copy(store, 2);
        return store;
    }

    return key;
}

void Inventory::AddItem(Item* item)
{
    container.push_back(item);
}

Item* Inventory::FindItem(map<const char*, const char*, str_compare>::iterator it)
{
    return FindItem(this->container, it);
}

Item* Inventory::FindItem(list<Item*> container, map<const char*, const char*, str_compare>::iterator it)
{
    list<Item*>::iterator it2;

    for (it2 = container.begin(); it2 != container.end(); ++it2)
    {
        if ((*it2)->item == it)
            return (*it2);
    }

    return NULL;
}

void Inventory::Initialize(bool NewVegas)
{
    if (!initialized)
    {
        Inventory::NewVegas = NewVegas;
        Inventory::internal = new Inventory();

        switch (NewVegas)
        {
        case true:
            for (int i = 0; FalloutNVItems[i][0] != NULL; i++)
                if (ResolveIndex(const_cast<char*>(FalloutNVItems[i][1])) != NULL && strlen(FalloutNVItems[i][1]) == 8)
                    Item_map.insert(pair<const char*, const char*>(FalloutNVItems[i][1], FalloutNVItems[i][0]));
            break;
        case false:
            for (int i = 0; Fallout3Items[i][0] != NULL; i++)
                if (ResolveIndex(const_cast<char*>(Fallout3Items[i][1])) != NULL && strlen(Fallout3Items[i][1]) == 8)
                    Item_map.insert(pair<const char*, const char*>(Fallout3Items[i][1], Fallout3Items[i][0]));
            break;
        }

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            snprintf(text, sizeof(text), "Successfully registered %d Fallout items", Item_map.size());
            debug->Print(text, true);
            snprintf(text, sizeof(text), "%d items could not be resolved or have an invalid format", (NewVegas ? (sizeof(FalloutNVItems) / 8) : (sizeof(Fallout3Items) / 8)) - Item_map.size());
            debug->Print(text, true);
        }
#endif

        initialized = true;
    }
}

void Inventory::Cleanup()
{
    if (initialized)
    {
        if (internal != NULL)
            delete internal;

        Mod_map.clear();
        Item_map.clear();

        initialized = false;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->Print((char*) "Performed cleanup on Inventory class", true);
#endif
    }
}

void Inventory::RegisterIndex(string mod, string idx)
{
    Mod_map.insert(pair<string, string>(mod, idx));
    Mod_map.insert(pair<string, string>(idx, mod));

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Registered Fallout mod index (%s => %s)", mod.c_str(), idx.c_str());
        debug->Print(text, true);
    }
#endif
}

#ifdef VAULTMP_DEBUG
void Inventory::SetDebugHandler(Debug* debug)
{
    Inventory::debug = debug;

    if (debug != NULL)
        debug->Print((char*) "Attached debug handler to Inventory class", true);
}
#endif

Inventory* Inventory::TransferInventory(Inventory* dest)
{
    if (dest == NULL)
        return NULL;

    internal->Copy(dest);
    delete internal;
    internal = new Inventory();
    return dest;
}

map<const char*, const char*, str_compare>::iterator Inventory::GetItemReference(string baseID)
{
    char baseID_real[16];

    map<const char*, const char*, str_compare>::iterator it;
    it = Item_map.find(baseID.c_str());

    if (it == Item_map.end())
        it = (Item_map.find(ResolveIndex(const_cast<char*>(baseID.c_str()), baseID_real)));

    return it;
}

bool Inventory::CreateDiff(Inventory* inv1, Inventory* inv2, Inventory* diff)
{
    if (inv1 == NULL || inv2 == NULL || diff == NULL)
        return false;

    diff->FlushInventory();

    list<Item*> inv1_list = inv1->GetItemList();
    list<Item*> inv2_list = inv2->GetItemList();

    list<Item*>::iterator it;

    for (it = inv1_list.begin(); it != inv1_list.end(); it = inv1_list.erase(it))
    {
        Item* list1_cur = (*it);
        Item* list2_find = Inventory::FindItem(inv2_list, list1_cur->item);

        Item* diff_new = new Item();
        diff_new->item = list1_cur->item;
        diff_new->type = list1_cur->type;

        if (list2_find == NULL)
        {
            diff_new->count = (list1_cur->count) * -1;
            diff_new->condition = list1_cur->condition;
            diff_new->worn = list1_cur->worn;
        }
        else
        {
            diff_new->count = list2_find->count - list1_cur->count;
            diff_new->condition = list2_find->condition;
            diff_new->worn = list2_find->worn;

            inv2_list.remove(list2_find);

            if (diff_new->count == 0 && diff_new->condition == list1_cur->condition && diff_new->worn == list1_cur->worn)
            {
                delete diff_new;
                continue;
            }
        }

        diff->AddItem(diff_new);
    }

    for (it = inv2_list.begin(); it != inv2_list.end(); it = inv2_list.erase(it))
        diff->AddItem(*it);

    return true;
}

bool Inventory::AddItem_Internal(string baseID, int count, int type, float condition, bool worn)
{
    return Inventory::internal->AddItem(baseID, count, type, condition, worn);
}

bool Inventory::AddItem(string baseID, int count, int type, float condition, bool worn)
{
    map<const char*, const char*, str_compare>::iterator it;
    it = GetItemReference(baseID);

    if (it != Item_map.end())
    {
        Item* item = new Item();
        item->item = it;
        item->count = count;
        item->type = type;
        item->condition = condition;
        item->worn = worn;

        this->container.push_back(item);

#ifdef VAULTMP_DEBUG
        if (debug != NULL && this != internal)
        {
            char text[128];
            snprintf(text, sizeof(text), "Added item \"%s\" (%s, count: %d, condition: %f, worn: %d) to inventory %08x", it->second, it->first, count, condition, (int) worn, this);
            debug->Print(text, true);
        }
#endif

        return true;
    }
#ifdef VAULTMP_DEBUG
    else if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Item is not known to vaultmp (%s)", baseID.c_str());
        debug->Print(text, true);
    }
#endif

    return false;
}

bool Inventory::RemoveItem(string baseID, int count)
{
    char baseID_real[16];

    map<const char*, const char*, str_compare>::iterator it;
    it = Item_map.find(baseID.c_str());

    if (it == Item_map.end())
        it = (Item_map.find(ResolveIndex(const_cast<char*>(baseID.c_str()), baseID_real)));

    if (it != Item_map.end())
    {
        int s_count = count;

        while (count != 0)
        {
            Item* find = FindItem(it);

            if (find == NULL)
                return false;

            if (find->count - count <= 0)
            {
                count = abs(find->count - count);
                delete find;
                container.remove(find);
            }
            else
            {
                find->count -= count;
                count = 0;
            }
        }

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            snprintf(text, sizeof(text), "Removed %d items \"%s\" (%s) from inventory %08x", s_count, it->second, it->first, this);
            debug->Print(text, true);
        }
#endif

        return true;
    }
#ifdef VAULTMP_DEBUG
    else if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Item is not known to vaultmp (%s)", baseID.c_str());
        debug->Print(text, true);
    }
#endif

    return false;
}

bool Inventory::UpdateItem(string baseID, float condition, bool worn)
{
    char baseID_real[16];

    map<const char*, const char*, str_compare>::iterator it;
    it = Item_map.find(baseID.c_str());

    if (it == Item_map.end())
        it = (Item_map.find(ResolveIndex(const_cast<char*>(baseID.c_str()), baseID_real)));

    if (it != Item_map.end())
    {
        Item* find = FindItem(it);

        if (find == NULL)
            return false;

        find->condition = condition;
        find->worn = worn;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            snprintf(text, sizeof(text), "Updated item \"%s\" (%s, condition: %f, worn: %d) in inventory %08x", it->second, it->first, condition, (int) worn, this);
            debug->Print(text, true);
        }
#endif

        return true;
    }
#ifdef VAULTMP_DEBUG
    else if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Item is not known to vaultmp (%s)", baseID.c_str());
        debug->Print(text, true);
    }
#endif

    return false;
}

void Inventory::FlushInventory()
{
    list<Item*>::iterator it;

    for (it = container.begin(); it != container.end(); ++it)
        delete *it;

    container.clear();
}

bool Inventory::IsEmpty()
{
    return container.empty();
}

Inventory* Inventory::Copy(Inventory* copy)
{
    if (copy == NULL)
        copy = new Inventory();

    copy->FlushInventory();

    list<Item*>::iterator it;

    for (it = container.begin(); it != container.end(); ++it)
    {
        Item* item_copy = (*it);
        Item* item_new = new Item();

        item_new->item = item_copy->item;
        item_new->type = item_copy->type;
        item_new->count = item_copy->count;
        item_new->condition = item_copy->condition;
        item_new->worn = item_copy->worn;

        copy->AddItem(item_new);
    }

    return copy;
}

Parameter Inventory::GetItemBaseParam(Item* item)
{
    vector<string> itemdat;
    itemdat.push_back(string(item->item->first));
    Parameter Param_Item = Parameter(itemdat, &Data::EmptyVector);
    return Param_Item;
}

Parameter Inventory::GetItemCountParam(Item* item)
{
    vector<string> itemdat;
    char count[8];
    snprintf(count, sizeof(count), "%d", item->count);
    itemdat.push_back(string(count));
    Parameter Param_Item = Parameter(itemdat, &Data::EmptyVector);
    return Param_Item;
}

list<ParamList> Inventory::GetItemParamList_AddItem(bool hidden)
{
    list<ParamList> params;
    list<Item*>::iterator it;

    for (it = container.begin(); it != container.end(); ++it)
    {
        Item* item = *it;

        ParamList param_Item;
        param_Item.push_back(GetItemBaseParam(item));
        param_Item.push_back(GetItemCountParam(item));
        param_Item.push_back(hidden ? Data::Param_True : Data::Param_False);
        params.push_back(param_Item);
    }

    return params;
}

list<ParamList> Inventory::GetItemParamList_EquipItem(bool unequip, bool hidden)
{
    list<ParamList> params;
    list<Item*>::iterator it;

    for (it = container.begin(); it != container.end(); ++it)
    {
        Item* item = *it;

        if (!item->worn)
            continue;

        ParamList param_Item;
        param_Item.push_back(GetItemBaseParam(item));
        param_Item.push_back(unequip ? Data::Param_True : Data::Param_False);
        param_Item.push_back(hidden ? Data::Param_True : Data::Param_False);
        params.push_back(param_Item);
    }

    return params;
}

list<Item*> Inventory::GetItemList()
{
    return container;
}

void Inventory::PrintInventory()
{
#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        list<Item*>::iterator it;

        char text[128];
        snprintf(text, sizeof(text), "Inventory %08x contains %d items:", this, container.size());
        debug->Print(text, true);

        for (it = container.begin(); it != container.end(); ++it)
        {
            snprintf(text, sizeof(text), "Item \"%s\" (%s), count %d, type %d, condition %f, worn %d", (*it)->item->second, (*it)->item->first, (*it)->count, (*it)->type, (*it)->condition, (int) (*it)->worn);
            debug->Print(text, true);
        }
    }
#endif
}
