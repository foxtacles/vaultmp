#include "Item.h"
#include "Container.h"
#include "PacketFactory.h"

using namespace RakNet;

#ifdef VAULTMP_DEBUG
Debug* Item::debug;
#endif

#ifdef VAULTMP_DEBUG
void Item::SetDebugHandler(Debug* debug)
{
	Item::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Item class", true);
}
#endif

Item::Item(unsigned int refID, unsigned int baseID) : Object(refID, baseID)
{
	initialize();
}

Item::Item(const pDefault* packet) : Object(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	NetworkID id;
	unsigned int count;
	double condition;
	bool equipped, silent, stick;

	PacketFactory::Access<pTypes::ID_ITEM_NEW>(packet, id, count, condition, equipped, silent, stick);

	this->SetItemContainer(id);
	this->SetItemCount(count);
	this->SetItemCondition(condition);
	this->SetItemEquipped(equipped);
	this->SetItemSilent(silent);
	this->SetItemStick(stick);
}

Item::~Item()
{

}

void Item::initialize()
{
#ifdef VAULTSERVER
	unsigned int baseID = this->GetBase();

	const Record& record = Record::Lookup(baseID, vector<string>{"ALCH", "AMMO", "ARMA", "ARMO", "BOOK", "CCRD", "CDCK", "CHIP", "CMNY", "ENCH", "IMOD", "KEYM", "MISC", "NOTE", "RCPE", "WEAP", "LIGH"});

	if (this->GetName().empty())
		this->SetName(record.GetDescription());
#endif
}

NetworkID Item::GetItemContainer() const
{
	return this->item_Container.get();
}

unsigned int Item::GetItemCount() const
{
	return this->item_Count.get();
}

double Item::GetItemCondition() const
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

Lockable* Item::SetItemCondition(double condition)
{
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

NetworkID Item::Copy() const
{
	FactoryObject reference = GameFactory::GetObject(GameFactory::CreateInstance(ID_ITEM, 0x00000000, this->GetBase()));
	Item* item = vaultcast<Item>(reference);

	item->SetItemContainer(this->GetItemContainer());
	item->SetItemCount(this->GetItemCount());
	item->SetItemCondition(this->GetItemCondition());
	item->SetItemEquipped(this->GetItemEquipped());
	item->SetItemSilent(this->GetItemSilent());
	item->SetItemStick(this->GetItemStick());

	return item->GetNetworkID();
}

#ifdef VAULTSERVER
Lockable* Item::SetBase(unsigned int baseID)
{
	const Record& record = Record::Lookup(baseID, vector<string>{"ALCH", "AMMO", "ARMA", "ARMO", "BOOK", "CCRD", "CDCK", "CHIP", "CMNY", "ENCH", "IMOD", "KEYM", "MISC", "NOTE", "RCPE", "WEAP", "LIGH"});

	if (this->GetName().empty())
		this->SetName(record.GetDescription());

	return Reference::SetBase(baseID);
}
#endif

pPacket Item::toPacket() const
{
	pPacket pObjectNew = Object::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_ITEM_NEW>(pObjectNew, this->GetItemContainer(), this->GetItemCount(), this->GetItemCondition(), this->GetItemEquipped(), this->GetItemSilent(), this->GetItemStick());

	return packet;
}
