#include "ItemList.hpp"
#include "Network.hpp"
#include "Item.hpp"

#include <algorithm>

using namespace std;
using namespace RakNet;

#ifdef VAULTMP_DEBUG
DebugInput<ItemList> ItemList::debug;
#endif

ItemList::ItemList() : Base()
{
	initialize();
}

ItemList::ItemList(const pPacket& packet)
{
	initialize();

	pPacket pBaseNew = PacketFactory::Pop<pPacket>(packet);

	NetworkID id;

	PacketFactory::Access<pTypes::ID_BASE_NEW>(pBaseNew, id);

	this->SetNetworkID(id);

	vector<pPacket> items;

	PacketFactory::Access<pTypes::ID_ITEMLIST_NEW>(packet, items);

	for (const pPacket& _packet : items)
		AddItem(GameFactory::Create<Item, FailPolicy::Exception>(_packet));
}

ItemList::~ItemList() noexcept
{
	for (NetworkID id : container)
		GameFactory::Destroy(id);
}

void ItemList::initialize()
{

}

NetworkID ItemList::FindStackableItem(unsigned int baseID, float condition) const
{
	for (NetworkID id : container)
		if (GameFactory::Operate<Item>(id, [baseID, condition](Item* item) {
			return item->GetBase() == baseID && Utils::DoubleCompare(item->GetItemCondition(), condition, CONDITION_EPS);
		}))
			return id;

	return 0;
}

NetworkID ItemList::AddItem(NetworkID id)
{
	auto data = GameFactory::Operate<Item>(id, [this, id](Item* item) {
		NetworkID container = item->GetItemContainer();

		if (container)
		{
			// Alternative code path if the item has been received over network
			if (container == this->GetNetworkID() && find(this->container.begin(), this->container.end(), id) == this->container.end())
			{
				this->container.emplace_back(id);
				return make_pair(0u, 0.0f);
			}

			throw VaultException("Item is already owned by container %llu", container).stacktrace();
		}

		return make_pair(item->GetBase(), item->GetItemCondition());
	});

	if (!data.first)
		return id;

	NetworkID stackable = FindStackableItem(data.first, data.second);

	if (stackable)
	{
		auto data = GameFactory::Operate<Item, EXCEPTION_FACTORY_VALIDATED>(id, [](FactoryItem& item) {
			auto data = make_pair(item->GetItemEquipped(), item->GetItemCount());
			GameFactory::Destroy(item);
			return data;
		});

		GameFactory::Operate<Item>(stackable, [&data](Item* item) {
			if (data.first)
				item->SetItemEquipped(true);

			item->SetItemCount(item->GetItemCount() + data.second);
		});
	}
	else
	{
		GameFactory::Operate<Item>(id, [this](Item* item) {
			item->SetItemContainer(this->GetNetworkID());
		});

		container.emplace_back(id);
	}

	return stackable ? stackable : id;
}

ItemList::AddOp ItemList::AddItem(unsigned int baseID, unsigned int count, float condition, bool silent)
{
	AddOp result;

	if (!count)
		return result;

	NetworkID stackable = FindStackableItem(baseID, condition);

	if (stackable)
	{
		result.first = false;
		result.second = stackable;

		GameFactory::Operate<Item>(result.second, [count, silent](Item* item) {
			item->SetItemCount(item->GetItemCount() + count);
			item->SetItemSilent(silent);
		});
	}
	else
	{
		result.first = true;
		result.second = GameFactory::Create<Item, FailPolicy::Exception>(baseID);

		GameFactory::Operate<Item>(result.second, [this, count, condition, silent](Item* item) {
			item->SetItemCount(count);
			item->SetItemCondition(condition);
			item->SetItemSilent(silent);
			item->SetItemContainer(this->GetNetworkID());
		});

		container.emplace_back(result.second);
	}

	return result;
}

void ItemList::RemoveItem(NetworkID id)
{
	auto it = find(container.begin(), container.end(), id);

	if (it == container.end())
		throw VaultException("Unknown Item with NetworkID %llu in ItemList", id).stacktrace();

	GameFactory::Operate<Item>(id, [](Item* item) {
		item->SetItemContainer(0);
	});

	container.erase(it);
}

ItemList::RemoveOp ItemList::RemoveItem(unsigned int baseID, unsigned int count, bool silent)
{
	RemoveOp result;
	unsigned int count_ = count;

	for (NetworkID id : container)
	{
		if (!count)
			break;
		else
			GameFactory::Operate<Item>(id, [&result, id, baseID, &count, silent](Item* item) {
				if (item->GetBase() != baseID)
					return;

				if (item->GetItemCount() > count)
				{
					item->SetItemCount(item->GetItemCount() - count);
					item->SetItemSilent(silent);
					get<2>(result) = id;
					count = 0;
				}
				else
				{
					get<1>(result).emplace_back(id);
					item->SetItemSilent(silent);
					count -= item->GetItemCount();
				}
			});
	}

	get<0>(result) = count_ - count;

	return result;
}

NetworkID ItemList::EquipItem(unsigned int baseID, bool silent, bool stick) const
{
	NetworkID result = 0;

	if (!IsEquipped(baseID))
		for (NetworkID id : container)
			if (GameFactory::Operate<Item>(id, [id, baseID, silent, stick](Item* item) {
				if (item->GetBase() != baseID)
					return 0ull;

				item->SetItemEquipped(true);
				item->SetItemSilent(silent);
				item->SetItemStick(stick);
				return id;
			}))
				return id;

	return result;
}

NetworkID ItemList::UnequipItem(unsigned int baseID, bool silent, bool stick) const
{
	NetworkID id = IsEquipped(baseID);

	if (id)
		GameFactory::Operate<Item>(id, [silent, stick](Item* item) {
			item->SetItemEquipped(false);
			item->SetItemSilent(silent);
			item->SetItemStick(stick);
		});

	return id;
}

NetworkID ItemList::IsEquipped(unsigned int baseID) const
{
	for (NetworkID id : container)
		if (GameFactory::Operate<Item>(id, [baseID](Item* item) {
			return item->GetBase() == baseID && item->GetItemEquipped();
		}))
			return id;

	return 0;
}

unsigned int ItemList::GetItemCount(unsigned int baseID) const
{
	unsigned int count = 0;

	for (NetworkID id : container)
		GameFactory::Operate<Item>(id, [&count, baseID](Item* item) {
			if (!baseID || item->GetBase() == baseID)
				count += item->GetItemCount();
		});

	return count;
}

#ifdef VAULTSERVER
ItemList::Impl ItemList::GetItemTypes(const string& type) const
{
	Impl result;

	for (NetworkID id : container)
		GameFactory::Operate<Item>(id, [&result, &type, id](Item* item) {
			if (DB::Record::Lookup(item->GetBase(), type))
				result.emplace_back(id);
		});

	return result;
}
#endif

pPacket ItemList::toPacket() const
{
	vector<pPacket> items;
	items.reserve(container.size());

	for (NetworkID id : container)
		items.emplace_back(GameFactory::Operate<Item>(id, [](Item* item) { return item->toPacket(); }));

	pPacket pBaseNew = Base::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_ITEMLIST_NEW>(pBaseNew, move(items));

	return packet;
}
