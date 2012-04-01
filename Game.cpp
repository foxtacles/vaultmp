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
	Interface::DefineCommand( "RemoveItem", "%0.RemoveItem %1 %2 %3" );
	Interface::DefineCommand( "RemoveAllItems", "%0.RemoveAllItems" );
	Interface::DefineCommand( "MoveAllItems", "%0.RemoveAllItems %1 %2", "RemoveAllItems" );
	Interface::DefineCommand( "Kill", "%0.Kill" );
	Interface::DefineCommand( "KillActor", "%0.Kill %1 %2 %3", "Kill" );
	Interface::DefineCommand( "PlaceAtMe", "%0.PlaceAtMe %1 %2 %3 %4" );
	Interface::DefineCommand( "Load", "Load %0" );

    Interface::DefineCommand( "IsMoving", "%0.IsMoving" );
    Interface::DefineCommand( "IsAnimPlaying", "%0.IsAnimPlaying %1" );
    Interface::DefineCommand( "MarkForDelete", "%0.MarkForDelete" );
    Interface::DefineCommand( "ScanContainer", "%0.ScanContainer" );

	Interface::DefineCommand( "GetActorState", "%0.GetActorState %1" );
	Interface::DefineCommand( "GetActorStateNotSelf", "%0.GetActorState", "GetActorState" );
}

void Game::Startup()
{
	FactoryObject reference = GameFactory::GetObject( PLAYER_REFERENCE );
	Player* self = vaultcast<Player>( reference );

	Interface::StartSession();

	try
	{
		ParamList param_GetPos;
		param_GetPos.push_back( self->GetReferenceParam() );
		param_GetPos.push_back( Object::Param_Axis() );
		ParamContainer GetPos = ParamContainer( param_GetPos, &Data::AlwaysTrue );
		Interface::DefineNative( "GetPos", GetPos );
		Interface::ExecuteCommandLoop( "GetPos" );

		ParamList param_GetPos_NotSelf;
		param_GetPos_NotSelf.push_back( Player::CreateFunctor( FLAG_ENABLED | FLAG_NOTSELF | FLAG_ALIVE ) );
		param_GetPos_NotSelf.push_back( Object::Param_Axis() );
		ParamContainer GetPos_NotSelf = ParamContainer( param_GetPos_NotSelf, &Data::AlwaysTrue );
		Interface::DefineNative( "GetPosNotSelf", GetPos_NotSelf );
		Interface::ExecuteCommandLoop( "GetPosNotSelf", 30 );

		ParamList param_GetAngle;
		param_GetAngle.push_back( self->GetReferenceParam() );
		param_GetAngle.push_back( BuildParameter( vector<string> {API::RetrieveAxis_Reverse( Axis_X ), API::RetrieveAxis_Reverse( Axis_Z )} ) ); // exclude Y-angle, not used
		ParamContainer GetAngle = ParamContainer( param_GetAngle, &Data::AlwaysTrue );
		Interface::DefineNative( "GetAngle", GetAngle );
		Interface::ExecuteCommandLoop( "GetAngle" );

		ParamList param_GetActorState;
		param_GetActorState.push_back( self->GetReferenceParam() );
		param_GetActorState.push_back( Player::CreateFunctor( FLAG_MOVCONTROLS, self->GetNetworkID() ) );
		ParamContainer GetActorState = ParamContainer( param_GetActorState, &Data::AlwaysTrue );
		Interface::DefineNative( "GetActorState", GetActorState );
		Interface::ExecuteCommandLoop( "GetActorState" );

		ParamList param_GetParentCell;
		param_GetParentCell.push_back( Player::CreateFunctor( 0x00000000 ) );
		ParamContainer GetParentCell = ParamContainer( param_GetParentCell, &Data::AlwaysTrue );
		Interface::DefineNative( "GetParentCell", GetParentCell );
		Interface::ExecuteCommandLoop( "GetParentCell", 30 );

		ParamList param_ScanContainer;
		param_ScanContainer.push_back( self->GetReferenceParam() );
		ParamContainer ScanContainer = ParamContainer( param_ScanContainer, &Data::AlwaysTrue );
		Interface::DefineNative( "ScanContainer", ScanContainer );
		Interface::ExecuteCommandLoop( "ScanContainer", 50 );

		ParamList param_GetDead;
		param_GetDead.push_back( Player::CreateFunctor( FLAG_ENABLED ) );
		ParamContainer GetDead = ParamContainer( param_GetDead, &Data::AlwaysTrue );
		Interface::DefineNative( "GetDead", GetDead );
		Interface::ExecuteCommandLoop( "GetDead", 30 );

		vector<string> healthValues;

        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_Health ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_Head ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_Torso ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_LeftArm ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_RightArm ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_LeftLeg ) );
        healthValues.push_back( API::RetrieveValue_Reverse( Fallout::ActorVal_RightLeg ) );

		ParamList param_GetActorValue_Health;
		//param_GetActorValue_Health.push_back(Player::Param_EnabledPlayers);
		param_GetActorValue_Health.push_back( self->GetReferenceParam() );
		param_GetActorValue_Health.push_back( BuildParameter( healthValues ) );
		ParamContainer GetActorValue_Health = ParamContainer( param_GetActorValue_Health, &Data::AlwaysTrue );
		Interface::DefineNative( "GetActorValueHealth", GetActorValue_Health );
		Interface::ExecuteCommandLoop( "GetActorValueHealth", 30 );

		ParamList param_GetActorValue;
		param_GetActorValue.push_back( self->GetReferenceParam() );
		param_GetActorValue.push_back( Actor::Param_ActorValues() ); // we could exclude health values here
		ParamContainer GetActorValue = ParamContainer( param_GetActorValue, &Data::AlwaysTrue );
		Interface::DefineNative( "GetActorValue", GetActorValue );
		Interface::ExecuteCommandLoop( "GetActorValue", 100 );

		ParamList param_GetBaseActorValue;
		param_GetBaseActorValue.push_back( self->GetReferenceParam() );
		param_GetBaseActorValue.push_back( Actor::Param_ActorValues() );
		ParamContainer GetBaseActorValue = ParamContainer( param_GetBaseActorValue, &Data::AlwaysTrue );
		Interface::DefineNative( "GetBaseActorValue", GetBaseActorValue );
		Interface::ExecuteCommandLoop( "GetBaseActorValue", 200 );

		ParamList param_GetControl;
		param_GetControl.push_back( BuildParameter( API::RetrieveAllControls() ) );
		ParamContainer GetControl = ParamContainer( param_GetControl, &Data::AlwaysTrue );
		Interface::ExecuteCommandOnce( "GetControl", GetControl );
	}

	catch ( ... )
	{
		Interface::EndSession();
		throw;
	}

	Interface::EndSession();
}

void Game::LoadGame( string savegame )
{
	Utils::RemoveExtension( savegame );

	Interface::StartSession();

	ParamList param_Load;
	param_Load.push_back( BuildParameter( savegame ) );
	ParamContainer Load = ParamContainer( param_Load, &Data::AlwaysTrue );
	Interface::ExecuteCommandOnce( "Load", Load );

	Interface::EndSession();
}

void Game::NewPlayer( NetworkID id, unsigned int baseID, string name )
{
	FactoryObject reference = GameFactory::GetObject( PLAYER_REFERENCE );
	Player* self = vaultcast<Player>( reference );

	Value<unsigned int>* store = new Value<unsigned int>;
	signed int key = store->Lock( true );

	Interface::StartSession();

    ParamList param_PlaceAtMe;
    param_PlaceAtMe.push_back(self->GetReferenceParam()); // need something else here
    param_PlaceAtMe.push_back(BuildParameter(Utils::LongToHex(baseID)));
    param_PlaceAtMe.push_back(Data::Param_True);
    ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, key);

	Interface::EndSession();

    unsigned int refID;

    try
    {
        refID = store->get_future(chrono::seconds(15));
    }
    catch (...)
    {
        delete store;
        throw VaultException( "Player creation with baseID %08X and NetworkID %lld failed", baseID, id );
    }

	GameFactory::CreateKnownInstance( ID_PLAYER, id, baseID );
	reference = GameFactory::GetObject( id );
	Player* player = vaultcast<Player>( reference );
	player->SetReference( refID );
	delete store;

	SetName( reference, name );
	SetRestrained( reference, true );
}

void Game::PlayerLeft( FactoryObject& reference )
{
	if ( !vaultcast<Player>( reference ) )
		throw VaultException( "Object with reference %08X is not a Player", ( *reference )->GetReference() );

	Delete( reference );
}

void Game::Enable( FactoryObject reference, bool enable )
{
	Object* object = vaultcast<Object>( reference );
	bool result = ( bool ) object->SetEnabled( enable );

	if ( result )
	{
		Interface::StartSession();

		if ( enable )
		{
			ParamList param_Enable;
			param_Enable.push_back( object->GetReferenceParam() );
			param_Enable.push_back( Data::Param_True );
			ParamContainer Enable = ParamContainer( param_Enable, &Data::AlwaysTrue );
			Interface::ExecuteCommandOnce( "Enable", Enable );
		}

		else
		{
			ParamList param_Disable;
			param_Disable.push_back( object->GetReferenceParam() );
			param_Disable.push_back( Data::Param_False );
			ParamContainer Disable = ParamContainer( param_Disable, &Data::AlwaysTrue );
			Interface::ExecuteCommandOnce( "Disable", Disable );
		}

		Interface::EndSession();
	}
}

void Game::Delete( FactoryObject& reference )
{
	Enable( reference, false );

	Object* object = vaultcast<Object>( reference );

    Interface::StartSession();

    ParamList param_MarkForDelete;
    param_MarkForDelete.push_back( object->GetReferenceParam() );
    ParamContainer MarkForDelete = ParamContainer( param_MarkForDelete, &Data::AlwaysTrue );
    Interface::ExecuteCommandOnce( "MarkForDelete", MarkForDelete );

    Interface::EndSession();

	GameFactory::DestroyInstance( reference );
}

void Game::SetName( FactoryObject reference, string name )
{
	Object* object = vaultcast<Object>( reference );

	object->SetName( name );

	Interface::StartSession();

	ParamList param_SetName;
	param_SetName.push_back( object->GetReferenceParam() );
	param_SetName.push_back( BuildParameter( object->GetName() ) );
	ParamContainer SetName = ParamContainer( param_SetName, &Data::AlwaysTrue );
	Interface::ExecuteCommandOnce( "SetName", SetName );

	Interface::EndSession();
}

void Game::SetRestrained(FactoryObject reference, bool restrained)
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

	Interface::StartSession();

    ParamList param_SetRestrained;
    param_SetRestrained.push_back(actor->GetReferenceParam());
    param_SetRestrained.push_back(restrained ? Data::Param_True : Data::Param_False);
    ParamContainer SetRestrained = ParamContainer(param_SetRestrained, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("SetRestrained", SetRestrained);

	Interface::EndSession();
}

void Game::SetPos( FactoryObject reference, double X, double Y, double Z )
{
	Object* object = vaultcast<Object>( reference );
	bool result = ( ( bool ) object->SetNetworkPos( Axis_X, X ) | ( bool ) object->SetNetworkPos( Axis_Y, Y ) | ( bool ) object->SetNetworkPos( Axis_Z, Z ) );

	if ( result )
		SetPos( reference );
}

void Game::SetPos( FactoryObject reference )
{
	Object* object = vaultcast<Object>( reference );

	if ( object->GetEnabled() )
	{
		Actor* actor = vaultcast<Actor>( object ); // maybe we should consider items, too (they have physics)

		if ( actor == NULL || ( !actor->IsNearPoint( object->GetNetworkPos( Axis_X ), object->GetNetworkPos( Axis_Y ), object->GetNetworkPos( Axis_Z ), 200.0 ) && actor->GetActorMovingAnimation() == AnimGroup_Idle ) || actor->IsActorJumping() )
		{
			Lockable* key = NULL;

			Interface::StartSession();

			key = object->SetGamePos( Axis_X, object->GetNetworkPos( Axis_X ) );

            ParamList param_SetPos;
            param_SetPos.push_back(object->GetReferenceParam());
            param_SetPos.push_back(BuildParameter(API::RetrieveAxis_Reverse(Axis_X)));
            param_SetPos.push_back(BuildParameter(object->GetNetworkPos(Axis_X)));
            ParamContainer SetPos = ParamContainer(param_SetPos, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("SetPos", SetPos, 0, key ? key->Lock(true) : 0);

			key = object->SetGamePos( Axis_Y, object->GetNetworkPos( Axis_Y ) );

            param_SetPos.clear();
            param_SetPos.push_back(object->GetReferenceParam());
            param_SetPos.push_back(BuildParameter(API::RetrieveAxis_Reverse(Axis_Y)));
            param_SetPos.push_back(BuildParameter(object->GetNetworkPos(Axis_Y)));
            SetPos = ParamContainer(param_SetPos, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("SetPos", SetPos, 0, key ? key->Lock(true) : 0);

			key = object->SetGamePos( Axis_Z, object->GetNetworkPos( Axis_Z ) );

            param_SetPos.clear();
            param_SetPos.push_back(object->GetReferenceParam());
            param_SetPos.push_back(BuildParameter(API::RetrieveAxis_Reverse(Axis_Z)));
            param_SetPos.push_back(BuildParameter(object->GetNetworkPos(Axis_Z)));
            SetPos = ParamContainer(param_SetPos, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("SetPos", SetPos, 0, key ? key->Lock(true) : 0);

			Interface::EndSession();
		}
	}
}

void Game::SetAngle( FactoryObject reference, unsigned char axis, double value )
{
	Object* object = vaultcast<Object>( reference );
	bool result = ( bool ) object->SetAngle( axis, value );

	if ( result && object->GetEnabled() )
		SetAngle( reference, axis );
}

void Game::SetAngle( FactoryObject reference, unsigned char axis )
{
	Object* object = vaultcast<Object>( reference );

	Interface::StartSession();

	ParamList param_SetAngle;
	param_SetAngle.push_back( object->GetReferenceParam() );
	param_SetAngle.push_back( BuildParameter( API::RetrieveAxis_Reverse( axis ) ) );

	double value = object->GetAngle( axis );
	Actor* actor = vaultcast<Actor>( object );

	if ( axis == Axis_Z && actor )
	{
		if ( actor->GetActorMovingXY() == 0x01 )
			AdjustZAngle( value, -45.0 );

		else if ( actor->GetActorMovingXY() == 0x02 )
			AdjustZAngle( value, 45.0 );
	}

	param_SetAngle.push_back( BuildParameter( value ) );
	ParamContainer SetAngle = ParamContainer( param_SetAngle, &Data::AlwaysTrue );
	Interface::ExecuteCommandOnce( "SetAngle", SetAngle );

	Interface::EndSession();
}

void Game::SetNetworkCell( vector<FactoryObject> reference, unsigned int cell )
{
	Object* object = vaultcast<Object>( reference[0] );
	Player* self = vaultcast<Player>( reference[1] );

	if ( !self )
		throw VaultException( "Object with reference %08X is not a Player", ( *reference[1] )->GetReference() );

	object->SetNetworkCell( cell );

	if ( object != self )
	{
		if ( object->GetNetworkCell() != self->GetGameCell() )
			Enable( reference[0], false );

		else
			Enable( reference[0], true );
	}
}

void Game::SetActorValue( FactoryObject reference, bool base, unsigned char index, double value )
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

	Lockable* result;

	if ( base )
		result = actor->SetActorBaseValue( index, value );

	else
		result = actor->SetActorValue( index, value );

	if ( result )
	{
		signed int key = result->Lock( true );

        if (base)
        {
            ParamList param_SetActorValue;
            param_SetActorValue.push_back(actor->GetReferenceParam());
            param_SetActorValue.push_back(BuildParameter(API::RetrieveValue_Reverse(index)));
            param_SetActorValue.push_back(BuildParameter(value));
            ParamContainer SetActorValue = ParamContainer(param_SetActorValue, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("SetActorValue", SetActorValue, 0, key);
        }
        else
        {
            ParamList param_ForceActorValue;
            param_ForceActorValue.push_back(actor->GetReferenceParam());
            param_ForceActorValue.push_back(BuildParameter(API::RetrieveValue_Reverse(index)));
            param_ForceActorValue.push_back(BuildParameter(value));
            ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("ForceActorValue", ForceActorValue, 0, key);
        }

		if ( base )
		{
			ParamList param_SetActorValue;
			param_SetActorValue.push_back( actor->GetReferenceParam() );
			param_SetActorValue.push_back( BuildParameter( API::RetrieveValue_Reverse( index ) ) );
			param_SetActorValue.push_back( BuildParameter( value ) );
			ParamContainer SetActorValue = ParamContainer( param_SetActorValue, &Data::AlwaysTrue );
			Interface::ExecuteCommandOnce( "SetActorValue", SetActorValue, 0, key );
		}

		else
		{
			ParamList param_ForceActorValue;
			param_ForceActorValue.push_back( actor->GetReferenceParam() );
			param_ForceActorValue.push_back( BuildParameter( API::RetrieveValue_Reverse( index ) ) );
			param_ForceActorValue.push_back( BuildParameter( value ) );
			ParamContainer ForceActorValue = ParamContainer( param_ForceActorValue, &Data::AlwaysTrue );
			Interface::ExecuteCommandOnce( "ForceActorValue", ForceActorValue, 0, key );
		}

		Interface::EndSession();
	}
}

void Game::SetActorState( FactoryObject reference, unsigned char index, unsigned char moving, bool alerted, bool sneaking )
{
	Actor* actor = vaultcast<Actor>( reference );

	if ( !actor )
		throw VaultException( "Object with reference %08X is not an Actor", ( *reference )->GetReference() );

	Lockable* result;

	result = actor->SetActorMovingXY( moving );

	if ( result && actor->GetEnabled() )
		SetAngle( reference, Axis_Z );

	result = actor->SetActorAlerted( alerted );

	if ( result && actor->GetEnabled() )
	{
		SetRestrained( reference, false );

		signed int key = result->Lock( true );

		Interface::StartSession();

        ParamList param_SetAlert;
        param_SetAlert.push_back(actor->GetReferenceParam());
        param_SetAlert.push_back(alerted ? Data::Param_True : Data::Param_False);
        ParamContainer SetAlert = ParamContainer(param_SetAlert, &Data::AlwaysTrue);
        Interface::ExecuteCommandOnce("SetAlert", SetAlert, 0, key);

		Interface::EndSession();

        SetRestrained(reference, true);
    }

	result = actor->SetActorSneaking( sneaking );

	if ( result && actor->GetEnabled() )
	{
		SetRestrained( reference, false );

		signed int key = result->Lock( true );

		Interface::StartSession();

        ParamList param_SetForceSneak;
        param_SetForceSneak.push_back(actor->GetReferenceParam());
        param_SetForceSneak.push_back(sneaking ? Data::Param_True : Data::Param_False);
        ParamContainer SetForceSneak = ParamContainer(param_SetForceSneak, &Data::AlwaysTrue);
        Interface::ExecuteCommandOnce("SetForceSneak", SetForceSneak, 0, key);

		Interface::EndSession();

		SetRestrained( reference, true );
	}

	result = actor->SetActorMovingAnimation( index );

	if ( result && actor->GetEnabled() )
	{
		signed int key = result->Lock( true );

		Interface::StartSession();

        ParamList param_PlayGroup;
        param_PlayGroup.push_back(actor->GetReferenceParam());
        param_PlayGroup.push_back(BuildParameter(API::RetrieveAnim_Reverse(index)));
        param_PlayGroup.push_back(Data::Param_True);
        ParamContainer PlayGroup = ParamContainer(param_PlayGroup, &Data::AlwaysTrue);
        Interface::ExecuteCommandOnce("PlayGroup", PlayGroup, 0, key);

		Interface::EndSession();

		if ( index == AnimGroup_Idle )
			SetPos( reference );
	}
}

void Game::MoveTo( vector<FactoryObject> reference, bool cell )
{
	Object* object = vaultcast<Object>( reference[0] );
	Object* object2 = vaultcast<Object>( reference[1] );

	Lockable* result = object->SetGameCell( object2->GetGameCell() );

	if ( result )
	{
		signed int key = result->Lock( true );

		Interface::StartSession();

		ParamList param_MoveTo;
		param_MoveTo.push_back( object->GetReferenceParam() );
		param_MoveTo.push_back( object2->GetReferenceParam() );

		if ( cell )
		{
			param_MoveTo.push_back( BuildParameter( object->GetNetworkPos( Axis_X ) - object2->GetNetworkPos( Axis_X ) ) );
			param_MoveTo.push_back( BuildParameter( object->GetNetworkPos( Axis_Y ) - object2->GetNetworkPos( Axis_Y ) ) );
			param_MoveTo.push_back( BuildParameter( object->GetNetworkPos( Axis_Z ) - object2->GetNetworkPos( Axis_Z ) ) );
		}

		ParamContainer MoveTo = ParamContainer( param_MoveTo, &Data::AlwaysTrue );
		Interface::ExecuteCommandOnce( "MoveTo", MoveTo, 0, key );

		Interface::EndSession();
	}
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

void Game::PlaceAtMe( Lockable* data, unsigned int refID )
{
	if ( data == NULL )
		throw VaultException( "Could not relocate reference storage" );

	Value<unsigned int>* store = dynamic_cast<Value<unsigned int>*>( data );

	if ( store == NULL )
		throw VaultException( "Reference storage is corrupted" );

	store->set( refID );
	store->set_promise();
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
			Enable( reference[0], true );
			MoveTo( reference, true );
			SetAngle( reference[0], Axis_Z );
		}
	}

	bool result = ( bool ) object->SetGameCell( cell );

	if ( object != self )
	{
		if ( object->GetNetworkCell() != self->GetGameCell() )
			Enable( reference[0], false );

		else
			Enable( reference[0], true );
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

	for ( int i = 0; i < count; ++i )
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

		container->ApplyDiff( diff );
	}

	GameFactory::DestroyInstance( _temp );
}

void Game::Failure_PlaceAtMe( unsigned int refID, unsigned int baseID, unsigned int count, signed int key )
{
	Interface::StartSession();

    ParamList param_PlaceAtMe;
    param_PlaceAtMe.push_back(BuildParameter(Utils::LongToHex(refID)));
    param_PlaceAtMe.push_back(BuildParameter(Utils::LongToHex(baseID)));
    param_PlaceAtMe.push_back(BuildParameter(count));
    ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
    Interface::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, key);

	Interface::EndSession();
}
