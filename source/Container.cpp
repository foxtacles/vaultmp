#include "Container.h"
#include "PacketFactory.h"

#ifdef VAULTMP_DEBUG
Debug* Container::debug;
#endif

#ifdef VAULTMP_DEBUG
void Container::SetDebugHandler(Debug* debug)
{
	Container::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Container class", true);
}
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
	FactoryObject _item = GameFactory::GetObject(id);
	FactoryObject _item2 = GameFactory::GetObject(id2);
	Item* item = vaultcast<Item>(_item);
	Item* item2 = vaultcast<Item>(_item2);

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

StripCopy Container::Strip() const
{
	StripCopy result;
	result.first = this->Copy();

	FactoryObject _copy = GameFactory::GetObject(result.first);
	Container* copy = vaultcast<Container>(_copy);
	list<NetworkID> this_container = this->container;

	list<NetworkID>::iterator it, it2, it3, it4;

	for (it = this_container.begin(), it2 = copy->container.begin(); it != this_container.end() && it2 != copy->container.end(); ++it, ++it2)
	{
		auto it5 = result.second.emplace(*it2, list<NetworkID>{*it});

		FactoryObject _opt = GameFactory::GetObject(*it2);
		Item* opt = vaultcast<Item>(_opt);

		if (opt->GetItemEquipped())
			continue;

		for (++(it3 = it2), ++(it4 = it); it3 != copy->container.end() && it4 != this_container.end();)
		{
			FactoryObject _ins = GameFactory::GetObject(*it3);
			Item* ins = vaultcast<Item>(_ins);

			if (ins->GetBase() == opt->GetBase())
			{
				if (!ins->GetItemEquipped() && Utils::DoubleCompare(ins->GetItemCondition(), opt->GetItemCondition(), 0.001))
				{
					opt->SetItemCount(opt->GetItemCount() + ins->GetItemCount());
					GameFactory::DestroyInstance(_ins);
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
	FactoryObject reference = GameFactory::GetObject(id);

	if (vaultcast<Item>(reference))
	{
		container.emplace_back(id);
		container.sort(Item_sort);
	}
	else
		throw VaultException("Object with reference %08X is not an Item", (*reference)->GetReference());
}

ContainerDiff Container::AddItem(unsigned int baseID, unsigned int count, double condition, bool silent) const
{
	FactoryObject _item = GameFactory::GetObject(GameFactory::CreateInstance(ID_ITEM, baseID));
	Item* item = vaultcast<Item>(_item);
	item->SetItemCount(count);
	item->SetItemCondition(condition);
	item->SetItemSilent(silent);

	ContainerDiff diff;
	diff.second.emplace_back(item->GetNetworkID());

	return diff;
}

void Container::RemoveItem(NetworkID id)
{
	auto it = find(container.begin(), container.end(), id);

	if (it == container.end())
		throw VaultException("Unknown Item with NetworkID %llu in Container with reference %08X", id, this->GetReference());

	container.erase(it);
}

ContainerDiff Container::RemoveItem(unsigned int baseID, unsigned int count, bool silent) const
{
	ContainerDiff diff;
	list<NetworkID>::const_iterator it;

	for (it = container.begin(); it != container.end() && count; ++it)
	{
		FactoryObject _reference = GameFactory::GetObject(*it);
		Item* item = vaultcast<Item>(_reference);

		if (item->GetBase() == baseID)
		{
			item->SetItemSilent(silent);
			diff.first.emplace_back(item->GetNetworkID());

			if (item->GetItemCount() > count)
			{
				FactoryObject _item = GameFactory::GetObject(item->Copy());
				Item* _copy = vaultcast<Item>(_item);
				_copy->SetItemCount(_copy->GetItemCount() - count);
				_copy->SetItemSilent(silent);
				diff.second.emplace_back(_copy->GetNetworkID());
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
	return diff;
}

ContainerDiff Container::EquipItem(unsigned int baseID, bool silent, bool stick) const
{
	ContainerDiff diff;

	if (!IsEquipped(baseID))
	{
		for (const NetworkID& id : container)
		{
			FactoryObject _reference = GameFactory::GetObject(id);
			Item* item = vaultcast<Item>(_reference);

			if (item->GetBase() == baseID)
			{
				diff.first.emplace_back(id);

				FactoryObject _item = GameFactory::GetObject(item->Copy());
				Item* _copy = vaultcast<Item>(_item);
				_copy->SetItemEquipped(true);
				_copy->SetItemSilent(silent);
				_copy->SetItemStick(stick);
				diff.second.emplace_back(_copy->GetNetworkID());

				return diff;
			}
		}
	}

	return diff;
}

ContainerDiff Container::UnequipItem(unsigned int baseID, bool silent, bool stick) const
{
	ContainerDiff diff;
	NetworkID id = IsEquipped(baseID);

	if (id)
	{
		FactoryObject _reference = GameFactory::GetObject(id);
		Item* item = vaultcast<Item>(_reference);

		diff.first.emplace_back(id);

		FactoryObject _item = GameFactory::GetObject(item->Copy());
		Item* _copy = vaultcast<Item>(_item);
		_copy->SetItemEquipped(false);
		_copy->SetItemSilent(silent);
		_copy->SetItemStick(stick);
		diff.second.emplace_back(_copy->GetNetworkID());
	}

	return diff;
}

ContainerDiff Container::Compare(NetworkID id) const
{
	FactoryObject reference = GameFactory::GetObject(id);
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", (*reference)->GetReference());

	ContainerDiff diff;

	StripCopy _strip_self = this->Strip();
	unordered_map<NetworkID, list<NetworkID>>& _strip_assoc = _strip_self.second;
	FactoryObject _self = GameFactory::GetObject(_strip_self.first);
	FactoryObject _compare = GameFactory::GetObject(container->Strip().first);
	Container* self = vaultcast<Container>(_self);
	Container* compare = vaultcast<Container>(_compare);

	list<NetworkID>::iterator it, it2;

	for (it = compare->container.begin(), it2 = self->container.begin(); it != compare->container.end() && it2 != self->container.end();)
	{
		FactoryObject _iCompare = GameFactory::GetObject(*it);
		FactoryObject _iSelf = GameFactory::GetObject(*it2);
		Item* iCompare = vaultcast<Item>(_iCompare);
		Item* iSelf = vaultcast<Item>(_iSelf);

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
						GameFactory::DestroyInstance(_iCompare);
						GameFactory::DestroyInstance(_iSelf);
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

	GameFactory::DestroyInstance(_self);
	GameFactory::DestroyInstance(_compare);
	return diff;
}

NetworkID Container::IsEquipped(unsigned int baseID) const
{
	for (const NetworkID& id : container)
	{
		FactoryObject _reference = GameFactory::GetObject(id);
		Item* item = vaultcast<Item>(_reference);

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
		FactoryObject _iDelete = GameFactory::GetObject(id);
		Item* iDelete = vaultcast<Item>(_iDelete);

		Diff* _diff = nullptr;
		_diff = &assoc_delete.emplace(iDelete->GetBase(), Diff()).first->second;

		_diff->count -= iDelete->GetItemCount();
		_diff->equipped -= iDelete->GetItemEquipped();
		_diff->silent = iDelete->GetItemSilent();
		_diff->stick = iDelete->GetItemStick();

		this->RemoveItem(id);
		GameFactory::DestroyInstance(_iDelete);
	}

	for (NetworkID& id : diff.second)
	{
		FactoryObject _iNew = GameFactory::GetObject(id);
		Item* iNew = vaultcast<Item>(_iNew);

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
		FactoryObject item = GameFactory::GetObject(id);
		_diff.second.emplace_back(vaultcast<Item>(item)->toPacket());
	}

	return _diff;
}

void FreeDiff(ContainerDiff& diff)
{
	for (NetworkID& id : diff.second)
	{
		FactoryObject _item = GameFactory::GetObject(id);
		GameFactory::DestroyInstance(_item);
	}

	diff.first.clear();
	diff.second.clear();
}

NetworkID Container::Copy() const
{
	FactoryObject reference = GameFactory::GetObject(GameFactory::CreateInstance(ID_CONTAINER, 0x00000000, this->GetBase()));
	Container* container = vaultcast<Container>(reference);

	for (const NetworkID& id : this->container)
	{
		FactoryObject _reference = GameFactory::GetObject(id);
		Item* item = vaultcast<Item>(_reference);
		container->container.emplace_back(item->Copy());
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
		FactoryObject _reference = GameFactory::GetObject(id);
		Item* item = vaultcast<Item>(_reference);

		if (!baseID || item->GetBase() == baseID)
			count += item->GetItemCount();
	}

	return count;
}

void Container::PrintContainer() const
{
#ifdef VAULTMP_DEBUG

	if (debug)
	{
		debug->PrintFormat("Content of container %08X (%s):", true, this->GetBase(), this->GetName().c_str());

		for (const NetworkID& id : container)
		{
			FactoryObject _reference = GameFactory::GetObject(id);
			Item* item = vaultcast<Item>(_reference);
			debug->PrintFormat("%d of %s (%08X), condition %f, equipped state %d", true, item->GetItemCount(), item->GetName().c_str(), item->GetBase(), (float) item->GetItemCondition(), (int) item->GetItemEquipped());
		}
	}

#endif
}

void Container::FlushContainer()
{
	for (NetworkID& id : container)
	{
		FactoryObject _reference = GameFactory::GetObject(id);
		GameFactory::DestroyInstance(_reference);
	}

	container.clear();
}

const list<NetworkID>& Container::GetItemList() const
{
	return container;
}

#ifdef VAULTSERVER
list<NetworkID> Container::GetItemTypes(string type) const
{
	list<NetworkID> result;

	for (const NetworkID& id : container)
	{
		FactoryObject _reference = GameFactory::GetObject(id);
		Item* item = vaultcast<Item>(_reference);
		const Record& record = Record::Lookup(item->GetBase());

		if (record.GetType().compare(type) == 0)
			result.emplace_back(id);
	}

	return result;
}
#endif

pPacket Container::toPacket() const
{
	vector<pPacket> items;
	items.reserve(container.size());

	for (const NetworkID& id : container)
	{
		FactoryObject _reference = GameFactory::GetObject(id);
		Item* item = vaultcast<Item>(_reference);
		items.emplace_back(item->toPacket());
	}

	pPacket pObjectNew = Object::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_CONTAINER_NEW>(pObjectNew, move(items));

	return packet;
}
