#include "List.hpp"
#include "ListItem.hpp"

#include <algorithm>

using namespace std;
using namespace RakNet;

List::List() : Window()
{
	initialize();
}

List::List(const pDefault* packet) : Window(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	vector<pPacket> items;

	PacketFactory::Access<pTypes::ID_LIST_NEW>(packet, items);

	for (const pPacket& _packet : items)
		AddItem(GameFactory::Create<ListItem>(_packet.get()));
}

List::~List() noexcept {}

void List::initialize()
{

}

void List::AddItem(NetworkID id)
{
	if (!GameFactory::Operate<ListItem>(id, [this, id](FactoryListItem& listitem) {
		NetworkID container = listitem->GetItemContainer();

		if (!container)
			return true;

		// Alternative code path if the item has been received over network
		if (container == this->GetNetworkID() && find(this->container.begin(), this->container.end(), id) == this->container.end())
		{
			this->container.emplace_back(id);
			return false;
		}

		throw VaultException("ListItem is already owned by container %llu", container).stacktrace();
	})) return;

	GameFactory::Operate<ListItem>(id, [this](FactoryListItem& listitem) {
		listitem->SetItemContainer(this->GetNetworkID());
	});

	container.emplace_back(id);
}

void List::RemoveItem(NetworkID id)
{
	auto it = find(container.begin(), container.end(), id);

	if (it == container.end())
		throw VaultException("Unknown ListItem with NetworkID %llu in List", id).stacktrace();

	GameFactory::Operate<ListItem>(id, [](FactoryListItem& listitem) {
		listitem->SetItemContainer(0);
	});

	container.erase(it);
}

List::Impl List::RemoveAllItems()
{
	for (const NetworkID& id : container)
		GameFactory::Destroy(id);

	return move(container);
}

pPacket List::toPacket() const
{
	vector<pPacket> items;
	items.reserve(container.size());

	for (const NetworkID& id : container)
		items.emplace_back(GameFactory::Operate<ListItem>(id, [](FactoryListItem& listitem) { return listitem->toPacket(); }));

	pPacket pWindowNew = Window::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_LIST_NEW>(pWindowNew, move(items));

	return packet;
}
