#include "Reference.h"

IndexLookup Reference::Mods;

Reference::Reference(unsigned int refID, unsigned int baseID)
{
	this->refID.set(refID);
	this->baseID.set(baseID);
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
	unsigned char idx = (unsigned char)(((unsigned int)(baseID & 0xFF000000)) >> 24);
	IndexLookup::iterator it = Mods.find(idx);

	if (it != Mods.end())
		return (baseID & 0x00FFFFFF) | (((unsigned int) it->second) << 24);

	return baseID;
}

Lockable* Reference::SetReference(unsigned int refID)
{
	if (this->refID.get() == refID)
		return NULL;

	if (!this->refID.set(refID))
		return NULL;

	return &this->refID;
}

Lockable* Reference::SetBase(unsigned int baseID)
{
	if (this->baseID.get() == baseID)
		return NULL;

	if (!this->baseID.set(baseID))
		return NULL;

	return &this->baseID;
}

unsigned int Reference::GetReference() const
{
	return refID.get();
}

unsigned int Reference::GetBase() const
{
	return baseID.get();
}

bool Reference::IsPersistent() const
{
	unsigned int refID = GetReference();
	return !(refID & 0xFF000000) && refID;
}

const Parameter Reference::GetReferenceParam() const
{
	return Parameter(vector<string> {Utils::LongToHex(refID.get())}, NULL);
}

const Parameter Reference::GetBaseParam() const
{
	return Parameter(vector<string> {Utils::LongToHex(baseID.get())}, NULL);
}
