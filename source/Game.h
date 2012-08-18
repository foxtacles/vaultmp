#ifndef GAME_H
#define GAME_H

#include "vaultmp.h"
#include "Interface.h"
#include "Player.h"
#include "Network.h"
#include "Shared.h"
#include "VaultException.h"

#include <future>
#include <chrono>
#include <memory>

/**
 * \brief Client game code, using the Interface to execute commands and communicate with the game
 *
 * Has the command result handler and uses the NetworkClient class, also creates and queues packets
 */

class Game
{
		friend class NetworkClient;
		friend class Bethesda;

	private:
		Game() = delete;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		static unsigned char game;
		static RakNetGUID server;

		static void AdjustZAngle(double& Z, double diff);

		typedef pair<future<void>, chrono::milliseconds> AsyncPack;
		typedef pair<set<unsigned int>, set<unsigned int>> CellDiff;
		typedef unordered_map<unsigned int, set<unsigned int>> CellData;

		static CellData cellData;

	public:
		/**
		 * \brief Handles translated command results from the game
		 */
		static void CommandHandler(unsigned int key, const vector<double>& info, double result, bool error);
		/**
		 * \brief Builds an authenticate packet for the server
		 */
		static NetworkResponse Authenticate(string password);
		/**
		 * \brief Starts the game command schedule
		 */
		static void Startup();
		/**
		 * \brief Future set
		 */
		template <typename T>
		static void FutureSet(const weak_ptr<Lockable>& data, T t);
		/**
		 * \brief Async task execution
		 */
		static void AsyncDispatch(function<void()>&& func);

		/**
		 * Game functions
		 */

		/**
		 * \brief Loads a savegame
		 */
		static void LoadGame(string savegame = string());
		/**
		 * \brief Loads an interior cell
		 */
		static void CenterOnCell(string cell = string());
		/**
		 * \brief Loads a exterior cell
		 */
		static void CenterOnExterior(signed int x, signed int y);
		/**
		 * \brief Loads a worldspace and exterior cell
		 */
		static void CenterOnWorld(unsigned int baseID, signed int x, signed int y);
		/**
		 * \brief Loads the environment after savegame load
		 */
		static void LoadEnvironment();
		/**
		 * \brief Display a Fallout UI message
		 */
		static void UIMessage(const string& message);
		/**
		 * \brief Display a GUI chat message
		 */
		static void ChatMessage(const string& message);
		/**
		 * \brief Creates a new Object
		 */
		static void NewObject(FactoryObject& reference);
		/**
		 * \brief Creates a new Item
		 */
		static void NewItem(FactoryObject& reference);
		/**
		 * \brief Creates a new Container
		 */
		static void NewContainer(FactoryObject& reference);
		/**
		 * \brief Creates a new Actor
		 */
		static void NewActor(FactoryObject& reference);
		/**
		 * \brief Creates a new Player
		 */
		static void NewPlayer(FactoryObject& reference);
		/**
		 * \brief Removes an Object from the game
		 */
		static void RemoveObject(const FactoryObject& reference);
		/**
		 * \brief Places an Object in-game
		 */
		static void PlaceAtMe(const FactoryObject& reference, unsigned int baseID, unsigned int count, unsigned int key = 0);
		static void PlaceAtMe(unsigned int refID, unsigned int baseID, unsigned int count, unsigned int key = 0);
		/**
		 * \brief Enables / Disables an Object
		 */
		static void ToggleEnabled(const FactoryObject& reference);
		/**
		 * \brief Deletes an Object
		 */
		static void Delete(FactoryObject& reference);
		/**
		 * \brief Sets the name of an Object
		 */
		static void SetName(const FactoryObject& reference);
		/**
		 * \brief Puts an Actor into restrained / unrestrained state
		 */
		static void SetRestrained(const FactoryObject& reference, bool restrained);
		/**
		 * \brief Sets the position of an Object
		 */
		static void SetPos(const FactoryObject& reference);
		/**
		 * \brief Sets the angles of an Object
		 */
		static void SetAngle(const FactoryObject& reference);
		/**
		 * \brief Moves an Object to another Object
		 */
		static void MoveTo(const FactoryObject& reference, const FactoryObject& object, bool cell = false, unsigned int key = 0);
		/**
		 * \brief Sets an actor value of an Actor
		 */
		static void SetActorValue(const FactoryObject& reference, bool base, unsigned char index, unsigned int key = 0);
		/**
		 * \brief Sets the sneaking state of an Actor
		 */
		static function<void()> SetActorSneaking(const FactoryObject& reference, unsigned int key = 0);
		/**
		 * \brief Sets the alerted state of an Actor
		 */
		static function<void()> SetActorAlerted(const FactoryObject& reference, unsigned int key = 0);
		/**
		 * \brief Plays an animation on an Actor
		 */
		static void SetActorAnimation(const FactoryObject& reference, unsigned char anim, unsigned int key = 0);
		/**
		 * \brief Sets the moving animation of an Actor
		 */
		static void SetActorMovingAnimation(const FactoryObject& reference, unsigned int key = 0);
		/**
		 * \brief Sets the weapon animation of an Actor
		 */
		static void SetActorWeaponAnimation(const FactoryObject& reference, unsigned int key = 0);
		/**
		 * \brief Kills an Actor
		 */
		static void KillActor(const FactoryObject& reference, unsigned short limbs, signed char cause, unsigned int key = 0);
		/**
		 * \brief Makes an Actor fire a weapon
		 */
		static void FireWeapon(const FactoryObject& reference, unsigned int weapon, unsigned int key = 0);
		/**
		 * \brief Adds an Item to a Container
		 */
		static void AddItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key = 0);
		static void AddItem(const FactoryObject& reference, unsigned int baseID, unsigned int count, double condition, bool silent = false, unsigned int key = 0);
		/**
		 * \brief Removes an Item from a Container
		 */
		static void RemoveItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key = 0);
		static void RemoveItem(const FactoryObject& reference, unsigned int baseID, unsigned int count, bool silent = false, unsigned int key = 0);
		/**
		 * \brief Removes all items from a Container
		 */
		static void RemoveAllItems(const FactoryObject& reference, unsigned int key = 0);
		/**
		 * \brief Removes all (including fixed equipment) items from a Container
		 */
		static void RemoveAllItemsEx(FactoryObject& reference);
		/**
		 * \brief Makes an Actor equip an Item
		 */
		static void EquipItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key = 0);
		static void EquipItem(const FactoryObject& reference, unsigned int baseID, bool silent = false, bool stick = false, unsigned int key = 0);
		/**
		 * \brief Makes an Actor unequip an Item
		 */
		static void UnequipItem(const FactoryObject& reference, const FactoryObject& item, unsigned int key = 0);
		static void UnequipItem(const FactoryObject& reference, unsigned int baseID, bool silent = false, bool stick = false, unsigned int key = 0);
		/**
		 * \brief Scans a cell for forms and returns the delta to previous scan
		 */
		static CellDiff ScanCell(unsigned int type = UINT_MAX, unsigned int depth = 0, bool taken = false);

		/**
		 * Network functions
		 */

		/**
		 * \brief Network function to handle Object position
		 */
		static void net_SetPos(const FactoryObject& reference, double X, double Y, double Z);
		/**
		 * \brief Network function to handle Object position
		 */
		static void net_SetAngle(const FactoryObject& reference, unsigned char axis, double value);
		/**
		 * \brief Network function to handle Object cell
		 */
		static void net_SetCell(const FactoryObject& reference, const FactoryObject& player, unsigned int cell);
		/**
		 * \brief Network function to handle Container update
		 */
		static void net_ContainerUpdate(FactoryObject& reference, const pair<list<NetworkID>, vector<pPacket>>& _diff);
		/**
		 * \brief Network function to handle Actor value
		 */
		static void net_SetActorValue(const FactoryObject& reference, bool base, unsigned char index, double value);
		/**
		 * \brief Network function to handle Actor state
		 */
		static void net_SetActorState(const FactoryObject& reference, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking, bool firing);
		/**
		 * \brief Network function to handle Actor death
		 */
		static void net_SetActorDead(FactoryObject& reference, bool dead, unsigned short limbs, signed char cause);
		/**
		 * \brief Network function to handle Actor fire weapon
		 */
		static void net_FireWeapon(const FactoryObject& reference, unsigned int weapon, double rate);
		/**
		 * \brief Network function to handle UI message
		 */
		static void net_UIMessage(const string& message);
		/**
		 * \brief Network function to handle chat message
		 */
		static void net_ChatMessage(const string& message);

		/**
		 * Interface functions
		 */

		/**
		 * \brief Handles GetPos command result
		 */
		static void GetPos(const FactoryObject& reference, unsigned char axis, double value);
		/**
		 * \brief Handles GetAngle command result
		 */
		static void GetAngle(const FactoryObject& reference, unsigned char axis, double value);
		/**
		 * \brief Handles GetParentCell command result
		 */
		static void GetParentCell(const FactoryObject& reference, const FactoryObject& player, unsigned int cell);
		/**
		 * \brief Handles GetDead command result
		 */
		static void GetDead(const FactoryObject& reference, const FactoryObject& actor, bool dead);
		/**
		 * \brief Handles IsLimbGone command result
		 */
		static void IsLimbGone(unsigned int key, unsigned char limb, bool gone);
		/**
		 * \brief Handles GetActorValue command result
		 */
		static void GetActorValue(const FactoryObject& reference, bool base, unsigned char index, double value);
		/**
		 * \brief Handles GetActorState command result
		 */
		static void GetActorState(const FactoryObject& reference, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool sneaking);
		/**
		 * \brief Handles GetControl command result
		 */
		static void GetControl(const FactoryObject& reference, unsigned char control, unsigned char key);
		/**
		 * \brief Handles ScanContainer command result
		 */
		static void ScanContainer(const FactoryObject& reference, vector<unsigned char>& data);
		/**
		 * \brief Handles RemoveAllItemsEx command result
		 */
		static void GetRemoveAllItemsEx(const FactoryObject& reference, vector<unsigned char>& data);
		/**
		 * \brief Handles GetFirstRef / GetNextRef command result
		 */
		static void GetNextRef(unsigned int key, unsigned int refID);
		/**
		 * \brief Handles GUI message
		 */
		static void GetMessage(string message);

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif
};

#endif
