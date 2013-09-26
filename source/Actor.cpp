#include "Actor.hpp"

#ifndef VAULTSERVER
#include "Game.hpp"
#else
#include "vaultserver/Record.hpp"
#include "vaultserver/Weapon.hpp"
#include "Item.hpp"
#endif

#include <algorithm>

using namespace std;
using namespace RakNet;
using namespace Values;

const map<unsigned char, pair<const float, const float>> Actor::default_values = {
	{ActorVal_Energy, {50, 50}},
	{ActorVal_Responsibility, {50, 50}},
	{ActorVal_Strength, {5, 5}},
	{ActorVal_Perception, {5, 5}},
	{ActorVal_Endurance, {5, 5}},
	{ActorVal_Charisma, {5, 5}},
	{ActorVal_Intelligence, {5, 5}},
	{ActorVal_Agility, {5, 5}},
	{ActorVal_Luck, {5, 5}},
	{ActorVal_ActionPoints, {75, 75}},
	{ActorVal_CarryWeight, {200, 200}},
	{ActorVal_Health, {200, 200}},
	{ActorVal_PoisonResistance, {20, 20}},
	{ActorVal_RadResistance, {8, 8}},
	{ActorVal_SpeedMultiplier, {100, 100}},
	{ActorVal_Fatigue, {200, 200}},
	{ActorVal_Head, {100, 100}},
	{ActorVal_Torso, {100, 100}},
	{ActorVal_LeftArm, {100, 100}},
	{ActorVal_RightArm, {100, 100}},
	{ActorVal_LeftLeg, {100, 100}},
	{ActorVal_RightLeg, {100, 100}},
	{ActorVal_Brain, {100, 100}},
	{ActorVal_Barter, {15, 15}},
	{ActorVal_BigGuns, {15, 15}},
	{ActorVal_EnergyWeapons, {15, 15}},
	{ActorVal_Explosives, {15, 15}},
	{ActorVal_Lockpick, {15, 15}},
	{ActorVal_Medicine, {15, 15}},
	{ActorVal_MeleeWeapons, {15, 15}},
	{ActorVal_Repair, {15, 15}},
	{ActorVal_Science, {15, 15}},
	{ActorVal_SmallGuns, {15, 15}},
	{ActorVal_Sneak, {15, 15}},
	{ActorVal_Speech, {15, 15}},
	{ActorVal_Unarmed, {15, 15}},
};

#ifdef VAULTMP_DEBUG
DebugInput<Actor> Actor::debug;
#endif

Actor::Actor(unsigned int refID, unsigned int baseID) : Container(refID, baseID)
{
	initialize();

	for (const auto& value : default_values)
	{
		this->SetActorBaseValue(value.first, value.second.first);
		this->SetActorValue(value.first, value.second.second);
	}
}

Actor::Actor(const pPacket& packet) : Container(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	map<unsigned char, float> values, baseValues;
	map<unsigned char, float>::iterator it, it2;
	unsigned int race, idle;
	signed int age;
	unsigned char moving, movingxy, weapon;
	bool female, alerted, sneaking, dead;

	PacketFactory::Access<pTypes::ID_ACTOR_NEW>(packet, values, baseValues, race, age, idle, moving, movingxy, weapon, female, alerted, sneaking, dead);

	for (it = values.begin(), it2 = baseValues.begin(); it != values.end() && it2 != baseValues.end(); ++it, ++it2)
	{
		this->SetActorValue(it->first, it->second);
		this->SetActorBaseValue(it2->first, it2->second);
	}

	this->SetActorRace(race);
	this->SetActorAge(age);
	this->SetActorIdleAnimation(idle);
	this->SetActorMovingAnimation(moving);
	this->SetActorMovingXY(movingxy);
	this->SetActorWeaponAnimation(weapon);
	this->SetActorFemale(female);
	this->SetActorAlerted(alerted);
	this->SetActorSneaking(sneaking);
	this->SetActorDead(dead);
}

Actor::~Actor() noexcept {}

void Actor::initialize()
{
	for (const auto& value : API::values)
	{
		// emplace
		actor_Values.insert(make_pair(value.second, Value<float>()));
		actor_BaseValues.insert(make_pair(value.second, Value<float>()));
	}

#ifdef VAULTMP_DEBUG
	debug.print("New actor object created (ref: ", hex, this->GetReference(), ")");
#endif
}

#ifndef VAULTSERVER
FuncParameter Actor::CreateFunctor(unsigned int flags, NetworkID id)
{
	return FuncParameter(unique_ptr<VaultFunctor>(new ActorFunctor(flags, id)));
}
#endif

float Actor::GetActorValue(unsigned char index) const
{
	return actor_Values.at(index).get();
}

float Actor::GetActorBaseValue(unsigned char index) const
{
	return actor_BaseValues.at(index).get();
}

unsigned int Actor::GetActorRace() const
{
	return actor_Race.get();
}

signed int Actor::GetActorAge() const
{
	return actor_Age.get();
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

Lockable* Actor::SetActorValue(unsigned char index, float value)
{
	return SetObjectValue(this->actor_Values.at(index), value);
}

Lockable* Actor::SetActorBaseValue(unsigned char index, float value)
{
	return SetObjectValue(this->actor_BaseValues.at(index), value);
}

Lockable* Actor::SetActorRace(unsigned int race)
{
	return SetObjectValue(this->actor_Race, race);
}

Lockable* Actor::SetActorAge(signed int age)
{
	return SetObjectValue(this->actor_Age, age);
}

Lockable* Actor::SetActorIdleAnimation(unsigned int idle)
{
	return SetObjectValue(this->anim_Idle, idle);
}

Lockable* Actor::SetActorMovingAnimation(unsigned char index)
{
	if (find_if(API::anims.begin(), API::anims.end(), [index](const decltype(API::values)::value_type& value) { return value.second == index; }) == API::anims.end())
		throw VaultException("Value %02X not defined in database", index).stacktrace();

	return SetObjectValue(this->anim_Moving, index);
}

Lockable* Actor::SetActorMovingXY(unsigned char moving)
{
	return SetObjectValue(this->state_MovingXY, moving);
}

Lockable* Actor::SetActorWeaponAnimation(unsigned char index)
{
	if (find_if(API::anims.begin(), API::anims.end(), [index](const decltype(API::values)::value_type& value) { return value.second == index; }) == API::anims.end())
		throw VaultException("Value %02X not defined in database", index).stacktrace();

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
	const DB::Record* record = *DB::Record::Lookup(baseID, vector<string>{"NPC_", "CREA"});

	if (this->GetName().empty())
		this->SetName(record->GetDescription());

	return Reference::SetBase(baseID);
}
#endif

bool Actor::IsActorJumping() const
{
	unsigned char anim = this->GetActorMovingAnimation();

	return (anim >= AnimGroup_JumpStart && anim <= AnimGroup_JumpLand)
		|| (anim >= AnimGroup_JumpLoopForward && anim <= AnimGroup_JumpLoopRight)
		|| (anim >= AnimGroup_JumpLandForward && anim <= AnimGroup_JumpLandRight);
}

bool Actor::IsActorFiring() const
{
	unsigned char anim = this->GetActorWeaponAnimation();
#ifdef VAULTSERVER
	unsigned int weapon = this->GetEquippedWeapon();
	return (anim >= AnimGroup_AttackLeft && anim <= AnimGroup_AttackSpin2ISDown && weapon && DB::Weapon::Lookup(weapon)->GetAmmo()); // || is throwable weapon
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
	auto weapons = this->GetItemTypes("WEAP");
	unsigned int baseID;

	// this won't reliably work if the actor has equipped more than one weapon
	for (auto weapon : weapons)
		if ((baseID = GameFactory::Operate<Item>(weapon, [](Item* item) {
			return item->GetItemEquipped() ? item->GetBase() : 0x00000000;
		}))) return baseID;

	return 0x00000000;
}
#endif

pPacket Actor::toPacket() const
{
	map<unsigned char, float> values, baseValues;

	for (const auto& value : actor_Values)
	{
		// emplace
		values.insert(make_pair(value.first, this->GetActorValue(value.first)));
		baseValues.insert(make_pair(value.first, this->GetActorBaseValue(value.first)));
	}

	pPacket pContainerNew = Container::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_ACTOR_NEW>(pContainerNew, values, baseValues, this->GetActorRace(), this->GetActorAge(), this->GetActorIdleAnimation(), this->GetActorMovingAnimation(), this->GetActorMovingXY(), this->GetActorWeaponAnimation(), this->GetActorFemale(), this->GetActorAlerted(), this->GetActorSneaking(), this->GetActorDead());

	return packet;
}

#ifndef VAULTSERVER
vector<string> ActorFunctor::operator()()
{
	vector<string> result;

	NetworkID id = get();

	if (id)
	{

	}
	else
	{
		auto references = Game::GetContext(ID_ACTOR);

		for (unsigned int refID : references)
			GameFactory::Operate<Actor, RET_F_VALID>(refID, [this, refID, &result](FactoryActor& actor) {
				if (!filter(actor))
					result.emplace_back(Utils::toString(refID));
			});
	}

	_next(result);

	return result;
}

bool ActorFunctor::filter(FactoryWrapper<Reference>& reference)
{
	if (ContainerFunctor::filter(reference))
		return true;

	return GameFactory::Operate<Actor>(reference->GetNetworkID(), [this](Actor* actor) {
		unsigned int flags = this->flags();

		if (flags & FLAG_ALIVE && actor->GetActorDead())
			return true;

		return false;
	});
}
#endif
