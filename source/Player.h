#ifndef PLAYER_H
#define PLAYER_H

#define TYPECLASS
#include "GameFactory.h"

#include <string>
#include <map>
#include <vector>

#include "Actor.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#define DEFAULT_PLAYER_RESPAWN  8000

const unsigned int FLAG_MOVCONTROLS      = FLAG_NOTALERTED << 1;

using namespace std;

/**
 * \brief Derives from Actor class and represents a player in-game
 *
 * Data specific to Players are a set of controls
 */

class Player : public Actor
{
		friend class GameFactory;
		friend class PlayerFunctor;

	private:

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		static unsigned int default_respawn;

		unordered_map<unsigned char, pair<Value<unsigned char>, Value<bool> > > player_Controls;
		Value<unsigned int> player_Respawn;

		void initialize();

		Player(const Player&);
		Player& operator=(const Player&);

	protected:
		Player(unsigned int refID, unsigned int baseID);
		Player(const pDefault* packet);
		virtual ~Player();

	public:

		/**
		 * \brief Sets the default respawn time
		 */
		static void SetRespawn(unsigned int respawn);

		/**
		 * \brief Creates a Parameter containing a VaultFunctor initialized with the given flags
		 *
		 * Used to pass Player references matching the provided flags to the Interface
		 * Can also be used to pass data of a given Player to the Interface
		 */
		static FuncParameter CreateFunctor(unsigned int flags, NetworkID id = 0);

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

		/**
		 * \brief Retrieves the key associated to the Player's control
		 */
		unsigned char GetPlayerControl(unsigned char control) const;
		/**
		 * \brief Given a control code, returns its enabled state
		 */
		bool GetPlayerControlEnabled(unsigned char control) const;
		/**
		 * \brief Returns the Player's respawn time
		 */
		unsigned int GetPlayerRespawn() const;

		/**
		 * \brief Associates a key to the Player's control code
		 */
		Lockable* SetPlayerControl(unsigned char control, unsigned char key);
		/**
		 * \brief Sets the enabled state of the given control code
		 */
		Lockable* SetPlayerControlEnabled(unsigned char control, bool state);
		/**
		 * \brief Sets the respawn time
		 */
		Lockable* SetPlayerRespawn(unsigned int respawn);

		/**
		 * \brief For network transfer
		 */
		virtual pDefault* toPacket();
};

class PlayerFunctor : public ActorFunctor
{
	public:
		PlayerFunctor(unsigned int flags, NetworkID id) : ActorFunctor(flags, id) {}
		virtual ~PlayerFunctor() {}

		virtual vector<string> operator()();
		virtual bool filter(Reference* reference);
};

#endif
