#include "Actor.h"

using namespace Values;

Parameter Actor::param_ActorValues = Parameter( vector<string>(), NULL );
Database* Actor::Actors = NULL;
Database* Actor::Creatures = NULL;

#ifdef VAULTMP_DEBUG
Debug* Actor::debug;
#endif

Actor::Actor( unsigned int refID, unsigned int baseID ) : Container( refID, baseID )
{
	vector<unsigned char>::iterator it;
	vector<unsigned char> data = API::RetrieveAllValues();

	for ( it = data.begin(); it != data.end(); ++it )
	{
		actor_Values.insert( pair<unsigned char, Value<double> >( *it, Value<double>() ) );
		actor_BaseValues.insert( pair<unsigned char, Value<double> >( *it, Value<double>() ) );
	}

	if ( Actor::Actors )
	{
		this->data = Actor::Actors->find( baseID );

		if ( this->data == Actor::Actors->end() )
			this->data = Actor::Actors->find( Reference::ResolveIndex( baseID ) );

		if ( this->data == Actor::Actors->end() )
			this->data = Actor::Creatures->find( baseID );

		if ( this->data == Actor::Creatures->end() )
			this->data = Actor::Creatures->find( Reference::ResolveIndex( baseID ) );

		if ( this->data != Actor::Actors->end() && this->data != Actor::Creatures->end() )
			this->SetName( string( this->data->second ) );
	}

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "New actor object created (ref: %08X)", true, this->GetReference() );

#endif
}

Actor::~Actor()
{

}

#ifdef VAULTMP_DEBUG
void Actor::SetDebugHandler( Debug* debug )
{
	Actor::debug = debug;

	if ( debug != NULL )
		debug->Print( "Attached debug handler to Actor class", true );
}
#endif

const Parameter& Actor::Param_ActorValues()
{
	if ( param_ActorValues.first.empty() )
		param_ActorValues.first = API::RetrieveAllValues_Reverse();

	return param_ActorValues;
}

const Parameter Actor::CreateFunctor( unsigned int flags )
{
	return Parameter( vector<string>(), new ActorFunctor( flags ) );
}

double Actor::GetActorValue( unsigned char index ) const
{
	return SAFE_FIND( actor_Values, index )->second.Get();
}

double Actor::GetActorBaseValue( unsigned char index ) const
{
	return SAFE_FIND( actor_BaseValues, index )->second.Get();
}

unsigned char Actor::GetActorMovingAnimation() const
{
	return anim_Moving.Get();
}

unsigned char Actor::GetActorMovingXY() const
{
	return state_MovingXY.Get();
}

bool Actor::GetActorAlerted() const
{
	return state_Alerted.Get();
}

bool Actor::GetActorSneaking() const
{
	return state_Sneaking.Get();
}

bool Actor::GetActorDead() const
{
	return state_Dead.Get();
}

Lockable* Actor::SetActorValue( unsigned char index, double value )
{
	Value<double>& data = SAFE_FIND( this->actor_Values, index )->second;

	if ( Utils::DoubleCompare( data.Get(), value, 0.01 ) )
		return NULL;

	if ( !data.Set( value ) )
		return NULL;

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "Actor value %s was set to %f (ref: %08X)", true, API::RetrieveValue_Reverse( index ).c_str(), ( float ) value, this->GetReference() );

#endif

	return &data;
}

Lockable* Actor::SetActorBaseValue( unsigned char index, double value )
{
	Value<double>& data = SAFE_FIND( this->actor_BaseValues, index )->second;

	if ( Utils::DoubleCompare( data.Get(), value, 0.01 ) )
		return NULL;

	if ( !data.Set( value ) )
		return NULL;

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "Actor base value %s was set to %f (ref: %08X)", true, API::RetrieveValue_Reverse( index ).c_str(), ( float ) value, this->GetReference() );

#endif

	return &data;
}

Lockable* Actor::SetActorMovingAnimation( unsigned char index )
{
	string anim = API::RetrieveAnim_Reverse( index );

	if ( anim.empty() )
		throw VaultException( "Value %02X not defined in database", index );

	if ( anim_Moving.Get() == index )
		return NULL;

	if ( !anim_Moving.Set( index ) )
		return NULL;

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "Actor moving animation was set to %s (ref: %08X)", true,  anim.c_str(), this->GetReference() );

#endif

	return &anim_Moving;
}

Lockable* Actor::SetActorMovingXY( unsigned char moving )
{
	if ( this->state_MovingXY.Get() == moving )
		return NULL;

	if ( !this->state_MovingXY.Set( moving ) )
		return NULL;

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "Actor moving state was set to %02X (ref: %08X)", true, moving, this->GetReference() );

#endif

	return &this->state_MovingXY;
}

Lockable* Actor::SetActorAlerted( bool state )
{
	if ( this->state_Alerted.Get() == state )
		return NULL;

	if ( !this->state_Alerted.Set( state ) )
		return NULL;

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "Actor alerted state was set to %d (ref: %08X)", true, ( int ) state, this->GetReference() );

#endif

	return &this->state_Alerted;
}

Lockable* Actor::SetActorSneaking( bool state )
{
	if ( this->state_Sneaking.Get() == state )
		return NULL;

	if ( !this->state_Sneaking.Set( state ) )
		return NULL;

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "Actor sneaking state was set to %d (ref: %08X)", true, ( int ) state, this->GetReference() );

#endif

	return &this->state_Sneaking;
}

Lockable* Actor::SetActorDead( bool state )
{
	if ( this->state_Dead.Get() == state )
		return NULL;

	if ( !this->state_Dead.Set( state ) )
		return NULL;

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "Actor dead state was set to %d (ref: %08X)", true, ( int ) state, this->GetReference() );

#endif

	return &this->state_Dead;
}

bool Actor::IsActorJumping() const
{
	unsigned char anim = this->GetActorMovingAnimation();
	unsigned char game = API::GetGameCode();

	return ( ( game & OBLIVION && anim >= Oblivion::AnimGroup_JumpStart && anim <= Oblivion::AnimGroup_JumpLand )
			 || ( game & FALLOUT3 && ( ( anim >= Fallout3::AnimGroup_JumpStart && anim <= Fallout3::AnimGroup_JumpLand )
									   || ( anim >= Fallout3::AnimGroup_JumpLoopForward && anim <= Fallout3::AnimGroup_JumpLoopRight )
									   || ( anim >= Fallout3::AnimGroup_JumpLandForward && anim <= Fallout3::AnimGroup_JumpLandRight ) ) )
			 || ( game & NEWVEGAS && ( ( anim >= FalloutNV::AnimGroup_JumpStart && anim <= FalloutNV::AnimGroup_JumpLand )
									   || ( anim >= FalloutNV::AnimGroup_JumpLoopForward && anim <= FalloutNV::AnimGroup_JumpLoopRight )
									   || ( anim >= FalloutNV::AnimGroup_JumpLandForward && anim <= FalloutNV::AnimGroup_JumpLandRight ) ) ) );
}

pDefault* Actor::toPacket()
{
    vector<unsigned char>::iterator it;
    vector<unsigned char> data = API::RetrieveAllValues();
    map<unsigned char, double> values;
    map<unsigned char, double> baseValues;

    for (it = data.begin(); it != data.end(); ++it)
    {
        values.insert(pair<unsigned char, double>(*it, this->GetActorValue(*it)));
        baseValues.insert(pair<unsigned char, double>(*it, this->GetActorBaseValue(*it)));
    }

    pDefault* pContainerNew = Container::toPacket();

    pDefault* packet = PacketFactory::CreatePacket(ID_ACTOR_NEW, pContainerNew, &values, &baseValues, this->GetActorMovingAnimation(), this->GetActorMovingXY(),
                                                   this->GetActorAlerted(), this->GetActorSneaking(), this->GetActorDead());

    PacketFactory::FreePacket(pContainerNew);

    return packet;
}

vector<string> ActorFunctor::operator()()
{
	vector<string> result;
	vector<FactoryObject>::iterator it;
	vector<FactoryObject> actorlist = GameFactory::GetObjectTypes( ALL_ACTORS );

	for ( it = actorlist.begin(); it != actorlist.end(); GameFactory::LeaveReference( *it ), ++it )
	{
		Actor* actor = vaultcast<Actor>( *it );
		unsigned int refID = actor->GetReference();

		if ( refID != 0x00000000 )
		{
			if ( flags & FLAG_NOTSELF && refID == PLAYER_REFERENCE )
				continue;

			if ( flags & FLAG_ENABLED && !actor->GetEnabled() )
				continue;

			else if ( flags & FLAG_DISABLED && actor->GetEnabled() )
				continue;

			if ( flags & FLAG_ALIVE && actor->GetActorDead() )
				continue;

			else if ( flags & FLAG_DEAD && !actor->GetActorDead() )
				continue;

			if ( flags & FLAG_ISALERTED && !actor->GetActorAlerted() )
				continue;

			else if ( flags & FLAG_NOTALERTED && actor->GetActorAlerted() )
				continue;
		}

		result.push_back( Utils::LongToHex( refID ) );
	}

	_next( result );

	return result;
}

ActorFunctor::~ActorFunctor()
{

}
