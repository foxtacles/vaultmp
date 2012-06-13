#include "Actor.h"

using namespace Values;

RawParameter Actor::param_ActorValues = RawParameter(vector<string>());

#ifdef VAULTMP_DEBUG
Debug* Actor::debug;
#endif

#ifdef VAULTSERVER
Database* Actor::dbActors = NULL;
Database* Actor::dbCreatures = NULL;
#endif

Actor::Actor(unsigned int refID, unsigned int baseID) : Container(refID, baseID)
{
	initialize();
}

Actor::Actor(const pDefault* packet) : Container(PacketFactory::ExtractPartial(packet))
{
	initialize();

	map<unsigned char, double> values, baseValues;
	map<unsigned char, double>::iterator it, it2;
	unsigned char moving, moving_xy;
	bool alerted, sneaking, dead;

	PacketFactory::Access(packet, &values, &baseValues, &moving, &moving_xy, &alerted, &sneaking, &dead);

	for (it = values.begin(), it2 = baseValues.begin(); it != values.end() && it2 != baseValues.end(); ++it, ++it2)
	{
		this->SetActorValue(it->first, it->second);
		this->SetActorBaseValue(it2->first, it2->second);
	}

	this->SetActorMovingAnimation(moving);
	this->SetActorMovingXY(moving_xy);
	this->SetActorAlerted(alerted);
	this->SetActorSneaking(sneaking);
	this->SetActorDead(dead);
}

Actor::Actor(pPacket&& packet) : Actor(static_cast<const pDefault*>(packet.get()))
{

}

Actor::~Actor()
{

}

void Actor::initialize()
{
	vector<unsigned char> data = API::RetrieveAllValues();

	for (unsigned char _data : data)
	{
		actor_Values.insert(pair<unsigned char, Value<double>>(_data, Value<double>()));
		actor_BaseValues.insert(pair<unsigned char, Value<double>>(_data, Value<double>()));
	}

#ifdef VAULTSERVER

	unsigned int baseID = this->GetBase();

	try
	{
		const Database::Record& record = dbActors->Lookup(baseID);

		if (this->GetName().empty())
			this->SetName(record.description);
	}
	catch (...)
	{
		const Database::Record& record = dbCreatures->Lookup(baseID);

		if (this->GetName().empty())
			this->SetName(record.description);
	}

#endif

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("New actor object created (ref: %08X)", true, this->GetReference());

#endif
}

#ifdef VAULTMP_DEBUG
void Actor::SetDebugHandler(Debug* debug)
{
	Actor::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Actor class", true);
}
#endif

const RawParameter& Actor::Param_ActorValues()
{
	if (param_ActorValues.get().empty())
		param_ActorValues = API::RetrieveAllValues_Reverse();

	return param_ActorValues;
}

FuncParameter Actor::CreateFunctor(unsigned int flags, NetworkID id)
{
	return FuncParameter(unique_ptr<VaultFunctor>(new ActorFunctor(flags, id)));
}

double Actor::GetActorValue(unsigned char index) const
{
	return actor_Values.at(index).get();
}

double Actor::GetActorBaseValue(unsigned char index) const
{
	return actor_BaseValues.at(index).get();
}

unsigned char Actor::GetActorMovingAnimation() const
{
	return anim_Moving.get();
}

unsigned char Actor::GetActorMovingXY() const
{
	return state_MovingXY.get();
}

bool Actor::GetActorAlerted() const
{
	return state_Alerted.get();
}

bool Actor::GetActorSneaking() const
{
	return state_Sneaking.get();
}

bool Actor::GetActorDead() const
{
	return state_Dead.get();
}

Lockable* Actor::SetActorValue(unsigned char index, double value)
{
	return SetObjectValue(this->actor_Values.at(index), value);
}

Lockable* Actor::SetActorBaseValue(unsigned char index, double value)
{
	return SetObjectValue(this->actor_BaseValues.at(index), value);
}

Lockable* Actor::SetActorMovingAnimation(unsigned char index)
{
	string anim = API::RetrieveAnim_Reverse(index);

	if (anim.empty())
		throw VaultException("Value %02X not defined in database", index);

	return SetObjectValue(this->anim_Moving, index);
}

Lockable* Actor::SetActorMovingXY(unsigned char moving)
{
	return SetObjectValue(this->state_MovingXY, moving);
}

Lockable* Actor::SetActorAlerted(bool state)
{
	return SetObjectValue(this->state_Alerted, state);
}

Lockable* Actor::SetActorSneaking(bool state)
{
	return SetObjectValue(this->state_Sneaking, state);
}

Lockable* Actor::SetActorDead(bool state)
{
	return SetObjectValue(this->state_Dead, state);
}

#ifdef VAULTSERVER
Lockable* Actor::SetBase(unsigned int baseID)
{
	try
	{
		const Database::Record& record = dbActors->Lookup(baseID);

		if (this->GetName().empty())
			this->SetName(record.name);
	}
	catch (...)
	{
		const Database::Record& record = dbCreatures->Lookup(baseID);

		if (this->GetName().empty())
			this->SetName(record.name);
	}

	return Reference::SetBase(baseID);
}
#endif

bool Actor::IsActorJumping() const
{
	unsigned char anim = this->GetActorMovingAnimation();
	unsigned char game = API::GetGameCode();

	return ((game& FALLOUT3 && ((anim >= Fallout3::AnimGroup_JumpStart && anim <= Fallout3::AnimGroup_JumpLand)
								|| (anim >= Fallout3::AnimGroup_JumpLoopForward && anim <= Fallout3::AnimGroup_JumpLoopRight)
								|| (anim >= Fallout3::AnimGroup_JumpLandForward && anim <= Fallout3::AnimGroup_JumpLandRight)))
			|| (game& NEWVEGAS && ((anim >= FalloutNV::AnimGroup_JumpStart && anim <= FalloutNV::AnimGroup_JumpLand)
								   || (anim >= FalloutNV::AnimGroup_JumpLoopForward && anim <= FalloutNV::AnimGroup_JumpLoopRight)
								   || (anim >= FalloutNV::AnimGroup_JumpLandForward && anim <= FalloutNV::AnimGroup_JumpLandRight))));
}

pPacket Actor::toPacket()
{
	vector<unsigned char> data = API::RetrieveAllValues();
	map<unsigned char, double> values, baseValues;

	for (unsigned char _data : data)
	{
		values.insert(pair<unsigned char, double>(_data, this->GetActorValue(_data)));
		baseValues.insert(pair<unsigned char, double>(_data, this->GetActorBaseValue(_data)));
	}

	pPacket pContainerNew = Container::toPacket();
	pPacket packet = PacketFactory::CreatePacket(ID_ACTOR_NEW, pContainerNew.get(), &values, &baseValues, this->GetActorMovingAnimation(), this->GetActorMovingXY(), this->GetActorAlerted(), this->GetActorSneaking(), this->GetActorDead());

	return packet;
}

vector<string> ActorFunctor::operator()()
{
	vector<string> result;

	NetworkID id = get();

	if (id)
	{
		//FactoryObject reference = GameFactory::GetObject(id);
		//Reference* actor = vaultcast<Actor>(reference);
	}
	else
	{
		vector<FactoryObject>::iterator it;
		vector<FactoryObject> references = GameFactory::GetObjectTypes(ID_ACTOR);
		unsigned int refID;

		for (it = references.begin(); it != references.end(); GameFactory::LeaveReference(*it), ++it)
			if ((refID = (**it)->GetReference()) && !filter(**it))
				result.push_back(Utils::toString(refID));
	}

	_next(result);

	return result;
}

bool ActorFunctor::filter(Reference* reference)
{
	if (ObjectFunctor::filter(reference))
		return true;

	Actor* actor = vaultcast<Actor>(reference);
	unsigned int flags = this->flags();

	if (flags & FLAG_ALIVE && actor->GetActorDead())
		return true;

	else if (flags & FLAG_DEAD && !actor->GetActorDead())
		return true;

	if (flags & FLAG_ISALERTED && !actor->GetActorAlerted())
		return true;

	else if (flags & FLAG_NOTALERTED && actor->GetActorAlerted())
		return true;

	return false;
}
