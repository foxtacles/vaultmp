#include "Container.h"

#ifndef VAULTSERVER
#include "Game.h"
#else
#include "Item.h"
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
		IL.AddItem(GameFactory::Create<Item>(_packet.get()));
}

Container::~Container() noexcept {}

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
	return GameFactory::Operate<Container>(GameFactory::Create<Container>(0x00000000, this->GetBase()), [this](FactoryContainer& container) {
		IL.Copy(container->IL);
		return container->GetNetworkID();
	});
}

pPacket Container::toPacket() const
{
	vector<pPacket> items;
	items.reserve(IL.GetItemList().size());

	for (const NetworkID& id : IL.GetItemList())
		items.emplace_back(GameFactory::Operate<Item>(id, [](FactoryItem& item) { return item->toPacket(); }));

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

	return Object::SetBase(baseID);
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

		for (unsigned int refID : references)
			GameFactory::Operate<Container, FailPolicy::Return>(refID, [this, refID, &result](FactoryContainer& container) {
				if (!filter(container))
					result.emplace_back(Utils::toString(refID));
			});
	}

	_next(result);

	return result;
}

bool ContainerFunctor::filter(FactoryWrapper<Reference>& reference)
{
	if (ObjectFunctor::filter(reference))
		return true;

	return false;
}
#endif
