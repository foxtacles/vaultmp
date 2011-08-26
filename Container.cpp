#include "Container.h"
#include "Items.h"

bool Container::initialized = false;
int Container::game = 0;
ItemDatabase* Container::Items = NULL;
IndexLookup Container::Mods;

#ifdef VAULTMP_DEBUG
Debug* Container::debug;
#endif

#ifdef VAULTMP_DEBUG
void Container::SetDebugHandler(Debug* debug)
{
    Container::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Container class", true);
}
#endif

Container::Container(unsigned int refID, unsigned int baseID) : Object(refID, baseID)
{

}

Container::~Container()
{
    this->FlushContainer();
}

unsigned int Container::ResolveIndex(unsigned int baseID)
{
    unsigned char idx = (unsigned char) (((unsigned int) (baseID & 0xFF000000)) >> 24);
    IndexLookup::iterator it = Mods.find(idx);

    if (it != Mods.end())
        return (baseID & 0x00FFFFFF) | (((unsigned int) it->second) << 24);

    return baseID;
}

void Container::Initialize(int game)
{
    if (!initialized)
    {
        Container::game = game;

        switch (game)
        {
        case FALLOUT3:
            Items = &Fallout3Items;
            break;
        case NEWVEGAS:
            Items = &FalloutNVItems;
            break;
        case OBLIVION:
            Items = &OblivionItems;
            break;
        default:
            throw VaultException("Bad game ID %08X", game);
        }

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Found %d items in the database", true, Items->size());
#endif

        initialized = true;
    }
}

void Container::Cleanup()
{
    if (initialized)
    {
        initialized = false;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->Print("Performed cleanup on Container class", true);
#endif
    }
}

void Container::RegisterIndex(unsigned char real, unsigned char idx)
{

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Registered Fallout mod index (%X => %X)", true, real, idx);
#endif
}

/*
bool Container::CreateDiff(Container* inv1, Container* inv2, Container* diff)
{
    if (inv1 == NULL || inv2 == NULL || diff == NULL)
        return false;

    diff->FlushContainer();

    list<Item*> inv1_list = inv1->GetItemList();
    list<Item*> inv2_list = inv2->GetItemList();

    list<Item*>::iterator it;

    for (it = inv1_list.begin(); it != inv1_list.end(); it = inv1_list.erase(it))
    {
        Item* list1_cur = (*it);
        Item* list2_find = Container::FindItem(inv2_list, list1_cur->item);

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
    {
        Item* list2_cur = (*it);

        Item* diff_new = new Item();
        diff_new->item = list2_cur->item;
        diff_new->type = list2_cur->type;
        diff_new->count = list2_cur->count;
        diff_new->condition = list2_cur->condition;
        diff_new->worn = list2_cur->worn;

        diff->AddItem(diff_new);
    }

    return true;
}
*/

bool Container::IsEmpty()
{
    return container.empty();
}

void Container::FlushContainer()
{
    list<Item*>::iterator it;

    for (it = container.begin(); it != container.end(); ++it)
        delete *it;

    container.clear();
}

void Container::PrintContainer()
{

}

list<Item*> Container::GetItemList()
{
    return container;
}
