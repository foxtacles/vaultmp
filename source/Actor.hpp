#ifndef ACTOR_H
#define ACTOR_H

#include "vaultmp.hpp"
#include "Container.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

const unsigned int FLAG_ALIVE = FLAG_SELF << 1;

/**
 * \brief Derives from Container class and represents an actor in-game
 *
 * Data specific to Actors are actor values, animations and various states
 */

class Actor : public Container
{
		friend class GameFactory;

	private:
#ifdef VAULTMP_DEBUG
		static DebugInput<Actor> debug;
#endif

		std::unordered_map<unsigned char, Value<float>> actor_Values;
		std::unordered_map<unsigned char, Value<float>> actor_BaseValues;
		Value<unsigned int> actor_Race;
		Value<signed int> actor_Age;
		Value<unsigned int> anim_Idle;
		Value<unsigned char> anim_Moving;
		Value<unsigned char> state_MovingXY;
		Value<unsigned char> anim_Weapon;
		Value<bool> state_Female;
		Value<bool> state_Alerted;
		Value<bool> state_Sneaking;
		Value<bool> state_Dead;
		Value<unsigned short> death_Limbs;
		Value<signed char> death_Cause;

		void initialize();

		Actor(const Actor&) = delete;
		Actor& operator=(const Actor&) = delete;

	protected:
		Actor(unsigned int refID, unsigned int baseID);
		Actor(unsigned int baseID) : Actor(0x00000000, baseID) {}
		Actor(const pPacket& packet);
		Actor(pPacket&& packet) : Actor(packet) {};

	public:
		virtual ~Actor() noexcept;

		static const std::map<unsigned char, std::pair<const float, const float>> default_values;

#ifndef VAULTSERVER
		/**
		 * \brief Creates a Parameter containing a VaultFunctor initialized with the given flags
		 *
		 * Used to pass Actor references matching the provided flags to the Interface
		 * Can also be used to pass data of a given Actor to the Interface
		 */
		static FuncParameter CreateFunctor(unsigned int flags, RakNet::NetworkID id = 0);
#endif
		/**
		 * \brief Retrieves the Actor's actor value specified by index (actor value hex code)
		 */
		float GetActorValue(unsigned char index) const;
		/**
		 * \brief Retrieves the Actor's base actor value specified by index (actor value hex code)
		 */
		float GetActorBaseValue(unsigned char index) const;
		/**
		 * \brief Retrieves the Actor's race
		 */
		unsigned int GetActorRace() const;
		/**
		 * \brief Retrieves the Actor's relative age
		 */
		signed int GetActorAge() const;
		/**
		 * \brief Retrieves the Actor's idle animation
		 */
		unsigned int GetActorIdleAnimation() const;
		/**
		 * \brief Retrieves the Actor's moving animation
		 */
		unsigned char GetActorMovingAnimation() const;
		/**
		 * \brief Retrieves the Actor's moving state
		 *
		 * 0x00 - the actor moves Forward / Backward
		 * 0x01 - the actor moves ForwardLeft / BackwardRight
		 * 0x02 - the actor moves ForwardRight / BackwardLeft
		 */
		unsigned char GetActorMovingXY() const;
		/**
		 * \brief Retrieves the Actor's weapon animation
		 */
		unsigned char GetActorWeaponAnimation() const;
		/**
		 * \brief Retrieves the Actor's sex
		 */
		bool GetActorFemale() const;
		/**
		 * \brief Retrieves the Actor's alerted state
		 */
		bool GetActorAlerted() const;
		/**
		 * \brief Retrieves the Actor's sneaking state
		 */
		bool GetActorSneaking() const;
		/**
		 * \brief Retrieves the Actor's dead state
		 */
		bool GetActorDead() const;
		/**
		 * \brief Retrieves the Actor's last limb state after death
		 */
		unsigned short GetActorLimbs() const;
		/**
		 * \brief Retrieves the Actor's last death cause
		 */
		signed char GetActorDeathCause() const;

		/**
		 * \brief Sets the Actor's actor value specified by index (actor value hex code)
		 */
		Lockable* SetActorValue(unsigned char index, float value);
		/**
		 * \brief Sets the Actor's base actor value specified by index (actor value hex code)
		 */
		Lockable* SetActorBaseValue(unsigned char index, float value);
		/**
		 * \brief Sets the Actor's race
		 */
		Lockable* SetActorRace(unsigned int race);
		/**
		 * \brief Sets the Actor's relative age
		 */
		Lockable* SetActorAge(signed int age);
		/**
		 * \brief Sets the Actor's idle animation
		 */
		Lockable* SetActorIdleAnimation(unsigned int idle);
		/**
		 * \brief Sets the Actor's moving animation
		 */
		Lockable* SetActorMovingAnimation(unsigned char index);
		/**
		 * \brief Sets the Actor's moving state
		 *
		 * 0x00 - the actor moves Forward / Backward
		 * 0x01 - the actor moves ForwardLeft / BackwardRight
		 * 0x02 - the actor moves ForwardRight / BackwardLeft
		 */
		Lockable* SetActorMovingXY(unsigned char moving);
		/**
		 * \brief Sets the Actor's weapon animation
		 */
		Lockable* SetActorWeaponAnimation(unsigned char index);
		/**
		 * \brief Sets the Actor's sex
		 */
		Lockable* SetActorFemale(bool state);
		/**
		 * \brief Sets the Actor's alerted state
		 */
		Lockable* SetActorAlerted(bool state);
		/**
		 * \brief Sets the Actor's sneaking state
		 */
		Lockable* SetActorSneaking(bool state);
		/**
		 * \brief Sets the Actor's dead state
		 */
		Lockable* SetActorDead(bool state, unsigned short limbs, signed char cause);

#ifdef VAULTSERVER
		/**
		 * \brief Sets the Actor's base ID
		 */
		virtual Lockable* SetBase(unsigned int baseID);
#endif

		/**
		 * \brief Returns true if the Actor is jumping
		 */
		bool IsActorJumping() const;
		/**
		 * \brief Returns true if the Actor is firing a weapon
		 *
		 *	Only accurate in vaultserver
		 */
		bool IsActorFiring() const;
		/**
		 * \brief Returns true if the Actor is punching
		 *
		 *	Only accurate in vaultserver
		 */
		bool IsActorPunching() const;
		/**
		 * \brief Returns true if the Actor is punching (power)
		 */
		bool IsActorPowerPunching() const;
		/**
		 * \brief Returns true if the Actor is attacking
		 */
		bool IsActorAttacking() const;

#ifdef VAULTSERVER
		/**
		 * \brief Returns the baseID of the Actor's equipped weapon
		 */
		unsigned int GetEquippedWeapon() const;
#endif

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#ifndef VAULTSERVER
class ActorFunctor : public ContainerFunctor
{
	public:
		ActorFunctor(unsigned int flags, RakNet::NetworkID id) : ContainerFunctor(flags, id) {}
		virtual ~ActorFunctor() {}

		virtual std::vector<std::string> operator()();
		virtual bool filter(FactoryWrapper<Reference>& reference);
};
#endif

GF_TYPE_WRAPPER(Actor, Container, ID_ACTOR, ALL_ACTORS)

PF_MAKE(ID_ACTOR_NEW, pGeneratorReferenceExtend, std::map<unsigned char, float>, std::map<unsigned char, float>, unsigned int, signed int, unsigned int, unsigned char, unsigned char, unsigned char, bool, bool, bool, bool, unsigned short, signed char)
template<>
inline const typename pTypesMap<pTypes::ID_ACTOR_NEW>::type* PacketFactory::Cast_<pTypes::ID_ACTOR_NEW>::Cast(const pPacket* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_ACTOR_NEW>::type*>(packet) : nullptr;
}
PF_MAKE(ID_UPDATE_STATE, pGeneratorReference, unsigned int, unsigned char, unsigned char, unsigned char, bool, bool, bool)
PF_MAKE(ID_UPDATE_RACE, pGeneratorReference, unsigned int, signed int, signed int)
PF_MAKE(ID_UPDATE_SEX, pGeneratorReference, bool)
PF_MAKE(ID_UPDATE_DEAD, pGeneratorReference, bool, unsigned short, signed char)
PF_MAKE(ID_UPDATE_FIREWEAPON, pGeneratorReference, unsigned int)
PF_MAKE(ID_UPDATE_IDLE, pGeneratorReference, unsigned int, std::string)

#endif
