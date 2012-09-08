#include "Actor.h"
#include "PacketFactory.h"

using namespace Values;

RawParameter Actor::param_ActorValues = RawParameter(vector<string>());

#ifdef VAULTMP_DEBUG
Debug* Actor::debug;
#endif

Actor::Actor(unsigned int refID, unsigned int baseID) : Container(refID, baseID)
{
	initialize();
}

Actor::Actor(const pDefault* packet) : Container(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	map<unsigned char, double> values, baseValues;
	map<unsigned char, double>::iterator it, it2;
	unsigned int race, idle;
	unsigned char moving, movingxy, weapon;
	bool female, alerted, sneaking, dead;

	PacketFactory::Access<pTypes::ID_ACTOR_NEW>(packet, values, baseValues, race, idle, moving, movingxy, weapon, female, alerted, sneaking, dead);

	for (it = values.begin(), it2 = baseValues.begin(); it != values.end() && it2 != baseValues.end(); ++it, ++it2)
	{
		this->SetActorValue(it->first, it->second);
		this->SetActorBaseValue(it2->first, it2->second);
	}

	this->SetActorRace(race);
	this->SetActorIdleAnimation(idle);
	this->SetActorMovingAnimation(moving);
	this->SetActorMovingXY(movingxy);
	this->SetActorWeaponAnimation(weapon);
	this->SetActorFemale(female);
	this->SetActorAlerted(alerted);
	this->SetActorSneaking(sneaking);
	this->SetActorDead(dead);
}

Actor::Actor(pPacket&& packet) : Actor(packet.get())
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
		// emplace
		actor_Values.insert(make_pair(_data, Value<double>()));
		actor_BaseValues.insert(make_pair(_data, Value<double>()));
	}

#ifdef VAULTSERVER
	unsigned int baseID = this->GetBase();

	if (baseID != PLAYER_BASE)
	{
		const Record& record = Record::Lookup(baseID, vector<string>{"NPC_", "CREA"});

		if (this->GetName().empty())
			this->SetName(record.GetDescription());
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

unsigned int Actor::GetActorRace() const
{
	return actor_Race.get();
}

unsigned int Actor::GetActorIdleAnimation() const
{
	return anim_Idle.get();
}

unsigned char Actor::GetActorMovingAnimation() const
{
	return anim_Moving.get();
}

unsigned char Actor::GetActorMovingXY() const
{
	return state_MovingXY.get();
}

unsigned char Actor::GetActorWeaponAnimation() const
{
	return anim_Weapon.get();
}

bool Actor::GetActorFemale() const
{
	return state_Female.get();
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

Lockable* Actor::SetActorRace(unsigned int race)
{
	return SetObjectValue(this->actor_Race, race);
}

Lockable* Actor::SetActorIdleAnimation(unsigned int idle)
{
	return SetObjectValue(this->anim_Idle, idle);
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

Lockable* Actor::SetActorWeaponAnimation(unsigned char index)
{
	string anim = API::RetrieveAnim_Reverse(index);

	if (anim.empty())
		throw VaultException("Value %02X not defined in database", index);

	return SetObjectValue(this->anim_Weapon, index);
}

Lockable* Actor::SetActorFemale(bool state)
{
	return SetObjectValue(this->state_Female, state);
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
	const Record& record = Record::Lookup(baseID, vector<string>{"NPC_", "CREA"});

	if (this->GetName().empty())
		this->SetName(record.GetDescription());

	return Reference::SetBase(baseID);
}
#endif

bool Actor::IsActorJumping() const
{
	unsigned char anim = this->GetActorMovingAnimation();
	unsigned char game = API::GetGameCode();

	return ((game & FALLOUT3 && ((anim >= Fallout3::AnimGroup_JumpStart && anim <= Fallout3::AnimGroup_JumpLand)
								|| (anim >= Fallout3::AnimGroup_JumpLoopForward && anim <= Fallout3::AnimGroup_JumpLoopRight)
								|| (anim >= Fallout3::AnimGroup_JumpLandForward && anim <= Fallout3::AnimGroup_JumpLandRight)))
			|| (game & NEWVEGAS && ((anim >= FalloutNV::AnimGroup_JumpStart && anim <= FalloutNV::AnimGroup_JumpLand)
								   || (anim >= FalloutNV::AnimGroup_JumpLoopForward && anim <= FalloutNV::AnimGroup_JumpLoopRight)
								   || (anim >= FalloutNV::AnimGroup_JumpLandForward && anim <= FalloutNV::AnimGroup_JumpLandRight))));
}

bool Actor::IsActorFiring() const
{
	unsigned char anim = this->GetActorWeaponAnimation();
#ifdef VAULTSERVER
	unsigned int weapon = this->GetEquippedWeapon();
	return (anim >= AnimGroup_AttackLeft && anim <= AnimGroup_AttackSpin2ISDown && weapon && Weapon::Lookup(weapon).GetAmmo()); // || is throwable weapon
#else
	return (anim >= AnimGroup_AttackLeft && anim <= AnimGroup_AttackSpin2ISDown);
#endif
}

bool Actor::IsActorPunching() const
{
	unsigned char anim = this->GetActorWeaponAnimation();
#ifdef VAULTSERVER
	return (this->IsActorPowerPunching() || (anim >= AnimGroup_AttackLeft && anim <= AnimGroup_AttackSpin2ISDown && !this->GetEquippedWeapon()));
#else
	return (this->IsActorPowerPunching() || (anim >= AnimGroup_AttackLeft && anim <= AnimGroup_AttackSpin2ISDown));
#endif
}

bool Actor::IsActorPowerPunching() const
{
	unsigned char anim = this->GetActorWeaponAnimation();
	return (anim >= AnimGroup_AttackPower && anim <= AnimGroup_AttackRightPower);
}

bool Actor::IsActorAttacking() const
{
	return (this->IsActorFiring() || this->IsActorPunching());
}

#ifdef VAULTSERVER
unsigned int Actor::GetEquippedWeapon() const
{
	list<NetworkID> weapons = this->GetItemTypes("WEAP");

	// this won't reliably work if the actor has equipped more than one weapon
	for (NetworkID& weapon : weapons)
	{
		FactoryObject _reference = GameFactory::GetObject(weapon);
		Item* item = vaultcast<Item>(_reference);

		if (item->GetItemEquipped())
			return item->GetBase();
	}

	return 0x00000000;
}
#endif

pPacket Actor::toPacket() const
{
	vector<unsigned char> data = API::RetrieveAllValues();
	map<unsigned char, double> values, baseValues;

	for (unsigned char _data : data)
	{
		// emplace
		values.insert(make_pair(_data, this->GetActorValue(_data)));
		baseValues.insert(make_pair(_data, this->GetActorBaseValue(_data)));
	}

	pPacket pContainerNew = Container::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_ACTOR_NEW>(pContainerNew, values, baseValues, this->GetActorRace(), this->GetActorIdleAnimation(), this->GetActorMovingAnimation(), this->GetActorMovingXY(), this->GetActorWeaponAnimation(), this->GetActorFemale(), this->GetActorAlerted(), this->GetActorSneaking(), this->GetActorDead());

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
			if ((refID = (*it)->GetReference()) && !filter(*it))
				result.emplace_back(Utils::toString(refID));
	}

	_next(result);

	return result;
}

bool ActorFunctor::filter(FactoryObject& reference)
{
	if (ObjectFunctor::filter(reference))
		return true;

	Actor* actor = vaultcast<Actor>(reference);
	unsigned int flags = this->flags();

	if (flags & FLAG_ALIVE && actor->GetActorDead())
		return true;

	else if (flags & FLAG_DEAD && !actor->GetActorDead())
		return true;

	if (flags & FLAG_ISALERT && !actor->GetActorAlerted())
		return true;

	else if (flags & FLAG_NOTALERT && actor->GetActorAlerted())
		return true;

	// FLAG_SELFALERT

	return false;
}
