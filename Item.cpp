#include "Item.h"
#include "Container.h"

#ifdef VAULTMP_DEBUG
Debug* Item::debug;
#endif

#ifdef VAULTMP_DEBUG
void Item::SetDebugHandler( Debug* debug )
{
	Item::debug = debug;

	if ( debug != NULL )
		debug->Print( "Attached debug handler to Item class", true );
}
#endif

Item::Item( unsigned int refID, unsigned int baseID ) : Object( refID, baseID )
{
    initialize();
}

Item::Item(const pDefault* packet) : Object(PacketFactory::ExtractPartial(packet))
{
    initialize();

    unsigned int count;
    double condition;
    bool equipped;

    PacketFactory::Access(packet, &count, &condition, &equipped);

    this->SetItemCount(count);
    this->SetItemCondition(condition);
    this->SetItemEquipped(equipped);
}

Item::Item(pDefault* packet) : Item(reinterpret_cast<const pDefault*>(packet))
{
    PacketFactory::FreePacket(packet);
}

Item::~Item()
{

}

void Item::initialize()
{
    unsigned int baseID = this->GetBase();

	data = Container::Items->find( baseID );

	if ( data == Container::Items->end() )
		data = Container::Items->find( Reference::ResolveIndex( baseID ) );

	if ( data != Container::Items->end() )
		this->SetName( string( data->second ) );
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

bool Item::SetItemCount( unsigned int count )
{
	if ( this->item_Count.get() == count )
		return false;

	if ( !this->item_Count.set( count ) )
		return false;

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "Item count was set to %d (ref: %08X)", true, count, this->GetReference() );

#endif
}

bool Item::SetItemCondition( double condition )
{
	if ( this->item_Condition.get() == condition )
		return false;

	if ( !this->item_Condition.set( condition ) )
		return false;

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "Item condition was set to %f (ref: %08X)", true, ( float ) condition, this->GetReference() );

#endif
}

bool Item::SetItemEquipped( bool state )
{
	if ( this->state_Equipped.get() == state )
		return false;

	if ( !this->state_Equipped.set( state ) )
		return false;

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "Item equipped state was set to %d (ref: %08X)", true, ( int ) state, this->GetReference() );

#endif

	return true;
}

NetworkID Item::Copy() const
{
	FactoryObject reference = GameFactory::GetObject( GameFactory::CreateInstance( ID_ITEM, 0x00000000, this->GetBase() ) );
	Item* item = vaultcast<Item>( reference );

	item->SetItemCount( this->GetItemCount() );
	item->SetItemCondition( this->GetItemCondition() );
	item->SetItemEquipped( this->GetItemEquipped() );

	return item->GetNetworkID();
}

pDefault* Item::toPacket()
{
    pDefault* pObjectNew = Object::toPacket();

    pDefault* packet = PacketFactory::CreatePacket(ID_ITEM_NEW, pObjectNew, this->GetItemCount(), this->GetItemCondition(), this->GetItemEquipped());

    PacketFactory::FreePacket(pObjectNew);

    return packet;
}
