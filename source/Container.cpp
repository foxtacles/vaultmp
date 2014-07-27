#include "Container.hpp"

#ifndef VAULTSERVER
#include "Game.hpp"
#endif

using namespace std;
using namespace RakNet;

#ifdef VAULTMP_DEBUG
DebugInput<Container> Container::debug;
#endif

Container::Container(unsigned int refID, unsigned int baseID) : Object(refID, baseID), ItemList()
{
	initialize();
}

Container::Container(const pPacket& packet) : Object(PacketFactory::Pop<pPacket>(packet)), ItemList(PacketFactory::Pop<pPacket>(packet))
{
	initialize();
}

Container::~Container() noexcept {}

void Container::initialize()
{

}

#ifndef VAULTSERVER
FuncParameter Container::CreateFunctor(unsigned int flags, NetworkID id)
{
	return FuncParameter(unique_ptr<VaultFunctor>(new ContainerFunctor(flags, id)));
}
#endif

pPacket Container::toPacket() const
{
	pPacket pObjectNew = Object::toPacket();
	pPacket pItemListNew = ItemList::toPacket();

	pPacket packet = PacketFactory::Create<pTypes::ID_CONTAINER_NEW>(pObjectNew, pItemListNew);

	return packet;
}

#ifdef VAULTSERVER
Lockable* Container::SetBase(unsigned int baseID)
{
	const DB::Record* record = *DB::Record::Lookup(baseID, "CONT");

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

	if (!id)
	{
		auto references = Game::GetContext(ID_CONTAINER);

		for (unsigned int refID : references)
			GameFactory::Operate<Container, RETURN_FACTORY_VALIDATED>(refID, [this, refID, &result](FactoryContainer& container) {
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
