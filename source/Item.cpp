#include "Item.hpp"

using namespace std;
using namespace RakNet;

#ifdef VAULTMP_DEBUG
DebugInput<Item> Item::debug;
#endif

Item::Item(unsigned int refID, unsigned int baseID) : Object(refID, baseID)
{
	initialize();
}

Item::Item(const pPacket& packet) : Object(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	NetworkID id;
	unsigned int count;
	float condition;
	bool equipped, silent, stick;

	PacketFactory::Access<pTypes::ID_ITEM_NEW>(packet, id, count, condition, equipped, silent, stick);

	this->SetItemContainer(id);
	this->SetItemCount(count);
	this->SetItemCondition(condition);
	this->SetItemEquipped(equipped);
	this->SetItemSilent(silent);
	this->SetItemStick(stick);
}

Item::~Item() noexcept {}

void Item::initialize()
{
	this->SetItemSilent(true);
}

NetworkID Item::GetItemContainer() const
{
	return this->item_Container.get();
}

unsigned int Item::GetItemCount() const
{
	return this->item_Count.get();
}

float Item::GetItemCondition() const
{
	return this->item_Condition.get();
}

bool Item::GetItemEquipped() const
{
	return this->state_Equipped.get();
}

bool Item::GetItemSilent() const
{
	return this->flag_Silent.get();
}

bool Item::GetItemStick() const
{
	return this->flag_Stick.get();
}

Lockable* Item::SetItemContainer(NetworkID id)
{
	return SetObjectValue(this->item_Container, id);
}

Lockable* Item::SetItemCount(unsigned int count)
{
	return SetObjectValue(this->item_Count, count);
}

Lockable* Item::SetItemCondition(float condition)
{
	if (condition < 0.00f || condition > 100.0f)
		return nullptr;

	return SetObjectValue(this->item_Condition, condition);
}

Lockable* Item::SetItemEquipped(bool state)
{
	return SetObjectValue(this->state_Equipped, state);
}

Lockable* Item::SetItemSilent(bool silent)
{
	return SetObjectValue(this->flag_Silent, silent);
}

Lockable* Item::SetItemStick(bool stick)
{
	return SetObjectValue(this->flag_Stick, stick);
}

#ifdef VAULTSERVER
Lockable* Item::SetBase(unsigned int baseID)
{
	const DB::Record* record = *DB::Record::Lookup(baseID, vector<string>{"ALCH", "AMMO", "ARMA", "ARMO", "BOOK", "ENCH", "KEYM", "MISC", "NOTE", "WEAP", "LIGH"});

	if (this->GetName().empty())
		this->SetName(record->GetDescription());

	return Reference::SetBase(baseID);
}
#endif

pPacket Item::toPacket() const
{
	pPacket pObjectNew = Object::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_ITEM_NEW>(pObjectNew, this->GetItemContainer(), this->GetItemCount(), this->GetItemCondition(), this->GetItemEquipped(), this->GetItemSilent(), this->GetItemStick());

	return packet;
}
