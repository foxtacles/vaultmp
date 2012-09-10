#include "Reference.h"

#ifdef VAULTMP_DEBUG
Debug* Reference::debug;
#endif

Reference::Reference(unsigned int refID, unsigned int baseID)
{
	this->refID.set(refID);
	this->baseID.set(baseID);
	this->changed.set(false);
	this->SetNetworkIDManager(Network::Manager());
#ifdef VAULTMP_DEBUG
	//static_cast<CriticalSection*>(this)->SetDebugHandler(debug);
#endif
}

Reference::~Reference()
{

}

#ifdef VAULTMP_DEBUG
void Reference::SetDebugHandler(Debug* debug)
{
	Reference::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Reference class", true);
}
#endif

/*
unsigned int Reference::ResolveIndex(unsigned int baseID)
{
	unsigned char idx = (unsigned char)(((unsigned int)(baseID & 0xFF000000)) >> 24);
	IndexLookup::iterator it = Mods.find(idx);

	if (it != Mods.end())
		return (baseID & 0x00FFFFFF) | (((unsigned int) it->second) << 24);

	return baseID;
}
*/

template <typename T>
Lockable* Reference::SetObjectValue(Value<T>& dest, const T& value)
{
	if (dest.get() == value)
		return nullptr;

	if (!dest.set(value))
		return nullptr;

	changed.set(true);

#ifdef VAULTMP_DEBUG

#endif

	return &dest;
}

template <>
Lockable* Reference::SetObjectValue(Value<double>& dest, const double& value)
{
	if (Utils::DoubleCompare(dest.get(), value, 0.0001))
		return nullptr;

	if (!dest.set(value))
		return nullptr;

	changed.set(true);

#ifdef VAULTMP_DEBUG

#endif

	return &dest;
}
template Lockable* Reference::SetObjectValue(Value<unsigned int>& dest, const unsigned int& value);
template Lockable* Reference::SetObjectValue(Value<signed int>& dest, const signed int& value);
template Lockable* Reference::SetObjectValue(Value<unsigned char>& dest, const unsigned char& value);
template Lockable* Reference::SetObjectValue(Value<bool>& dest, const bool& value);
template Lockable* Reference::SetObjectValue(Value<string>& dest, const string& value);
template Lockable* Reference::SetObjectValue(Value<NetworkID>& dest, const NetworkID& id);

Lockable* Reference::SetReference(unsigned int refID)
{
	return SetObjectValue(this->refID, refID);
}

Lockable* Reference::SetBase(unsigned int baseID)
{
	return SetObjectValue(this->baseID, baseID);
}

Lockable* Reference::SetChanged(bool changed)
{
	return this->changed.set(changed) ? &this->changed : nullptr;
}

unsigned int Reference::GetReference() const
{
	return refID.get();
}

unsigned int Reference::GetBase() const
{
	return baseID.get();
}

bool Reference::GetChanged() const
{
	return changed.get();
}

bool Reference::IsPersistent() const
{
	unsigned int refID = GetReference();
	return !(refID & 0xFF000000) && refID;
}
