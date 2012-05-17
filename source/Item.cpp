#include "Item.h"
#include "Container.h"

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

Item::Item(const pDefault* packet) : Object(PacketFactory::ExtractPartial(packet))
{
	initialize();

	unsigned int count;
	double condition;
	bool equipped, silent, stick;

	PacketFactory::Access(packet, &count, &condition, &equipped, &silent, &stick);

	this->SetItemCount(count);
	this->SetItemCondition(condition);
	this->SetItemEquipped(equipped);
	this->SetItemSilent(silent);
	this->SetItemStick(stick);
}

Item::Item(pDefault* packet) : Item(static_cast<const pDefault*>(packet))
{
	PacketFactory::FreePacket(packet);
}

Item::~Item()
{

}

void Item::initialize()
{
	unsigned int baseID = this->GetBase();

	data = Container::Items->find(baseID);

	if (data == Container::Items->end())
		data = Container::Items->find(Reference::ResolveIndex(baseID));

	if (data != Container::Items->end())
		this->SetName(string(data->second));
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

bool Item::SetItemCount(unsigned int count)
{
	if (this->item_Count.get() == count)
		return false;

	if (!this->item_Count.set(count))
		return false;

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("Item count was set to %d (ref: %08X)", true, count, this->GetReference());

#endif
}

bool Item::SetItemCondition(double condition)
{
	if (this->item_Condition.get() == condition)
		return false;

	if (!this->item_Condition.set(condition))
		return false;

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("Item condition was set to %f (ref: %08X)", true, (float) condition, this->GetReference());

#endif
}

bool Item::SetItemEquipped(bool state)
{
	if (this->state_Equipped.get() == state)
		return false;

	if (!this->state_Equipped.set(state))
		return false;

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("Item equipped state was set to %d (ref: %08X)", true, (int) state, this->GetReference());

#endif

	return true;
}

bool Item::SetItemSilent(bool silent)
{
	if (this->flag_Silent.get() == silent)
		return false;

	if (!this->flag_Silent.set(silent))
		return false;

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("Item silent flag was set to %d (ref: %08X)", true, (int) silent, this->GetReference());

#endif

	return true;
}

bool Item::SetItemStick(bool stick)
{
	if (this->flag_Stick.get() == stick)
		return false;

	if (!this->flag_Stick.set(stick))
		return false;

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("Item stick flag was set to %d (ref: %08X)", true, (int) stick, this->GetReference());

#endif

	return true;
}

NetworkID Item::Copy() const
{
	FactoryObject reference = GameFactory::GetObject(GameFactory::CreateInstance(ID_ITEM, 0x00000000, this->GetBase()));
	Item* item = vaultcast<Item>(reference);

	item->SetItemCount(this->GetItemCount());
	item->SetItemCondition(this->GetItemCondition());
	item->SetItemEquipped(this->GetItemEquipped());
	item->SetItemSilent(this->GetItemSilent());
	item->SetItemStick(this->GetItemStick());

	return item->GetNetworkID();
}

pDefault* Item::toPacket()
{
	pDefault* pObjectNew = Object::toPacket();

	pDefault* packet = PacketFactory::CreatePacket(ID_ITEM_NEW, pObjectNew, this->GetItemCount(), this->GetItemCondition(), this->GetItemEquipped(), this->GetItemSilent(), this->GetItemStick());

	PacketFactory::FreePacket(pObjectNew);

	return packet;
}
