#include "ItemList.h"
#include "PacketFactory.h"
#include "GameFactory.h"

#ifndef VAULTSERVER
#include "Game.h"
#endif

using namespace std;
using namespace RakNet;

#ifdef VAULTMP_DEBUG
DebugInput<ItemList> ItemList::debug;
#endif

ItemList::~ItemList()
{
	this->FlushContainer();
}

bool ItemList::Item_sort(NetworkID id, NetworkID id2)
{
	auto items = GameFactory::GetMultiple<Item>(vector<NetworkID>{id, id2});
	auto& item = items[0];
	auto& item2 = items[1];

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

bool ItemList::Diff_sort(const pair<unsigned int, Diff>& diff, const pair<unsigned int, Diff>& diff2)
{
	if (diff.second.equipped > diff2.second.equipped)
		return false;

	return true;
}

ItemList::StripCopy ItemList::Strip() const
{
	StripCopy result;
	result.first = GameFactory::GetObject<Container>(source)->Copy();

	FactoryObject<Container> copy = GameFactory::GetObject<Container>(result.first).get();
	ItemListImpl this_container = this->container;

	ItemListImpl::iterator it, it2, it3, it4;

	for (it = this_container.begin(), it2 = copy->IL.container.begin(); it != this_container.end() && it2 != copy->IL.container.end(); ++it, ++it2)
	{
		auto it5 = result.second.emplace(*it2, ItemListImpl{*it});
		FactoryObject<Item> opt = GameFactory::GetObject<Item>(*it2).get();

		if (opt->GetItemEquipped())
			continue;

		for (++(it3 = it2), ++(it4 = it); it3 != copy->IL.GetItemList().end() && it4 != this_container.end();)
		{
			FactoryObject<Item> ins = GameFactory::GetObject<Item>(*it3).get();

			if (ins->GetBase() == opt->GetBase())
			{
				if (!ins->GetItemEquipped() && Utils::DoubleCompare(ins->GetItemCondition(), opt->GetItemCondition(), 0.001))
				{
					opt->SetItemCount(opt->GetItemCount() + ins->GetItemCount());
					GameFactory::DestroyInstance(ins);
					it5.first->second.emplace_back(*it4);
					copy->IL.container.erase(it3++);
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

void ItemList::AddItem(NetworkID id)
{
	GameFactory::GetObject<Item>(id).get()->SetItemContainer(source);
	container.emplace_back(id);
	container.sort(Item_sort);
}

ItemList::ContainerDiff ItemList::AddItem(unsigned int baseID, unsigned int count, double condition, bool silent) const
{
	ContainerDiff diff;

	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && (source && GameFactory::GetType(source) == ID_PLAYER))
		return diff;

	FactoryObject<Item> item = GameFactory::GetObject<Item>(GameFactory::CreateInstance(ID_ITEM, baseID)).get();
	item->SetItemCount(count);
	item->SetItemCondition(condition);
	item->SetItemSilent(silent);

	diff.second.emplace_back(item->GetNetworkID());

	return diff;
}

void ItemList::RemoveItem(NetworkID id)
{
	auto it = find(container.begin(), container.end(), id);

	if (it == container.end())
		throw VaultException("Unknown Item with NetworkID %llu in ItemList", id).stacktrace();

	FactoryObject<Item> reference = GameFactory::GetObject<Item>(*it).get();
	reference->SetItemContainer(0);
	container.erase(it);
}

ItemList::ContainerDiff ItemList::RemoveItem(unsigned int baseID, unsigned int count, bool silent) const
{
	ContainerDiff diff;
	ItemListImpl::const_iterator it;

	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && (source && GameFactory::GetType(source) == ID_PLAYER))
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
				FactoryObject<Item> copy = GameFactory::GetObject<Item>(item->Copy()).get();
				copy->SetItemCount(item->GetItemCount() - count);
				copy->SetItemSilent(silent);
				diff.second.emplace_back(copy->GetNetworkID());
				return diff;
			}

			count -= item->GetItemCount();
		}
	}

	return diff;
}

ItemList::ContainerDiff ItemList::RemoveAllItems() const
{
	ContainerDiff diff;
	diff.first = this->container;

	if (source && GameFactory::GetType(source) == ID_PLAYER)
		diff.first.remove_if([](const NetworkID& id)
		{
			FactoryObject<Item> reference = GameFactory::GetObject<Item>(id).get();
			unsigned int baseID = reference->GetBase();
			return (baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES);
		});

	return diff;
}

ItemList::ContainerDiff ItemList::EquipItem(unsigned int baseID, bool silent, bool stick) const
{
	ContainerDiff diff;

	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && (source && GameFactory::GetType(source) == ID_PLAYER))
		return diff;

	if (!IsEquipped(baseID))
	{
		for (const NetworkID& id : container)
		{
			FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();

			if (item->GetBase() == baseID)
			{
				diff.first.emplace_back(id);

				FactoryObject<Item> copy = GameFactory::GetObject<Item>(item->Copy()).get();
				copy->SetItemEquipped(true);
				copy->SetItemSilent(silent);
				copy->SetItemStick(stick);
				diff.second.emplace_back(copy->GetNetworkID());

				return diff;
			}
		}
	}

	return diff;
}

ItemList::ContainerDiff ItemList::UnequipItem(unsigned int baseID, bool silent, bool stick) const
{
	ContainerDiff diff;

	if ((baseID == PIPBOY_3000 || baseID == PIPBOY_GLOVES) && (source && GameFactory::GetType(source) == ID_PLAYER))
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

ItemList::ContainerDiff ItemList::Compare(NetworkID id) const
{
	FactoryObject<Container> container = GameFactory::GetObject<Container>(id).get();
	ContainerDiff diff;

	StripCopy _strip_self = this->Strip();
	unordered_map<NetworkID, ItemListImpl>& _strip_assoc = _strip_self.second;
	FactoryObject<Container> self = GameFactory::GetObject<Container>(_strip_self.first).get();
	FactoryObject<Container> compare = GameFactory::GetObject<Container>(container->IL.Strip().first).get();

	ItemListImpl::iterator it, it2;

	for (it = compare->IL.container.begin(), it2 = self->IL.container.begin(); it != compare->IL.container.end() && it2 != self->IL.container.end();)
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
						compare->IL.container.erase(it++);
						self->IL.container.erase(it2++);
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

	for (const auto& id : self->IL.container)
	{
		ItemListImpl& _delete = _strip_assoc.find(id)->second;

		for (const auto& id : _delete)
			diff.first.emplace_back(id);
	}

	for (it = compare->IL.container.begin(); it != compare->IL.container.end(); compare->IL.container.erase(it++))
		diff.second.emplace_back(*it);

	diff.first.sort(Item_sort);
	diff.second.sort(Item_sort);

	GameFactory::DestroyInstance(self);
	GameFactory::DestroyInstance(compare);
	return diff;
}

NetworkID ItemList::IsEquipped(unsigned int baseID) const
{
	for (const NetworkID& id : container)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();

		if (item->GetBase() == baseID && item->GetItemEquipped())
			return id;
	}

	return 0;
}

ItemList::GameDiff ItemList::ApplyDiff(ContainerDiff& diff)
{
	GameDiff result;
	unordered_map<unsigned int, Diff> assoc_delete;

	for (NetworkID& id : diff.first)
	{
		auto _iDelete = GameFactory::GetObject<Item>(id);

		if (!_iDelete)
		{
#ifdef VAULTMP_DEBUG
			debug.print("WARNING (ApplyDiff): item ", dec, id, " not found. Has it already been deleted? ", GameFactory::IsDeleted(id) ? "YES" : "NO", ", in the container? ", find(container.begin(), container.end(), id) != container.end() ? "YES" : "NO");
#endif
			continue;
		}

		auto& iDelete = _iDelete.get();
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

ItemList::ContainerDiff ItemList::ToContainerDiff(const NetDiff& diff)
{
	ContainerDiff _diff(make_pair(diff.first, ItemListImpl()));

	for (const auto& packet : diff.second)
		_diff.second.emplace_back(GameFactory::CreateKnownInstance(ID_ITEM, packet.get()));

	return _diff;
}

ItemList::NetDiff ItemList::ToNetDiff(const ContainerDiff& diff)
{
	NetDiff _diff(make_pair(diff.first, vector<pPacket>()));

	for (const auto& id : diff.second)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();
		_diff.second.emplace_back(item->toPacket());
	}

	return _diff;
}

void ItemList::FreeDiff(ContainerDiff& diff)
{
	for (NetworkID& id : diff.second)
		GameFactory::DestroyInstance(id);

	diff.first.clear();
	diff.second.clear();
}

ItemList ItemList::Copy(NetworkID source) const
{
	ItemList container(source);

	for (const NetworkID& id : this->container)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();
		container.AddItem(item->Copy());
	}

	return container;
}

bool ItemList::IsEmpty() const
{
	return container.empty();
}

unsigned int ItemList::GetItemCount(unsigned int baseID) const
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

void ItemList::FlushContainer()
{
	for (NetworkID& id : container)
		GameFactory::DestroyInstance(id);

	container.clear();
}

const ItemList::ItemListImpl& ItemList::GetItemList() const
{
	return container;
}

#ifdef VAULTSERVER
ItemList::ItemListImpl ItemList::GetItemTypes(const string& type) const
{
	ItemListImpl result;

	for (const NetworkID& id : container)
	{
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();

		if (DB::Record::Lookup(item->GetBase(), type))
			result.emplace_back(id);
	}

	return result;
}
#endif
