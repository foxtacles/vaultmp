#include "Reference.h"

IndexLookup Reference::Mods;

Reference::Reference(unsigned int refID, unsigned int baseID)
{
    this->refID.Set(refID);
    this->baseID.Set(baseID);
    this->SetNetworkIDManager(Network::Manager());
#ifdef VAULTMP_DEBUG
    //this->ToggleSectionDebug(true);
#endif
}

Reference::~Reference()
{

}

unsigned int Reference::ResolveIndex(unsigned int baseID)
{
    unsigned char idx = (unsigned char) (((unsigned int) (baseID & 0xFF000000)) >> 24);
    IndexLookup::iterator it = Mods.find(idx);

    if (it != Mods.end())
        return (baseID & 0x00FFFFFF) | (((unsigned int) it->second) << 24);

    return baseID;
}

Lockable* Reference::SetReference(unsigned int refID)
{
    if (this->refID.Get() == refID)
        return NULL;

    if (!this->refID.Set(refID))
        return NULL;

    return &this->refID;
}

Lockable* Reference::SetBase(unsigned int baseID)
{
    if (this->baseID.Get() == baseID)
        return NULL;

    if (!this->baseID.Set(baseID))
        return NULL;

    return &this->baseID;
}

unsigned int Reference::GetReference() const
{
    return refID.Get();
}

unsigned int Reference::GetBase() const
{
    return baseID.Get();
}

const Parameter Reference::GetReferenceParam() const
{
    return Parameter(vector<string>{Utils::LongToHex(refID.Get())}, NULL);
}

const Parameter Reference::GetBaseParam() const
{
    return Parameter(vector<string>{Utils::LongToHex(baseID.Get())}, NULL);
}
