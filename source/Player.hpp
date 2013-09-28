#ifndef PLAYER_H
#define PLAYER_H

#include "vaultmp.hpp"
#include "Actor.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

#include <unordered_set>

const unsigned int FLAG_MOVCONTROLS = FLAG_ALIVE << 1;

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
		typedef std::vector<RakNet::NetworkID> AttachedWindows;
		typedef std::vector<unsigned int> BaseIDTracker;
		typedef std::unordered_map<RakNet::NetworkID, std::vector<RakNet::NetworkID>> WindowTracker;

	private:
#ifdef VAULTMP_DEBUG
		static DebugInput<Player> debug;
#endif

#ifdef VAULTSERVER
		static Guarded<BaseIDTracker> baseIDs;
		static Guarded<WindowTracker> attachedWindows;

		static std::atomic<unsigned int> default_respawn;
		static std::atomic<unsigned int> default_cell;
		static std::atomic<bool> default_console;

		Value<unsigned int> player_Respawn;
		Value<unsigned int> player_Cell;
		Value<CellContext> player_CellContext;
		Value<bool> state_Console;
		Value<AttachedWindows> player_Windows;
#endif

		std::unordered_map<unsigned char, std::pair<Value<unsigned char>, Value<bool>>> player_Controls;

		void initialize();

		Player(const Player&) = delete;
		Player& operator=(const Player&) = delete;

	protected:
		Player(unsigned int refID, unsigned int baseID);
		Player(unsigned int baseID) : Player(0x00000000, baseID) {}
		Player(const pPacket& packet);

	public:
		virtual ~Player() noexcept;

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
		static BaseIDTracker GetBaseIDs() { return baseIDs.Operate([](BaseIDTracker& baseIDs) { return baseIDs; }); }
		/**
		 * \brief Returns the set of players who have a given window attached
		 */
		static WindowTracker::mapped_type GetWindowPlayers(RakNet::NetworkID id) { return attachedWindows.Operate([id](WindowTracker& attachedWindows) { return attachedWindows[id]; }); }
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
		 * \brief Returns the Player's attached windows
		 */
		const AttachedWindows& GetPlayerWindows() const { return *player_Windows; };
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
		 * \brief Attaches a window
		 */
		Lockable* AttachWindow(RakNet::NetworkID id);
		/**
		 * \brief Detaches a window
		 */
		Lockable* DetachWindow(RakNet::NetworkID id);
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
		virtual bool filter(FactoryWrapper<Reference>& reference);
};
#endif

GF_TYPE_WRAPPER_FINAL(Player, Actor, ID_PLAYER)

PF_MAKE(ID_PLAYER_NEW, pGeneratorReferenceExtend, std::map<unsigned char, std::pair<unsigned char, bool>>)
PF_MAKE(ID_UPDATE_CONTROL, pGeneratorReference, unsigned char, unsigned char)
PF_MAKE(ID_UPDATE_INTERIOR, pGeneratorReference, std::string, bool)
PF_MAKE(ID_UPDATE_EXTERIOR, pGeneratorReference, unsigned int, signed int, signed int, bool)
PF_MAKE(ID_UPDATE_CONTEXT, pGeneratorReference, std::array<unsigned int, 9>, bool)
PF_MAKE(ID_UPDATE_CONSOLE, pGeneratorReference, bool)

#endif
