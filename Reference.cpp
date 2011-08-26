#include "Reference.h"

Reference::Reference(unsigned int refID, unsigned int baseID)
{
    this->refID.Set(refID);
    this->baseID.Set(baseID);
    this->SetNetworkIDManager(Network::Manager());
}

Reference::~Reference()
{

}

void Reference::SetReference(unsigned int refID)
{
    this->refID.Set(refID);
}

void Reference::SetBase(unsigned int baseID)
{
    this->baseID.Set(baseID);
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
