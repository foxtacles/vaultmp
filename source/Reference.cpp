#include "Reference.hpp"
#include "Utils.hpp"

using namespace std;
using namespace RakNet;

#ifdef VAULTMP_DEBUG
DebugInput<Reference> Reference::debug;
#endif

Guarded<Reference::RefIDs> Reference::refIDs;

Reference::Reference(unsigned int refID, unsigned int baseID) : Base()
{
	this->SetReference(refID);
	this->SetBase(baseID);
}

Reference::Reference(const pPacket& packet)
{
	pPacket pBaseNew = PacketFactory::Pop<pPacket>(packet);

	NetworkID id;

	PacketFactory::Access<pTypes::ID_BASE_NEW>(pBaseNew, id);

	this->SetNetworkID(id);

	unsigned int refID, baseID;

	PacketFactory::Access<pTypes::ID_REFERENCE_NEW>(packet, refID, baseID);

	this->SetReference(refID);
	this->SetBase(baseID);
}

Reference::~Reference() noexcept
{
	refIDs.Operate([this](RefIDs& refIDs) {
		refIDs.erase(this->refID.get());
	});
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

template<>
Lockable* Reference::SetObjectValue(Value<float>& dest, const float& value)
{
	if (Utils::DoubleCompare(dest.get(), value, 0.001f))
		return nullptr;

	if (!dest.set(value))
		return nullptr;

	return &dest;
}

template<>
Lockable* Reference::SetObjectValue(Value<tuple<float, float, float>>& dest, const tuple<float, float, float>& value)
{
	if (Utils::DoubleCompare(get<0>(dest.get()), get<0>(value), 0.001f) && Utils::DoubleCompare(get<1>(dest.get()), get<1>(value), 0.001f) && Utils::DoubleCompare(get<2>(dest.get()), get<2>(value), 0.001f))
		return nullptr;

	if (!dest.set(value))
		return nullptr;

	return &dest;
}

unsigned int Reference::GetReference() const
{
	return refID.get();
}

unsigned int Reference::GetBase() const
{
	return baseID.get();
}

Lockable* Reference::SetReference(unsigned int refID)
{
	unsigned int old_refID = this->refID.get();

	auto* result = SetObjectValue(this->refID, refID);

	if (result)
		refIDs.Operate([this, old_refID, refID](RefIDs& refIDs) {
			refIDs.erase(old_refID);

			if (refID)
				refIDs.emplace(refID, this->GetNetworkID());
		});

	return result;
}

Lockable* Reference::SetBase(unsigned int baseID)
{
	return SetObjectValue(this->baseID, baseID);
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

pPacket Reference::toPacket() const
{
	pPacket pBaseNew = Base::toPacket();

	pPacket packet = PacketFactory::Create<pTypes::ID_REFERENCE_NEW>(pBaseNew, this->GetReference(), this->GetBase());

	return packet;
}
