#include "Item.h"
#include "Container.h"

#ifdef VAULTMP_DEBUG
Debug* Item::debug;
#endif

#ifdef VAULTMP_DEBUG
void Item::SetDebugHandler(Debug* debug)
{
    Item::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Item class", true);
}
#endif

Item::Item(unsigned int refID, unsigned int baseID) : Object(refID, baseID)
{
    data = Container::Items->find(baseID);

    if (data == Container::Items->end())
        data = Container::Items->find(Reference::ResolveIndex(baseID));

    if (data != Container::Items->end())
        this->SetName(string(data->second));
}

Item::~Item()
{

}

unsigned int Item::GetItemCount() const
{
    return this->item_Count.Get();
}

double Item::GetItemCondition() const
{
    return this->item_Condition.Get();
}

bool Item::GetItemEquipped() const
{
    return this->state_Equipped.Get();
}

bool Item::SetItemCount(unsigned int count)
{
    if (this->item_Count.Get() == count)
        return false;

    if (!this->item_Count.Set(count))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Item count was set to %d (ref: %08X)", true, count, this->GetReference());
#endif
}

bool Item::SetItemCondition(double condition)
{
    if (this->item_Condition.Get() == condition)
        return false;

    if (!this->item_Condition.Set(condition))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Item condition was set to %f (ref: %08X)", true, (float) condition, this->GetReference());
#endif
}

bool Item::SetItemEquipped(bool state)
{
    if (this->state_Equipped.Get() == state)
        return false;

    if (!this->state_Equipped.Set(state))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Item equipped state was set to %d (ref: %08X)", true, (int) state, this->GetReference());
#endif

    return true;
}

NetworkID Item::Copy() const
{
    FactoryObject reference = GameFactory::GetObject(GameFactory::CreateInstance(ID_ITEM, 0x00000000, this->GetBase()));
    Item* item = vaultcast<Item>(reference);

    item->SetItemCount(this->GetItemCount());
    item->SetItemCondition(this->GetItemCondition());
    item->SetItemEquipped(this->GetItemEquipped());

    return item->GetNetworkID();
}
