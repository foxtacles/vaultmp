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

template <typename T>
Lockable* Reference::SetObjectValue(Value<T>& dest, T value)
{
	if (dest.get() == value)
		return NULL;

	if (!dest.set(value))
		return NULL;

#ifdef VAULTMP_DEBUG

#endif

	return &dest;
}

template <>
Lockable* Reference::SetObjectValue(Value<double>& dest, double value)
{
	if (Utils::DoubleCompare(dest.get(), value, 0.0001))
		return NULL;

	if (!dest.set(value))
		return NULL;

#ifdef VAULTMP_DEBUG

#endif

	return &dest;
}

template Lockable* Reference::SetObjectValue(Value<unsigned int>& dest, unsigned int value);
template Lockable* Reference::SetObjectValue(Value<unsigned char>& dest, unsigned char value);
template Lockable* Reference::SetObjectValue(Value<bool>& dest, bool value);
template Lockable* Reference::SetObjectValue(Value<string>& dest, string value);

Lockable* Reference::SetReference(unsigned int refID)
{
	return SetObjectValue(this->refID, refID);
}

Lockable* Reference::SetBase(unsigned int baseID)
{
	return SetObjectValue(this->baseID, baseID);
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
