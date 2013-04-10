#include "Container.h"
#include "PacketFactory.h"
#include "GameFactory.h"

#ifndef VAULTSERVER
#include "Game.h"
#endif

using namespace std;
using namespace RakNet;

#ifdef VAULTMP_DEBUG
DebugInput<Container> Container::debug;
#endif

Container::Container(unsigned int refID, unsigned int baseID) : Object(refID, baseID), IL(this->GetNetworkID())
{
	initialize();
}

Container::Container(const pDefault* packet) : Object(PacketFactory::Pop<pPacket>(packet)), IL(this->GetNetworkID())
{
	initialize();

	vector<pPacket> items;

	PacketFactory::Access<pTypes::ID_CONTAINER_NEW>(packet, items);

	for (const pPacket& _packet : items)
	{
		NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, _packet.get());
		IL.AddItem(id);
	}
}

Container::Container(pPacket&& packet) : Container(packet.get())
{

}

Container::~Container()
{

}

void Container::initialize()
{
#ifdef VAULTSERVER
	unsigned int baseID = this->GetBase();

	if (baseID != PLAYER_BASE)
	{
		const DB::Record* record = *DB::Record::Lookup(baseID, vector<string>{"CONT", "NPC_", "CREA"});

		if (this->GetName().empty())
			this->SetName(record->GetDescription());
	}
#endif
}

#ifndef VAULTSERVER
FuncParameter Container::CreateFunctor(unsigned int flags, NetworkID id)
{
	return FuncParameter(unique_ptr<VaultFunctor>(new ContainerFunctor(flags, id)));
}
#endif

NetworkID Container::Copy() const
{
	FactoryObject<Container> container = GameFactory::GetObject<Container>(GameFactory::CreateInstance(ID_CONTAINER, 0x00000000, this->GetBase())).get();
	NetworkID id = container->GetNetworkID();
	IL.Copy(container->IL);
	return id;
}

Lockable* Container::getLock()
{
	if (flag_Lock.IsLocked())
		return nullptr;

	return &flag_Lock;
}

pPacket Container::toPacket() const
{
	vector<pPacket> items;
	items.reserve(IL.GetItemList().size());

	for (const NetworkID& id : IL.GetItemList())
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();
		items.emplace_back(item->toPacket());
	}

	pPacket pObjectNew = Object::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_CONTAINER_NEW>(pObjectNew, move(items));

	return packet;
}

#ifdef VAULTSERVER
Lockable* Container::SetBase(unsigned int baseID)
{
	const DB::Record* record = *DB::Record::Lookup(baseID, vector<string>{"CONT", "NPC_", "CREA"});

	if (this->GetName().empty())
		this->SetName(record->GetDescription());

	return Reference::SetBase(baseID);
}
#endif

#ifndef VAULTSERVER
vector<string> ContainerFunctor::operator()()
{
	vector<string> result;

	NetworkID id = get();

	if (id)
	{

	}
	else
	{
		auto references = Game::GetContext(ID_CONTAINER);
		Expected<FactoryObject<Container>> container;

		for (unsigned int refID : references)
			if ((container = GameFactory::GetObject<Container>(refID)))
			{
				auto& container_ = container.get();

				if (!filter(container_))
					result.emplace_back(Utils::toString(refID));

				GameFactory::LeaveReference(container_);
			}
	}

	_next(result);

	return result;
}

bool ContainerFunctor::filter(FactoryObject<Reference>& reference)
{
	if (ObjectFunctor::filter(reference))
		return true;

	return false;
}
#endif
