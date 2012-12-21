#include "Container.h"
#include "PacketFactory.h"
#include "GameFactory.h"

using namespace std;
using namespace RakNet;

#ifdef VAULTMP_DEBUG
DebugInput<Container> Container::debug;
#endif

Container::Container(unsigned int refID, unsigned int baseID) : Object(refID, baseID)
{
	initialize();
}

Container::Container(const pDefault* packet) : Object(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	vector<pPacket> items;

	PacketFactory::Access<pTypes::ID_CONTAINER_NEW>(packet, items);

	for (const pPacket& _packet : items)
	{
		NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, _packet.get());
		this->AddItem(id);
	}
}

Container::Container(pPacket&& packet) : Container(packet.get())
{

}

Container::~Container()
{
	this->FlushContainer();
}

void Container::initialize()
{

}

bool Container::Item_sort(NetworkID id, NetworkID id2)
{
	auto items = GameFactory::GetMultiple<Item>(vector<NetworkID>{id, id2});
	const auto& item = items[0];
	const auto& item2 = items[1];

	if (item->GetBase() > item2->GetBase())
		return false;

	else if (item->GetBase() == item2->GetBase())
	{
		if (item->GetItemEquipped() == item2->GetItemEquipped() && item->GetItemCondition() < item2->GetItemCondition())
			return false;
		else if (item2->GetItemEquipped())
			return false;
	}

	return true;
}

bool Container::Diff_sort(const pair<unsigned int, Diff>& diff, const pair<unsigned int, Diff>& diff2)
{
	if (diff.second.equipped > diff2.second.equipped)
		return false;

	return true;
}

Container::StripCopy Container::Strip() const
{
	StripCopy result;
	result.first = this->Copy();

	FactoryObject<Container> copy = GameFactory::GetObject<Container>(result.first).get();
	list<NetworkID> this_container = this->container;

	list<NetworkID>::iterator it, it2, it3, it4;

	for (it = this_container.begin(), it2 = copy->container.begin(); it != this_container.end() && it2 != copy->container.end(); ++it, ++it2)
	{
		auto it5 = result.second.emplace(*it2, list<NetworkID>{*it});
		FactoryObject<Item> opt = GameFactory::GetObject<Item>(*it2).get();

		if (opt->GetItemEquipped())
			continue;

		for (++(it3 = it2), ++(it4 = it); it3 != copy->container.end() && it4 != this_container.end();)
		{
			FactoryObject<Item> ins = GameFactory::GetObject<Item>(*it3).get();

			if (ins->GetBase() == opt->GetBase())
			{
				if (!ins->GetItemEquipped() && Utils::DoubleCompare(ins->GetItemCondition(), opt->GetItemCondition(), 0.001))
				{
					opt->SetItemCount(opt->GetItemCount() + ins->GetItemCount());
					GameFactory::DestroyInstance(ins);
					it5.first->second.emplace_back(*it4);
					copy->container.erase(it3++);
					this_container.erase(it4++);
					continue;
				}
			}
			else
				break;

			++it3;
			++it4;
		}
	}

	return result;
}

void Container::AddItem(NetworkID id)
{
	GameFactory::GetObject<Item>(id).get()->SetItemContainer(this->GetNetworkID());
	container.emplace_back(id);
	container.sort(Item_sort);
}

ContainerDiff Container::AddItem(unsigned int baseID, unsigned int count, double condition, bool silent) const
{
	ContainerDiff diff;

	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && GameFactory::GetType(this) == ID_PLAYER)
		return diff;

	FactoryObject<Item> item = GameFactory::GetObject<Item>(GameFactory::CreateInstance(ID_ITEM, baseID)).get();
	item->SetItemCount(count);
	item->SetItemCondition(condition);
	item->SetItemSilent(silent);

	diff.second.emplace_back(item->GetNetworkID());

	return diff;
}

void Container::RemoveItem(NetworkID id)
{
	auto it = find(container.begin(), container.end(), id);

	if (it == container.end())
		throw VaultException("Unknown Item with NetworkID %llu in Container with reference %08X", id, this->GetReference());

	FactoryObject<Item> reference = GameFactory::GetObject<Item>(*it).get();
	reference->SetItemContainer(0);
	container.erase(it);
}

ContainerDiff Container::RemoveItem(unsigned int baseID, unsigned int count, bool silent) const
{
	ContainerDiff diff;
	list<NetworkID>::const_iterator it;

	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && GameFactory::GetType(this) == ID_PLAYER)
		return diff;

	for (it = container.begin(); it != container.end() && count; ++it)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(*it).get();

		if (item->GetBase() == baseID)
		{
			item->SetItemSilent(silent);
			diff.first.emplace_back(item->GetNetworkID());

			if (item->GetItemCount() > count)
			{
				FactoryObject<Item> item = GameFactory::GetObject<Item>(item->Copy()).get();
				item->SetItemCount(item->GetItemCount() - count);
				item->SetItemSilent(silent);
				diff.second.emplace_back(item->GetNetworkID());
				return diff;
			}

			count -= item->GetItemCount();
		}
	}

	return diff;
}

ContainerDiff Container::RemoveAllItems() const
{
	ContainerDiff diff;
	diff.first = this->container;

	if (GameFactory::GetType(this) == ID_PLAYER)
		diff.first.remove_if([](const NetworkID& id)
		{
			FactoryObject<Item> reference = GameFactory::GetObject<Item>(id).get();
			unsigned int baseID = reference->GetBase();
			return (baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES);
		});

	return diff;
}

ContainerDiff Container::EquipItem(unsigned int baseID, bool silent, bool stick) const
{
	ContainerDiff diff;

	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && GameFactory::GetType(this) == ID_PLAYER)
		return diff;

	if (!IsEquipped(baseID))
	{
		for (const NetworkID& id : container)
		{
			FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();

			if (item->GetBase() == baseID)
			{
				diff.first.emplace_back(id);

				FactoryObject<Item> item = GameFactory::GetObject<Item>(item->Copy()).get();
				item->SetItemEquipped(true);
				item->SetItemSilent(silent);
				item->SetItemStick(stick);
				diff.second.emplace_back(item->GetNetworkID());

				return diff;
			}
		}
	}

	return diff;
}

ContainerDiff Container::UnequipItem(unsigned int baseID, bool silent, bool stick) const
{
	ContainerDiff diff;

	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && GameFactory::GetType(this) == ID_PLAYER)
		return diff;

	NetworkID id = IsEquipped(baseID);

	if (id)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();
		diff.first.emplace_back(id);

		FactoryObject<Item> copy = GameFactory::GetObject<Item>(item->Copy()).get();
		copy->SetItemEquipped(false);
		copy->SetItemSilent(silent);
		copy->SetItemStick(stick);
		diff.second.emplace_back(copy->GetNetworkID());
	}

	return diff;
}

ContainerDiff Container::Compare(NetworkID id) const
{
	FactoryObject<Container> container = GameFactory::GetObject<Container>(id).get();
	ContainerDiff diff;

	StripCopy _strip_self = this->Strip();
	unordered_map<NetworkID, list<NetworkID>>& _strip_assoc = _strip_self.second;
	FactoryObject<Container> self = GameFactory::GetObject<Container>(_strip_self.first).get();
	FactoryObject<Container> compare = GameFactory::GetObject<Container>(container->Strip().first).get();

	list<NetworkID>::iterator it, it2;

	for (it = compare->container.begin(), it2 = self->container.begin(); it != compare->container.end() && it2 != self->container.end();)
	{
		FactoryObject<Item> iCompare = GameFactory::GetObject<Item>(*it).get();
		FactoryObject<Item> iSelf = GameFactory::GetObject<Item>(*it2).get();

		unsigned int iCompare_base = iCompare->GetBase();
		unsigned int iSelf_base = iSelf->GetBase();

		if (iCompare_base == iSelf_base)
		{
			if (iCompare->GetItemEquipped() == iSelf->GetItemEquipped())
			{
				if (Utils::DoubleCompare(iCompare->GetItemCondition(), iSelf->GetItemCondition(), 0.001))
				{
					if (iCompare->GetItemCount() == iSelf->GetItemCount())   // Item in self is existent in compare (match)
					{
						GameFactory::DestroyInstance(iCompare);
						GameFactory::DestroyInstance(iSelf);
						compare->container.erase(it++);
						self->container.erase(it2++);
						continue;
					}
				}
			}
		}
		else if (iCompare_base < iSelf_base)   // Item in compare is not existent in self (new / changed)
		{
			++it;
			continue;
		}

		// Item in self is not existent in compare (delete / changed)
		++it2;
	}

	for (const auto& id : self->container)
	{
		list<NetworkID>& _delete = _strip_assoc.find(id)->second;

		for (const auto& id : _delete)
			diff.first.emplace_back(id);
	}

	for (it = compare->container.begin(); it != compare->container.end(); compare->container.erase(it++))
		diff.second.emplace_back(*it);

	diff.first.sort(Item_sort);
	diff.second.sort(Item_sort);

	GameFactory::DestroyInstance(self);
	GameFactory::DestroyInstance(compare);
	return diff;
}

NetworkID Container::IsEquipped(unsigned int baseID) const
{
	for (const NetworkID& id : container)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();

		if (item->GetBase() == baseID && item->GetItemEquipped())
			return id;
	}

	return 0;
}

GameDiff Container::ApplyDiff(ContainerDiff& diff)
{
	GameDiff result;
	unordered_map<unsigned int, Diff> assoc_delete;

	for (NetworkID& id : diff.first)
	{
		FactoryObject<Item> iDelete = GameFactory::GetObject<Item>(id).get();
		Diff* _diff = nullptr;
		_diff = &assoc_delete.emplace(iDelete->GetBase(), Diff()).first->second;

		_diff->count -= iDelete->GetItemCount();
		_diff->equipped -= iDelete->GetItemEquipped();
		_diff->condition = iDelete->GetItemCondition();
		_diff->silent = iDelete->GetItemSilent();
		_diff->stick = iDelete->GetItemStick();

		this->RemoveItem(id);
		GameFactory::DestroyInstance(iDelete);
	}

	for (NetworkID& id : diff.second)
	{
		FactoryObject<Item> iNew = GameFactory::GetObject<Item>(id).get();
		Diff* _diff = nullptr;
		auto it = assoc_delete.find(iNew->GetBase());

		if (it != assoc_delete.end())
		{
			_diff = &it->second;

			if (iNew->GetItemEquipped() && _diff->equipped == -1)
			{
				Diff _result;
				_result.count = 0;
				_result.condition = iNew->GetItemCondition();
				_result.equipped = 0;
				result.emplace_back(iNew->GetBase(), _result);

				_diff->count += iNew->GetItemCount(); // always 1
				_diff->equipped = 0;
			}
			else
			{
				_diff->count += iNew->GetItemCount();
				_diff->condition = iNew->GetItemCondition();
				_diff->silent = iNew->GetItemSilent();
				_diff->stick = iNew->GetItemStick();

				if (iNew->GetItemEquipped())
					_diff->equipped = 1;
			}
		}
		else
		{
			Diff _result;
			_result.count = iNew->GetItemCount();
			_result.condition = iNew->GetItemCondition();
			_result.equipped = iNew->GetItemEquipped();
			_result.silent = iNew->GetItemSilent();
			_result.stick = iNew->GetItemStick();
			result.emplace_back(iNew->GetBase(), _result);
		}

		this->AddItem(id);
	}

	for (const auto& _diff : assoc_delete)
	{
		if (_diff.second.count == 0 && _diff.second.equipped == 0)
			continue;

		result.emplace_back(_diff);
	}

	result.sort(Diff_sort);
	diff.first.clear();
	diff.second.clear();
	return result;
}

ContainerDiff Container::ToContainerDiff(const ContainerDiffNet& diff)
{
	ContainerDiff _diff(make_pair(diff.first, list<NetworkID>()));

	for (const auto& packet : diff.second)
		_diff.second.emplace_back(GameFactory::CreateKnownInstance(ID_ITEM, packet.get()));

	return _diff;
}

ContainerDiffNet Container::ToNetDiff(const ContainerDiff& diff)
{
	ContainerDiffNet _diff(make_pair(diff.first, vector<pPacket>()));

	for (const auto& id : diff.second)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();
		_diff.second.emplace_back(item->toPacket());
	}

	return _diff;
}

void FreeDiff(ContainerDiff& diff)
{
	for (NetworkID& id : diff.second)
		GameFactory::DestroyInstance(id);

	diff.first.clear();
	diff.second.clear();
}

NetworkID Container::Copy() const
{
	FactoryObject<Container> container = GameFactory::GetObject<Container>(GameFactory::CreateInstance(ID_CONTAINER, 0x00000000, this->GetBase())).get();

	for (const NetworkID& id : this->container)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();
		container->AddItem(item->Copy());
	}

	return container->GetNetworkID();
}

Lockable* Container::getLock()
{
	if (flag_Lock.IsLocked())
		return nullptr;

	return &flag_Lock;
}

bool Container::IsEmpty() const
{
	return container.empty();
}

unsigned int Container::GetItemCount(unsigned int baseID) const
{
	unsigned int count = 0;

	for (const NetworkID& id : container)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();

		if (!baseID || item->GetBase() == baseID)
			count += item->GetItemCount();
	}

	return count;
}

void Container::FlushContainer()
{
	for (NetworkID& id : container)
		GameFactory::DestroyInstance(id);

	container.clear();
}

const list<NetworkID>& Container::GetItemList() const
{
	return container;
}

#ifdef VAULTSERVER
list<NetworkID> Container::GetItemTypes(const string& type) const
{
	list<NetworkID> result;

	for (const NetworkID& id : container)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();

		try
		{
			Record::Lookup(item->GetBase(), type);
			result.emplace_back(id);
		}
		catch (...) {}
	}

	return result;
}
#endif


#ifdef VAULTMP_DEBUG
void Container::PrintContainer() const
{
	debug.print("Content of container ", hex, this->GetBase(), " (", this->GetName().c_str(), ")");

	for (const NetworkID& id : container)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();
		debug.print(dec, item->GetItemCount(), " of ", item->GetName().c_str(), " (", hex, item->GetBase(), "), condition ", item->GetItemCondition(), ", equipped state ", item->GetItemEquipped());
	}
}
#endif

pPacket Container::toPacket() const
{
	vector<pPacket> items;
	items.reserve(container.size());

	for (const NetworkID& id : container)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();
		items.emplace_back(item->toPacket());
	}

	pPacket pObjectNew = Object::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_CONTAINER_NEW>(pObjectNew, move(items));

	return packet;
}
