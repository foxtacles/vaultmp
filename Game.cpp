#include "Game.h"

unsigned char Game::game = 0x00;
RakNetGUID Game::server;

#ifdef VAULTMP_DEBUG
Debug* Game::debug = NULL;
#endif

using namespace Values;

#ifdef VAULTMP_DEBUG
void Game::SetDebugHandler( Debug* debug )
{
	Game::debug = debug;

	if ( debug != NULL )
		debug->Print( "Attached debug handler to Game class", true );
}
#endif

void Game::AdjustZAngle( double& Z, double diff )
{
	Z += diff;

	if ( Z > 360.0 )
		Z -= 360.0;

	else if ( Z < 0.00 )
		Z += 360.0;
}

void Game::Initialize()
{
	Interface::DefineCommand( "GetPos", "%0.GetPos %1" );
	Interface::DefineCommand( "GetPosNotSelf", "%0.GetPos %1", "GetPos" );
	Interface::DefineCommand( "SetPos", "%0.SetPos %1 %2" );
	Interface::DefineCommand( "GetAngle", "%0.GetAngle %1" );
	Interface::DefineCommand( "SetAngle", "%0.SetAngle %1 %2" );
	Interface::DefineCommand( "GetParentCell", "%0.GetParentCell" );
	Interface::DefineCommand( "GetControl", "GetControl %0" );
	Interface::DefineCommand( "GetBaseActorValue", "%0.GetBaseActorValue %1" );
	Interface::DefineCommand( "SetActorValue", "%0.SetActorValue %1 %2" );
	Interface::DefineCommand( "ForceActorValue", "%0.ForceActorValue %1 %2" );
	Interface::DefineCommand( "GetActorValue", "%0.GetActorValue %1" );
	Interface::DefineCommand( "GetActorValueHealth", "%0.GetActorValue %1", "GetActorValue" );
	Interface::DefineCommand( "GetDead", "%0.GetDead" );
	Interface::DefineCommand( "Enable", "%0.Enable %1" );
	Interface::DefineCommand( "Disable", "%0.Disable %1" );
	Interface::DefineCommand( "MoveTo", "%0.MoveTo %1 %2 %3 %4" );
	Interface::DefineCommand( "SetRestrained", "%0.SetRestrained %1" );
	Interface::DefineCommand( "PlayGroup", "%0.PlayGroup %1 %2" );
	Interface::DefineCommand( "SetAlert", "%0.SetAlert %1" );
	Interface::DefineCommand( "SetForceSneak", "%0.SetForceSneak %1" );
	Interface::DefineCommand( "SetName", "%0.SetName %1" );
	Interface::DefineCommand( "EquipItem", "%0.EquipItem %1 %2 %3" );
	Interface::DefineCommand( "UnequipItem", "%0.UnequipItem %1 %2 %3" );
	Interface::DefineCommand( "AddItem", "%0.AddItem %1 %2 %3" );
	Interface::DefineCommand( "AddItemHealthPercent", "%0.AddItemHealthPercent %1 %2 %3 %4" );
	Interface::DefineCommand( "RemoveItem", "%0.RemoveItem %1 %2 %3" );
	Interface::DefineCommand( "RemoveAllItems", "%0.RemoveAllItems" );
	Interface::DefineCommand( "MoveAllItems", "%0.RemoveAllItems %1 %2", "RemoveAllItems" );
	Interface::DefineCommand( "Kill", "%0.Kill" );
	Interface::DefineCommand( "KillActor", "%0.Kill %1 %2 %3", "Kill" );
	Interface::DefineCommand( "PlaceAtMe", "%0.PlaceAtMe %1 %2" );
	Interface::DefineCommand( "Load", "Load %0" );

    Interface::DefineCommand( "IsMoving", "%0.IsMoving" );
    Interface::DefineCommand( "IsAnimPlaying", "%0.IsAnimPlaying %1" );
    Interface::DefineCommand( "MarkForDelete", "%0.MarkForDelete" );
    Interface::DefineCommand( "ScanContainer", "%0.ScanContainer" );

	Interface::DefineCommand( "GetActorState", "%0.GetActorState %1" );
	Interface::DefineCommand( "GetActorStateNotSelf", "%0.GetActorState", "GetActorState" );
}

NetworkResponse Game::Authenticate( string password )
{
	NetworkResponse response;
	FactoryObject reference = GameFactory::GetObject( PLAYER_REFERENCE );
	Player* self = vaultcast<Player>( reference );
	pDefault* packet = PacketFactory::CreatePacket( ID_GAME_AUTH, self->GetName().c_str(), password.c_str() );
	response = Network::CompleteResponse( Network::CreateResponse( packet,
										  ( unsigned char ) HIGH_PRIORITY,
										  ( unsigned char ) RELIABLE_ORDERED,
										  CHANNEL_GAME,
										  server ) );
	return response;
}

void Game::Startup()
{
	FactoryObject reference = GameFactory::GetObject( PLAYER_REFERENCE );
	Player* self = vaultcast<Player>( reference );

	Interface::StartSetup();

		ParamContainer param_GetPos;
		param_GetPos.push_back( self->GetReferenceParam() );
		param_GetPos.push_back( Object::Param_Axis() );
		Interface::DefineNative( "GetPos", param_GetPos );
		Interface::SetupCommand( "GetPos" );

		ParamContainer param_GetPos_NotSelf;
		param_GetPos_NotSelf.push_back( Player::CreateFunctor( FLAG_ENABLED | FLAG_NOTSELF | FLAG_ALIVE ) );
		param_GetPos_NotSelf.push_back( Object::Param_Axis() );
		Interface::DefineNative( "GetPosNotSelf", param_GetPos_NotSelf );
		Interface::SetupCommand( "GetPosNotSelf", 30 );

		ParamContainer param_GetAngle;
		param_GetAngle.push_back( self->GetReferenceParam() );
		param_GetAngle.push_back( BuildParameter( vector<string> {API::RetrieveAxis_Reverse( Axis_X ), API::RetrieveAxis_Reverse( Axis_Z )} ) ); // exclude Y-angle, not used
		Interface::DefineNative( "GetAngle", param_GetAngle );
		Interface::SetupCommand( "GetAngle" );

		ParamContainer param_GetActorState;
		param_GetActorState.push_back( self->GetReferenceParam() );
		param_GetActorState.push_back( Player::CreateFunctor( FLAG_MOVCONTROLS, self->GetNetworkID() ) );
		Interface::DefineNative( "GetActorState", param_GetActorState );
		Interface::SetupCommand( "GetActorState" );

		ParamContainer param_GetParentCell;
		param_GetParentCell.push_back( Player::CreateFunctor( 0x00000000 ) );
		Interface::DefineNative( "GetParentCell", param_GetParentCell );
		Interface::SetupCommand( "GetParentCell", 30 );

		ParamContainer param_ScanContainer;
		param_ScanContainer.push_back( self->GetReferenceParam() );
		Interface::DefineNative( "ScanContainer", param_ScanContainer );
		Interface::SetupCommand( "ScanContainer", 50 );

		ParamContainer param_GetDead;
		param_GetDead.push_back( Player::CreateFunctor( FLAG_ENABLED ) );
		Interface::DefineNative( "GetDead", param_GetDead );
		Interface::SetupCommand( "GetDead", 30 );

		vector<string> healthValues;

        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_Health ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_Head ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_Torso ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_LeftArm ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_RightArm ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_LeftLeg ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_RightLeg ) );

		ParamContainer param_GetActorValue_Health;
		//param_GetActorValue_Health.push_back(Player::Param_EnabledPlayers);
		param_GetActorValue_Health.push_back( self->GetReferenceParam() );
		param_GetActorValue_Health.push_back( BuildParameter( healthValues ) );
		Interface::DefineNative( "GetActorValueHealth", param_GetActorValue_Health );
		Interface::SetupCommand( "GetActorValueHealth", 30 );

		ParamContainer param_GetActorValue;
		param_GetActorValue.push_back( self->GetReferenceParam() );
		param_GetActorValue.push_back( Actor::Param_ActorValues() ); // we could exclude health values here
		Interface::DefineNative( "GetActorValue", param_GetActorValue );
		Interface::SetupCommand( "GetActorValue", 100 );

		ParamContainer param_GetBaseActorValue;
		param_GetBaseActorValue.push_back( self->GetReferenceParam() );
		param_GetBaseActorValue.push_back( Actor::Param_ActorValues() );
		Interface::DefineNative( "GetBaseActorValue", param_GetBaseActorValue );
		Interface::SetupCommand( "GetBaseActorValue", 200 );

	Interface::EndSetup();
}

template <typename T>
void Game::FutureSet( Lockable* data, T t )
{
	if ( data == NULL )
		throw VaultException( "Could not relocate reference storage" );

	Value<T>* store = dynamic_cast<Value<T>*>( data );

	if ( store == NULL )
		throw VaultException( "Reference storage is corrupted" );

	store->set(t);
	store->set_promise();
}
template void Game::FutureSet(Lockable* data, unsigned int t);
template void Game::FutureSet(Lockable* data, bool t);

void Game::LoadGame( string savegame )
{
	Utils::RemoveExtension( savegame );

    Value<bool>* store = new Value<bool>;
    signed int key = store->Lock( true );

	Interface::StartDynamic();

	ParamContainer param_Load;
	param_Load.push_back( BuildParameter( savegame ) );
	Interface::ExecuteCommand( "Load", param_Load, key );

	Interface::EndDynamic();

	bool ready;

    try
    {
        ready = store->get_future(chrono::seconds(15));
    }
    catch (exception& e)
    {
        delete store;
        throw VaultException( "Loading of savegame %s failed (%s)", savegame.c_str(), e.what() );
    }

    delete store;
    // ready state
}

void Game::LoadEnvironment()
{
	FactoryObject reference = GameFactory::GetObject( PLAYER_REFERENCE );

    // load environment

    SetName( reference );

    Interface::StartDynamic();

    ParamContainer param_GetControl;
    param_GetControl.push_back( BuildParameter( API::RetrieveAllControls() ) );
    Interface::ExecuteCommand( "GetControl", param_GetControl );

    Interface::EndDynamic();
}

void Game::NewObject( FactoryObject reference )
{
	Object* object = vaultcast<Object>(reference);

    if (!object->GetReference())
    {
        Value<unsigned int>* store = new Value<unsigned int>;
        signed int key = store->Lock( true );

        PlaceAtMe(PLAYER_REFERENCE, object->GetBase(), 1, key);

        unsigned int refID;

        try
        {
            refID = store->get_future(chrono::seconds(15));
        }
        catch (exception& e)
        {
            delete store;
            throw VaultException( "Object creation with baseID %08X and NetworkID %lld failed (%s)", object->GetBase(), object->GetNetworkID(), e.what() );
        }

        object->SetReference(refID);
        delete store;
    }
    else
    {
        // existing objects
    }

	SetName( reference );
	SetRestrained( reference, true );
	SetPos(reference);
	SetAngle(reference);

	// maybe more
}

void Game::NewItem( FactoryObject reference )
{
    NewObject(reference);

    // set condition
}

void Game::NewContainer( FactoryObject reference )
{
    NewObject(reference);
    RemoveAllItems(reference);

    Container* container = vaultcast<Container>(reference);
    vector<FactoryObject> items = GameFactory::GetMultiple(vector<NetworkID>(container->GetItemList().begin(), container->GetItemList().end()));
    vector<FactoryObject>::iterator it;

    for (it = items.begin(); it != items.end(); ++it)
    {
        AddItem(vector<FactoryObject>{reference, *it});
        Item* item = vaultcast<Item>(*it);

        if (item->GetItemEquipped())
            EquipItem(vector<FactoryObject>{reference, *it});
    }
}

void Game::NewActor( FactoryObject reference )
{
    NewObject(reference);
    NewContainer(reference);

    vector<unsigned char> values = API::RetrieveAllValues();
    vector<unsigned char>::iterator it;

    for (it = values.begin(); it != values.end(); ++it)
    {
        SetActorValue(reference, true, *it);
        SetActorValue(reference, false, *it);
    }

    SetActorAlerted(reference);
    SetActorSneaking(reference);
    SetActorMovingAnimation(reference);
}

void Game::NewPlayer( FactoryObject reference )
{
    NewObject(reference);
    NewContainer(reference);
    NewActor(reference);

    // ...
}

void Game::RemoveObject( FactoryObject& reference )
{
	Delete( reference );
}

void Game::PlaceAtMe( FactoryObject reference, unsigned int baseID, unsigned int count, signed int key )
{
    Container* container = vaultcast<Container>(reference);

	if ( !container )
		throw VaultException( "Object with reference %08X is not a Container", ( *reference )->GetReference() );

    PlaceAtMe(container->GetReference(), baseID, count, key);
}

void Game::PlaceAtMe( unsigned int refID, unsigned int baseID, unsigned int count, signed int key )
{
	Interface::StartDynamic();

    ParamContainer param_PlaceAtMe;
    param_PlaceAtMe.push_back(BuildParameter(Utils::LongToHex(refID)));
    param_PlaceAtMe.push_back(BuildParameter(Utils::LongToHex(baseID)));
    param_PlaceAtMe.push_back(BuildParameter(count));
    Interface::ExecuteCommand("PlaceAtMe", param_PlaceAtMe, key);

	Interface::EndDynamic();
}

void Game::ToggleEnabled( FactoryObject reference )
{
	Object* object = vaultcast<Object>( reference );

    Interface::StartDynamic();

    if (object->GetEnabled())
    {
        ParamContainer param_Enable;
        param_Enable.push_back( object->GetReferenceParam() );
        param_Enable.push_back( Data::Param_True );
        Interface::ExecuteCommand( "Enable", param_Enable );
    }
    else
    {
        ParamContainer param_Disable;
        param_Disable.push_back( object->GetReferenceParam() );
        param_Disable.push_back( Data::Param_False );
        Interface::ExecuteCommand( "Disable", param_Disable );
    }

    Interface::EndDynamic();
}

void Game::Delete( FactoryObject& reference )
{
    Object* object = vaultcast<Object>(reference);

    if (object->SetEnabled(false))
        ToggleEnabled( reference);

    Interface::StartDynamic();

    ParamContainer param_MarkForDelete;
    param_MarkForDelete.push_back( object->GetReferenceParam() );
    Interface::ExecuteCommand( "MarkForDelete", param_MarkForDelete );

    Interface::EndDynamic();

	GameFactory::DestroyInstance( reference );
}

void Game::SetName( FactoryObject reference )
{
	Object* object = vaultcast<Object>( reference );
	string name = object->GetName();

	Interface::StartDynamic();

	ParamContainer param_SetName;
	param_SetName.push_back( object->GetReferenceParam() );
	param_SetName.push_back( BuildParameter( object->GetName() ) );
	Interface::ExecuteCommand( "SetName", param_SetName );

	Interface::EndDynamic();
}

void Game::SetRestrained(FactoryObject reference, bool restrained)
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

    //bool restrained = actor->GetActorRestrained();

	Interface::StartDynamic();

    ParamContainer param_SetRestrained;
    param_SetRestrained.push_back(actor->GetReferenceParam());
    param_SetRestrained.push_back(restrained ? Data::Param_True : Data::Param_False);
    Interface::ExecuteCommand("SetRestrained", param_SetRestrained);

	Interface::EndDynamic();
}

void Game::SetPos( FactoryObject reference )
{
	Object* object = vaultcast<Object>( reference );

    Lockable* key = NULL;

    Interface::StartDynamic();

    key = object->SetGamePos( Axis_X, object->GetNetworkPos( Axis_X ) );

    ParamContainer param_SetPos;
    param_SetPos.push_back(object->GetReferenceParam());
    param_SetPos.push_back(BuildParameter(API::RetrieveAxis_Reverse(Axis_X)));
    param_SetPos.push_back(BuildParameter(object->GetNetworkPos(Axis_X)));
    Interface::ExecuteCommand("SetPos", param_SetPos, key ? key->Lock(true) : 0);

    key = object->SetGamePos( Axis_Y, object->GetNetworkPos( Axis_Y ) );

    param_SetPos.clear();
    param_SetPos.push_back(object->GetReferenceParam());
    param_SetPos.push_back(BuildParameter(API::RetrieveAxis_Reverse(Axis_Y)));
    param_SetPos.push_back(BuildParameter(object->GetNetworkPos(Axis_Y)));
    Interface::ExecuteCommand("SetPos", param_SetPos, key ? key->Lock(true) : 0);

    key = object->SetGamePos( Axis_Z, object->GetNetworkPos( Axis_Z ) );

    param_SetPos.clear();
    param_SetPos.push_back(object->GetReferenceParam());
    param_SetPos.push_back(BuildParameter(API::RetrieveAxis_Reverse(Axis_Z)));
    param_SetPos.push_back(BuildParameter(object->GetNetworkPos(Axis_Z)));
    Interface::ExecuteCommand("SetPos", param_SetPos, key ? key->Lock(true) : 0);

    Interface::EndDynamic();
}

void Game::SetAngle( FactoryObject reference )
{
	Object* object = vaultcast<Object>( reference );

	Interface::StartDynamic();

	ParamContainer param_SetAngle;
	param_SetAngle.push_back( object->GetReferenceParam() );
	param_SetAngle.push_back( BuildParameter( API::RetrieveAxis_Reverse( Axis_X ) ) );
	param_SetAngle.push_back( BuildParameter( object->GetAngle(Axis_X) ) );
	Interface::ExecuteCommand( "SetAngle", param_SetAngle );

	param_SetAngle.clear();

	param_SetAngle.push_back( object->GetReferenceParam() );
	param_SetAngle.push_back( BuildParameter( API::RetrieveAxis_Reverse( Axis_Z ) ) );

	double value = object->GetAngle( Axis_Z );
	Actor* actor = vaultcast<Actor>( object );

	if ( actor )
	{
		if ( actor->GetActorMovingXY() == 0x01 )
			AdjustZAngle( value, -45.0 );

		else if ( actor->GetActorMovingXY() == 0x02 )
			AdjustZAngle( value, 45.0 );
	}

	param_SetAngle.push_back( BuildParameter( object->GetAngle(Axis_Z) ) );
	Interface::ExecuteCommand( "SetAngle", param_SetAngle );

	Interface::EndDynamic();
}

void Game::MoveTo( vector<FactoryObject> reference, bool cell, signed int key )
{
	Object* object = vaultcast<Object>( reference[0] );
	Object* object2 = vaultcast<Object>( reference[1] );

    Interface::StartDynamic();

    ParamContainer param_MoveTo;
    param_MoveTo.push_back( object->GetReferenceParam() );
    param_MoveTo.push_back( object2->GetReferenceParam() );

    if ( cell )
    {
        param_MoveTo.push_back( BuildParameter( object->GetNetworkPos( Axis_X ) - object2->GetNetworkPos( Axis_X ) ) );
        param_MoveTo.push_back( BuildParameter( object->GetNetworkPos( Axis_Y ) - object2->GetNetworkPos( Axis_Y ) ) );
        param_MoveTo.push_back( BuildParameter( object->GetNetworkPos( Axis_Z ) - object2->GetNetworkPos( Axis_Z ) ) );
    }

    Interface::ExecuteCommand( "MoveTo", param_MoveTo, key );

    Interface::EndDynamic();
}

void Game::SetActorValue( FactoryObject reference, bool base, unsigned char index, signed int key)
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

    Interface::StartDynamic();

	if ( base )
	{
        ParamContainer param_SetActorValue;
        param_SetActorValue.push_back(actor->GetReferenceParam());
        param_SetActorValue.push_back(BuildParameter(API::RetrieveValue_Reverse(index)));
        param_SetActorValue.push_back(BuildParameter(actor->GetActorBaseValue(index)));
        Interface::ExecuteCommand("SetActorValue", param_SetActorValue, key);
	}
	else
	{
        ParamContainer param_ForceActorValue;
        param_ForceActorValue.push_back(actor->GetReferenceParam());
        param_ForceActorValue.push_back(BuildParameter(API::RetrieveValue_Reverse(index)));
        param_ForceActorValue.push_back(BuildParameter(actor->GetActorValue(index)));
        Interface::ExecuteCommand("ForceActorValue", param_ForceActorValue, key);
	}

    Interface::EndDynamic();
}

void Game::SetActorSneaking(FactoryObject reference, signed int key)
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

    Interface::StartDynamic();

    ParamContainer param_SetForceSneak;
    param_SetForceSneak.push_back(actor->GetReferenceParam());
    param_SetForceSneak.push_back(actor->GetActorSneaking() ? Data::Param_True : Data::Param_False);
    Interface::ExecuteCommand("SetForceSneak", param_SetForceSneak, key);

    Interface::EndDynamic();
}

void Game::SetActorAlerted(FactoryObject reference, signed int key)
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

    Interface::StartDynamic();

    ParamContainer param_SetAlert;
    param_SetAlert.push_back(actor->GetReferenceParam());
    param_SetAlert.push_back(actor->GetActorAlerted() ? Data::Param_True : Data::Param_False);
    Interface::ExecuteCommand("SetAlert", param_SetAlert, key);

    Interface::EndDynamic();
}

void Game::SetActorMovingAnimation(FactoryObject reference, signed int key)
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

    Interface::StartDynamic();

    ParamContainer param_PlayGroup;
    param_PlayGroup.push_back(actor->GetReferenceParam());
    param_PlayGroup.push_back(BuildParameter(API::RetrieveAnim_Reverse(actor->GetActorMovingAnimation())));
    param_PlayGroup.push_back(Data::Param_True);
    Interface::ExecuteCommand("PlayGroup", param_PlayGroup, key);

    Interface::EndDynamic();
}

void Game::AddItem( vector<FactoryObject> reference, bool silent )
{
	Item* item = vaultcast<Item>( reference[1] );

	if ( !item )
		throw VaultException( "Object with reference %08X is not an Item", ( *reference[1] )->GetReference() );

    AddItem(reference[0], item->GetBase(), item->GetItemCount(), item->GetItemCondition(), silent);
}

void Game::AddItem( FactoryObject reference, unsigned int baseID, unsigned int count, double condition, bool silent )
{
    Container* container = vaultcast<Container>( reference );

	if ( !container )
		throw VaultException( "Object with reference %08X is not a Container", ( *reference )->GetReference() );

    Interface::StartDynamic();

    /*if (!condition || Utils::DoubleCompare(condition, 100.0, 0.01))
    {
        ParamContainer param_AddItem;
        param_AddItem.push_back( container->GetReferenceParam() );
        param_AddItem.push_back( BuildParameter(Utils::LongToHex(baseID)) );
        param_AddItem.push_back( BuildParameter(count) );
        param_AddItem.push_back( silent ? Data::Param_True : Data::Param_False );

        Interface::ExecuteCommand( "AddItem", param_AddItem);
    }
    else
    {*/
        // AddItemHealthPercent doesn't has the "equip best stuff"-bug
        ParamContainer param_AddItemHealthPercent;
        param_AddItemHealthPercent.push_back( container->GetReferenceParam() );
        param_AddItemHealthPercent.push_back( BuildParameter(Utils::LongToHex(baseID)) );
        param_AddItemHealthPercent.push_back( BuildParameter(count) );
        param_AddItemHealthPercent.push_back( BuildParameter(condition / 100) );
        param_AddItemHealthPercent.push_back( silent ? Data::Param_True : Data::Param_False );

        Interface::ExecuteCommand( "AddItemHealthPercent", param_AddItemHealthPercent);
    //}

    Interface::EndDynamic();
}

void Game::RemoveItem( vector<FactoryObject> reference, bool silent )
{
	Item* item = vaultcast<Item>( reference[1] );

	if ( !item )
		throw VaultException( "Object with reference %08X is not an Item", ( *reference[1] )->GetReference() );

    RemoveItem(reference[0], item->GetBase(), item->GetItemCount(), silent);
}

void Game::RemoveItem( FactoryObject reference, unsigned int baseID, unsigned int count, bool silent )
{
	Container* container = vaultcast<Container>( reference );

	if ( !container )
		throw VaultException( "Object with reference %08X is not a Container", ( *reference )->GetReference() );

    Interface::StartDynamic();

    ParamContainer param_RemoveItem;
    param_RemoveItem.push_back( container->GetReferenceParam() );
    param_RemoveItem.push_back( BuildParameter(Utils::LongToHex(baseID)) );
    param_RemoveItem.push_back( BuildParameter(count) );
    param_RemoveItem.push_back( silent ? Data::Param_True : Data::Param_False );

    Interface::ExecuteCommand( "RemoveItem", param_RemoveItem);

    Interface::EndDynamic();
}

void Game::RemoveAllItems( FactoryObject reference )
{
	Container* container = vaultcast<Container>( reference );

	if ( !container )
		throw VaultException( "Object with reference %08X is not a Container", ( *reference )->GetReference() );

    Interface::StartDynamic();

    ParamContainer param_RemoveAllItems;
    param_RemoveAllItems.push_back( container->GetReferenceParam() );

    Interface::ExecuteCommand( "RemoveAllItems", param_RemoveAllItems);

    Interface::EndDynamic();
}

void Game::EquipItem( vector<FactoryObject> reference, bool stick, bool silent )
{
	Item* item = vaultcast<Item>( reference[1] );

	if ( !item )
		throw VaultException( "Object with reference %08X is not an Item", ( *reference[1] )->GetReference() );

    RemoveItem(reference[0], item->GetBase(), stick, silent);
}

void Game::EquipItem( FactoryObject reference, unsigned int baseID, bool stick, bool silent )
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

    Interface::StartDynamic();

    ParamContainer param_EquipItem;
    param_EquipItem.push_back( actor->GetReferenceParam() );
    param_EquipItem.push_back( BuildParameter(Utils::LongToHex(baseID)) );
    param_EquipItem.push_back( stick ? Data::Param_True : Data::Param_False );
    param_EquipItem.push_back( silent ? Data::Param_True : Data::Param_False );

    Interface::ExecuteCommand( "EquipItem", param_EquipItem);

    Interface::EndDynamic();
}

void Game::UnequipItem( vector<FactoryObject> reference, bool stick, bool silent )
{
	Item* item = vaultcast<Item>( reference[1] );

	if ( !item )
		throw VaultException( "Object with reference %08X is not an Item", ( *reference[1] )->GetReference() );

    RemoveItem(reference[0], item->GetBase(), stick, silent);
}

void Game::UnequipItem( FactoryObject reference, unsigned int baseID, bool stick, bool silent )
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

    Interface::StartDynamic();

    ParamContainer param_UnequipItem;
    param_UnequipItem.push_back( actor->GetReferenceParam() );
    param_UnequipItem.push_back( BuildParameter(Utils::LongToHex(baseID)) );
    param_UnequipItem.push_back( stick ? Data::Param_True : Data::Param_False );
    param_UnequipItem.push_back( silent ? Data::Param_True : Data::Param_False );

    Interface::ExecuteCommand( "UnequipItem", param_UnequipItem);

    Interface::EndDynamic();
}

void Game::net_SetPos( FactoryObject reference, double X, double Y, double Z)
{
    Object* object = vaultcast<Object>( reference );
	bool result = ( ( bool ) object->SetNetworkPos( Axis_X, X ) | ( bool ) object->SetNetworkPos( Axis_Y, Y ) | ( bool ) object->SetNetworkPos( Axis_Z, Z ) );

	if ( result && object->GetEnabled())
	{
	    Actor* actor = vaultcast<Actor>( object ); // maybe we should consider items, too (they have physics)

        if ( actor == NULL || ( !actor->IsNearPoint( object->GetNetworkPos( Axis_X ), object->GetNetworkPos( Axis_Y ), object->GetNetworkPos( Axis_Z ), 200.0 ) && actor->GetActorMovingAnimation() == AnimGroup_Idle ) || actor->IsActorJumping() )
            SetPos( reference );
	}
}

void Game::net_SetAngle( FactoryObject reference, unsigned char axis, double value )
{
	Object* object = vaultcast<Object>( reference );
	bool result = ( bool ) object->SetAngle( axis, value );

	if ( result && object->GetEnabled() )
		SetAngle( reference );
}

void Game::net_SetCell( vector<FactoryObject> reference, unsigned int cell )
{
	Object* object = vaultcast<Object>( reference[0] );
	Player* self = vaultcast<Player>( reference[1] );

	if ( !self )
		throw VaultException( "Object with reference %08X is not a Player", ( *reference[1] )->GetReference() );

	object->SetNetworkCell( cell );

	if ( object != self )
	{
		if ( object->GetNetworkCell() != self->GetGameCell() )
		{
		    if (object->SetEnabled(false))
                ToggleEnabled( reference[0]);
		}
		else
		{
		    if (object->SetEnabled(true))
                ToggleEnabled( reference[0]);
		}
	}
}

void Game::net_ContainerUpdate( FactoryObject reference, ContainerDiff diff )
{
    Container* container = vaultcast<Container>(reference);

	if ( !container )
		throw VaultException( "Object with reference %08X is not a Container", ( *reference )->GetReference() );

    GameDiff gamediff = container->ApplyDiff(diff);
    GameDiff::iterator it;

    for (it = gamediff.begin(); it != gamediff.end(); ++it)
    {
        if (it->second.equipped)
        {
            if (it->second.equipped > 0)
                EquipItem(reference, it->first);
            else if (it->second.equipped < 0)
                UnequipItem(reference, it->first);
        }
        else if (it->second.count > 0)
            AddItem(reference, it->first, it->second.count, it->second.condition);
        else if (it->second.count < 0)
            RemoveItem(reference, it->first, abs(it->second.count));
        //else
            // new condition, can't handle yet
    }
}

void Game::net_SetActorValue( FactoryObject reference, bool base, unsigned char index, double value )
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

	Lockable* result;

	if ( base )
		result = actor->SetActorBaseValue( index, value );
	else
		result = actor->SetActorValue( index, value );

    if (result)
        SetActorValue(reference, base, index, result->Lock(true));
}

void Game::net_SetActorState( FactoryObject reference, unsigned char index, unsigned char moving, bool alerted, bool sneaking )
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

	Lockable* result;

	result = actor->SetActorMovingXY( moving );

	if ( result && actor->GetEnabled() )
		SetAngle( reference);

	result = actor->SetActorAlerted( alerted );

	if ( result && actor->GetEnabled() )
	{
		SetRestrained( reference, false );
		signed int key = result->Lock( true );
        SetActorAlerted(reference, key);
        SetRestrained(reference, true);
    }

	result = actor->SetActorSneaking( sneaking );

	if ( result && actor->GetEnabled() )
	{
		SetRestrained( reference, false );
		signed int key = result->Lock( true );
        SetActorSneaking(reference, key);
		SetRestrained( reference, true );
	}

	result = actor->SetActorMovingAnimation( index );

	if ( result && actor->GetEnabled() )
	{
		signed int key = result->Lock( true );
        SetActorMovingAnimation(reference, key);

		if ( index == AnimGroup_Idle )
			SetPos( reference );
	}
}

void Game::GetPos( FactoryObject reference, unsigned char axis, double value )
{
	Object* object = vaultcast<Object>( reference );
	bool result = ( bool ) object->SetGamePos( axis, value );

	if ( result && axis == Axis_Z && object->GetReference() == PLAYER_REFERENCE )
	{
		pDefault* packet = PacketFactory::CreatePacket( ID_UPDATE_POS, object->GetNetworkID(), object->GetGamePos( Axis_X ), object->GetGamePos( Axis_Y ), object->GetGamePos( Axis_Z ) );
		NetworkResponse response = Network::CompleteResponse( Network::CreateResponse( packet,
								   ( unsigned char ) HIGH_PRIORITY,
								   ( unsigned char ) RELIABLE_SEQUENCED,
								   CHANNEL_GAME,
								   server ) );
		Network::Queue( response );
	}
}

void Game::GetAngle( FactoryObject reference, unsigned char axis, double value )
{
	Object* object = vaultcast<Object>( reference );
	bool result = ( bool ) object->SetAngle( axis, value );

	if ( result )
	{
		pDefault* packet = PacketFactory::CreatePacket( ID_UPDATE_ANGLE, object->GetNetworkID(), axis, value );
		NetworkResponse response = Network::CompleteResponse( Network::CreateResponse( packet,
								   ( unsigned char ) HIGH_PRIORITY,
								   ( unsigned char ) RELIABLE_SEQUENCED,
								   CHANNEL_GAME,
								   server ) );
		Network::Queue( response );
	}
}

void Game::GetParentCell( vector<FactoryObject> reference, unsigned int cell )
{
	Object* object = vaultcast<Object>( reference[0] );
	Player* self = vaultcast<Player>( reference[1] );

	if ( !self )
		throw VaultException( "Object with reference %08X is not a Player", ( *reference[1] )->GetReference() );

	if ( object != self )
	{
		if ( self->GetGameCell() == object->GetNetworkCell() && object->GetGameCell() != object->GetNetworkCell() )
		{
		    if (object->SetEnabled(true))
                ToggleEnabled(reference[0]);

            Lockable* result = object->SetGameCell( self->GetGameCell() );

            if (result)
                MoveTo( reference, true, result->Lock(true) );

			SetAngle( reference[0] );
		}
	}

	bool result = ( bool ) object->SetGameCell( cell );

	if ( object != self )
	{
		if ( object->GetNetworkCell() != self->GetGameCell() )
		{
		    if (object->SetEnabled(false))
                ToggleEnabled( reference[0]);
		}
		else
		{
		    if (object->SetEnabled(true))
                ToggleEnabled( reference[0]);
		}
	}

	if ( result && object == self )
	{
		pDefault* packet = PacketFactory::CreatePacket( ID_UPDATE_CELL, object->GetNetworkID(), cell );
		NetworkResponse response = Network::CompleteResponse( Network::CreateResponse( packet,
								   ( unsigned char ) HIGH_PRIORITY,
								   ( unsigned char ) RELIABLE_SEQUENCED,
								   CHANNEL_GAME,
								   server ) );
		Network::Queue( response );
	}
}

void Game::GetActorValue( FactoryObject reference, bool base, unsigned char index, double value )
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

	bool result;

	if ( base )
		result = ( bool ) actor->SetActorBaseValue( index, value );

	else
		result = ( bool ) actor->SetActorValue( index, value );

	if ( result )
	{
		pDefault* packet = PacketFactory::CreatePacket( ID_UPDATE_VALUE, actor->GetNetworkID(), base, index, value );
		NetworkResponse response = Network::CompleteResponse( Network::CreateResponse( packet,
								   ( unsigned char ) HIGH_PRIORITY,
								   ( unsigned char ) RELIABLE_ORDERED,
								   CHANNEL_GAME,
								   server ) );
		Network::Queue( response );
	}
}

void Game::GetActorState( FactoryObject reference, unsigned char index, unsigned char moving, bool alerted, bool sneaking )
{
	Actor* actor  = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

	bool result;

	if ( index == 0xFF )
		index = AnimGroup_Idle;

	result = ( ( bool ) actor->SetActorMovingAnimation( index ) | ( bool ) actor->SetActorMovingXY( moving ) | ( bool ) actor->SetActorAlerted( alerted ) | ( bool ) actor->SetActorSneaking( sneaking ) );

	if ( result )
	{
		pDefault* packet = PacketFactory::CreatePacket( ID_UPDATE_STATE, actor->GetNetworkID(), index, moving, alerted, sneaking );
		NetworkResponse response = Network::CompleteResponse( Network::CreateResponse( packet,
								   ( unsigned char ) HIGH_PRIORITY,
								   ( unsigned char ) RELIABLE_ORDERED,
								   CHANNEL_GAME,
								   server ) );
		Network::Queue( response );
	}
}

void Game::GetControl( FactoryObject reference, unsigned char control, unsigned char key )
{
	Player* player = vaultcast<Player>( reference );

	if ( !player )
		throw VaultException( "Object with reference %08X is not a Player", ( *reference )->GetReference() );

	bool result;

	result = ( bool ) player->SetPlayerControl( control, key );

	if ( result )
	{
		pDefault* packet = PacketFactory::CreatePacket( ID_UPDATE_CONTROL, player->GetNetworkID(), control, key);
		NetworkResponse response = Network::CompleteResponse( Network::CreateResponse( packet,
								   ( unsigned char ) HIGH_PRIORITY,
								   ( unsigned char ) RELIABLE_ORDERED,
								   CHANNEL_GAME,
								   server ) );
		Network::Queue( response );
	}
}

void Game::ScanContainer( FactoryObject reference, vector<unsigned char>& data )
{
	Container* container = vaultcast<Container>( reference );

	if ( !container )
		throw VaultException( "Object with reference %08X is not a Container", ( *reference )->GetReference() );

#pragma pack(push, 1)
	struct ItemInfo
	{
		unsigned int baseID;
		unsigned int amount;
		unsigned int equipped;
		double condition;
	};
#pragma pack(pop)

	ItemInfo* items = reinterpret_cast<ItemInfo*>( &data[0] );
	unsigned int count = data.size() / sizeof( ItemInfo );

	FactoryObject _temp = GameFactory::GetObject( GameFactory::CreateInstance( ID_CONTAINER, 0x00000000 ) );
	Container* temp = vaultcast<Container>( _temp );

	for ( unsigned int i = 0; i < count; ++i )
	{
		FactoryObject _item = GameFactory::GetObject( GameFactory::CreateInstance( ID_ITEM, items[i].baseID ) );
		Item* item = vaultcast<Item>( _item );
		item->SetItemCount( items[i].amount );
		item->SetItemEquipped( ( bool ) items[i].equipped );
		item->SetItemCondition( items[i].condition );
		temp->AddItem( item->GetNetworkID() );
	}

	ContainerDiff diff = container->Compare( temp->GetNetworkID() );

	if ( !diff.first.empty() || !diff.second.empty() )
	{
		pDefault* packet = PacketFactory::CreatePacket( ID_UPDATE_CONTAINER, container->GetNetworkID(), &diff);
		NetworkResponse response = Network::CompleteResponse( Network::CreateResponse( packet,
								   ( unsigned char ) HIGH_PRIORITY,
								   ( unsigned char ) RELIABLE_ORDERED,
								   CHANNEL_GAME,
								   server ) );
		Network::Queue( response );

		container->ApplyDiff( diff );
	}

	GameFactory::DestroyInstance( _temp );
}
