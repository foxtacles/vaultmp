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

}

Item::~Item()
{

}

double Item::GetItemCondition()
{
    return this->item_Condition.Get();
}

Container* Item::GetItemContainer()
{
    return this->item_Container.Get();
}

bool Item::GetItemWorn()
{
    return this->state_Worn.Get();
}

bool Item::SetItemCondition(double condition)
{
    if (this->item_Condition.Get() == condition)
        return false;

    if (!this->item_Condition.Set(condition))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Item condition was set to %Lf (ref: %08X)", true, condition, this->GetReference());
#endif
}

bool Item::SetItemContainer(Container* container)
{
    if (this->item_Container.Get() == container)
        return false;

    if (!this->item_Container.Set(container))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Item container was set to %s (ref: %08X)", true, container->GetName().c_str(), this->GetReference());
#endif
}

bool Item::SetItemWorn(bool worn)
{
    if (this->state_Worn.Get() == worn)
        return false;

    if (!this->state_Worn.Set(worn))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Item worn state was set to %d (ref: %08X)", true, (int) worn, this->GetReference());
#endif

    return true;
}
