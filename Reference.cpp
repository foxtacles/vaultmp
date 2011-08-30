#include "Reference.h"

Reference::Reference(unsigned int refID, unsigned int baseID)
{
    this->refID.Set(refID);
    this->baseID.Set(baseID);
    this->SetNetworkIDManager(Network::Manager());
}

Reference::~Reference()
{
    Finalize();
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

unsigned int Reference::GetReference()
{
    return refID.Get();
}

unsigned int Reference::GetBase()
{
    return baseID.Get();
}

Parameter Reference::GetReferenceParam()
{
    return Parameter(vector<string>{Utils::LongToHex(refID.Get())}, &Data::EmptyVector);
}

Parameter Reference::GetBaseParam()
{
    return Parameter(vector<string>{Utils::LongToHex(baseID.Get())}, &Data::EmptyVector);
}
