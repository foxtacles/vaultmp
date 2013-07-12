#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <map>
#include <vector>
#include <array>

#include "Actor.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

const unsigned int FLAG_MOVCONTROLS = FLAG_NOTALERT << 1;

/**
 * \brief Derives from Actor class and represents a player in-game
 *
 * Data specific to Players are a set of controls
 */

class Player : public Actor
{
		friend class GameFactory;
		friend class PlayerFunctor;

	public:
		typedef std::array<unsigned int, 9> CellContext;

	private:
#ifdef VAULTMP_DEBUG
		static DebugInput<Player> debug;
#endif

#ifdef VAULTSERVER
		static std::unordered_set<unsigned int> baseIDs;

		static unsigned int default_respawn;
		static unsigned int default_cell;
		static bool default_console;

		Value<unsigned int> player_Respawn;
		Value<unsigned int> player_Cell;
		Value<CellContext> player_CellContext;
		Value<bool> state_Console;
#endif

		std::unordered_map<unsigned char, std::pair<Value<unsigned char>, Value<bool>>> player_Controls;

		void initialize();

		Player(const Player&) = delete;
		Player& operator=(const Player&) = delete;

	protected:
		Player(unsigned int refID, unsigned int baseID);
		Player(const pDefault* packet);

	public:
		virtual ~Player();

		static const std::map<unsigned char, std::pair<double, double>> f3_default_values;
		static const std::map<unsigned int, std::tuple<unsigned int, double, bool, bool, bool>> default_items;

#ifdef VAULTSERVER
		static const unsigned int DEFAULT_PLAYER_RESPAWN = 8000;

		/**
		 * \brief Gets the default respawn time
		 */
		static unsigned int GetRespawnTime();
		/**
		 * \brief Gets the default spawn cell
		 */
		static unsigned int GetSpawnCell();
		/**
		 * \brief Gets the default console state
		 */
		static bool GetConsoleEnabled();
		/**
		 * \brief Sets the default respawn time
		 */
		static void SetRespawnTime(unsigned int respawn);
		/**
		 * \brief Sets the default spawn cell
		 */
		static void SetSpawnCell(unsigned int cell);
		/**
		 * \brief Sets the default console state
		 */
		static void SetConsoleEnabled(bool enabled);
		/**
		 * \brief Returns the set of all used base IDs by players
		 */
		static const std::unordered_set<unsigned int>& GetBaseIDs();
#endif
#ifndef VAULTSERVER
		/**
		 * \brief Creates a Parameter containing a VaultFunctor initialized with the given flags
		 *
		 * Used to pass Player references matching the provided flags to the Interface
		 * Can also be used to pass data of a given Player to the Interface
		 */
		static FuncParameter CreateFunctor(unsigned int flags, RakNet::NetworkID id = 0);
#endif
		/**
		 * \brief Retrieves the key associated to the Player's control
		 */
		unsigned char GetPlayerControl(unsigned char control) const;
		/**
		 * \brief Given a control code, returns its enabled state
		 */
		bool GetPlayerControlEnabled(unsigned char control) const;
#ifdef VAULTSERVER
		/**
		 * \brief Returns the Player's respawn time
		 */
		unsigned int GetPlayerRespawnTime() const;
		/**
		 * \brief Returns the Player's spawn cell
		 */
		unsigned int GetPlayerSpawnCell() const;
		/**
		 * \brief Returns the Player's cell context
		 */
		const CellContext& GetPlayerCellContext() const;
		/**
		 * \brief Returns the Player's console state
		 */
		bool GetPlayerConsoleEnabled() const;
		/**
		 * \brief Returns the Player's chatbox enabled state
		 */
		bool GetPlayerChatboxEnabled() const;
		/**
		 * \brief Returns the Player's chatbox lock state
		 */
		bool GetPlayerChatboxLocked() const;
		/**
		 * \brief Returns the Player's chatbox position
		 */
		std::pair<double, double> GetPlayerChatboxPos() const;
		/**
		 * \brief Returns the Player's chatbox size
		 */
		std::pair<double, double> GetPlayerChatboxSize() const;
#endif

		/**
		 * \brief Associates a key with the Player's control code
		 */
		Lockable* SetPlayerControl(unsigned char control, unsigned char key);
		/**
		 * \brief Sets the enabled state of the given control code
		 */
		Lockable* SetPlayerControlEnabled(unsigned char control, bool state);
#ifdef VAULTSERVER
		/**
		 * \brief Sets the respawn time
		 */
		Lockable* SetPlayerRespawnTime(unsigned int respawn);
		/**
		 * \brief Sets the spawn cell
		 */
		Lockable* SetPlayerSpawnCell(unsigned int cell);
		/**
		 * \brief Sets the network cell / network cell context
		 */
		virtual Lockable* SetNetworkCell(unsigned int cell);
		/**
		 * \brief Sets the console state
		 */
		Lockable* SetPlayerConsoleEnabled(bool enabled);
		/**
		 * \brief Sets the chatbox enabled state
		 */
		Lockable* SetPlayerChatboxEnabled(bool enabled);
		/**
		 * \brief Sets the chatbox locked state
		 */
		Lockable* SetPlayerChatboxLocked(bool locked);
		/**
		 * \brief Sets the chatbox position
		 */
		Lockable* SetPlayerChatboxPos(double X, double Y);
		/**
		 * \brief Sets the chatbox size
		 */
		Lockable* SetPlayerChatboxSize(double X, double Y);
#endif

#ifdef VAULTSERVER
		/**
		 * \brief Sets the Player's base ID
		 */
		virtual Lockable* SetBase(unsigned int baseID);
#endif

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#ifndef VAULTSERVER
class PlayerFunctor : public ActorFunctor
{
	public:
		PlayerFunctor(unsigned int flags, RakNet::NetworkID id) : ActorFunctor(flags, id) {}
		virtual ~PlayerFunctor() {}

		virtual std::vector<std::string> operator()();
		virtual bool filter(FactoryObject<Reference>& reference);
};
#endif

#endif
