#include "Reference.h"

using namespace std;
using namespace RakNet;

#ifdef VAULTMP_DEBUG
DebugInput<Reference> Reference::debug;
#endif

Reference::Reference(unsigned int refID, unsigned int baseID)
{
	this->refID.set(refID);
	this->baseID.set(baseID);
	this->changed.set(false);
	this->SetNetworkIDManager(Network::Manager());
}

Reference::~Reference()
{

}

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
template Lockable* Reference::SetObjectValue(Value<unsigned int>&, const unsigned int&);
template Lockable* Reference::SetObjectValue(Value<signed int>&, const signed int&);
template Lockable* Reference::SetObjectValue(Value<unsigned char>&, const unsigned char&);
template Lockable* Reference::SetObjectValue(Value<bool>&, const bool&);
template Lockable* Reference::SetObjectValue(Value<string>&, const string&);
template Lockable* Reference::SetObjectValue(Value<NetworkID>&, const NetworkID&);
template Lockable* Reference::SetObjectValue(Value<array<unsigned int, 6>>&, const array<unsigned int, 6>&);

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
	return ((refID & 0xFF000000) != 0xFF000000) && refID;
}

#ifndef VAULTSERVER
void Reference::Enqueue(const function<void()>& task)
{
	tasks.push(task);
}

void Reference::Work()
{
	while (!tasks.empty())
	{
		tasks.front()();
		tasks.pop();
	}
}

void Reference::Release()
{
	while (!tasks.empty())
		tasks.pop();
}
#endif
