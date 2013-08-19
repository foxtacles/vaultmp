#ifndef GAME_H
#define GAME_H

#include "vaultmp.h"
#include "Guarded.h"
#include "Player.h"
#include "Network.h"
#include "GameFactory.h"
#include "Item.h"
#include "Button.h"
#include "Text.h"
#include "Edit.h"
#include "Checkbox.h"
#include "RakNet.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#include <utility>
#include <future>
#include <set>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>

/**
 * \brief Client game code, using the Interface to execute commands and communicate with the game
 *
 * Has the command result handler and uses the NetworkClient class, also creates and queues packets
 */

class Game
{
		friend class NetworkClient;
		friend class Bethesda;

	public:
		typedef std::pair<std::future<void>, std::chrono::milliseconds> AsyncPack;
		typedef std::pair<std::set<unsigned int>, std::set<unsigned int>> CellDiff;
		typedef std::unordered_map<unsigned int, std::unordered_map<unsigned int, std::set<unsigned int>>> CellRefs;
		typedef std::unordered_map<unsigned int, std::unordered_set<RakNet::NetworkID>> UninitializedObjects;
		typedef std::unordered_map<unsigned int, std::vector<unsigned int>> DeletedObjects;
		typedef std::unordered_map<unsigned int, unsigned int> BaseRaces;
		typedef std::unordered_map<unsigned int, signed int> Globals;
		typedef unsigned int Weather;
		typedef unsigned int PlayerBase;
		typedef std::function<void()> SpawnFunc;
		typedef std::deque<std::function<void()>> StartupQueue;

	private:
		Game() = delete;

#ifdef VAULTMP_DEBUG
		static DebugInput<Game> debug;
#endif

		static RakNet::RakNetGUID server;

		static void AdjustZAngle(double& Z, double diff);
		static Guarded<CellRefs> cellRefs;
		static Guarded<Player::CellContext> cellContext;
		static Guarded<UninitializedObjects> uninitObj;
		static Guarded<DeletedObjects> deletedObj;
		static Guarded<DeletedObjects> deletedStatic;
		static BaseRaces baseRaces;
		static Globals globals;
		static Weather weather;
		static PlayerBase playerBase;
		static SpawnFunc spawnFunc;
		static Player::CellContext spawnContext;
		static StartupQueue startupQueue;
		static bool GUIMode;
		static bool startup;

	public:
		/**
		 * \brief Handles translated command results from the game
		 */
		static void CommandHandler(unsigned int key, const std::vector<double>& info, double result, bool error);
		/**
		 * \brief Builds an authenticate packet for the server
		 */
		static NetworkResponse Authenticate(const std::string& password);
		/**
		 * \brief Starts the game command schedule
		 */
		static void Startup();
		/**
		 * \brief Future set
		 */
		template <typename T>
		static void FutureSet(const std::weak_ptr<Lockable>& data, T&& t);
		/**
		 * \brief Async task execution
		 */
		static void AsyncDispatch(std::function<void()>&& func);
		/**
		 * \brief Job task execution
		 */
		static void JobDispatch(std::chrono::milliseconds&& time, std::function<void()>&& func);
		/**
		 * \brief Delay or execute
		 */
		static void DelayOrExecute(const FactoryObject& reference, std::function<void(unsigned int)>&& func, unsigned int key = 0x00000000);

		/**
		 * Game functions
		 */

		/**
		 * \brief Loads a savegame
		 */
		static void LoadGame(std::string savegame = std::string());
		/**
		 * \brief Loads an interior cell
		 */
		static void CenterOnCell(const std::string& cell, bool spawn = false);
		/**
		 * \brief Loads a exterior cell
		 */
		static void CenterOnExterior(signed int x, signed int y, bool spawn = false);
		/**
		 * \brief Loads a worldspace and exterior cell
		 */
		static void CenterOnWorld(unsigned int baseID, signed int x, signed int y, bool spawn = false);
		/**
		 * \brief Sets a INI setting
		 */
		static void SetINISetting(const std::string& key, const std::string& value);
		/**
		 * \brief Sets a global value
		 */
		static void SetGlobalValue(unsigned int global, signed int value);
		/**
		 * \brief Loads the environment after game boot
		 */
		static void LoadEnvironment();
		static void NewDispatch(FactoryObject& reference);
		/**
		 * \brief Display a Fallout UI message
		 */
		static void UIMessage(const std::string& message, unsigned char emoticon);
		/**
		 * \brief Display a GUI chat message
		 */
		static void ChatMessage(const std::string& message);
		/**
		 * \brief Creates a new Object
		 */
		static void NewObject(FactoryObject& reference);
		static void NewObject_(FactoryObject& reference);
		/**
		 * \brief Creates a new Item
		 */
		static void NewItem(FactoryItem& reference);
		static void NewItem_(FactoryItem& reference);
		/**
		 * \brief Creates a new Container
		 */
		static void NewContainer(FactoryContainer& reference);
		static void NewContainer_(FactoryContainer& reference);
		/**
		 * \brief Creates a new Actor
		 */
		static void NewActor(FactoryActor& reference);
		static void NewActor_(FactoryActor& reference);
		/**
		 * \brief Creates a new Player
		 */
		static void NewPlayer(FactoryPlayer& reference);
		static void NewPlayer_(FactoryPlayer& reference);
		/**
		 * \brief Removes an Object from the game
		 */
		static void RemoveObject(const FactoryObject& reference);
		static void RemoveObject(unsigned int refID);
		/**
		 * \brief Creates a new Window
		 */
		static void NewWindow(const FactoryWindow& reference);
		/**
		 * \brief Creates a new Button
		 */
		static void NewButton(const FactoryButton& reference);
		/**
		 * \brief Creates a new Text
		 */
		static void NewText(const FactoryText& reference);
		/**
		 * \brief Creates a new Edit
		 */
		static void NewEdit(const FactoryEdit& reference);
		/**
		 * \brief Creates a new Checkbox
		 */
		static void NewCheckbox(const FactoryCheckbox& reference);
		/**
		 * \brief Places an Object in-game
		 */
		static void PlaceAtMe(const FactoryObject& reference, unsigned int baseID, double condition = 1.00, unsigned int count = 1, unsigned int key = 0);
		static void PlaceAtMe(unsigned int refID, unsigned int baseID, double condition = 1.00, unsigned int count = 1, unsigned int key = 0);
		/**
		 * \brief Enables / Disables an Object
		 */
		static void ToggleEnabled(const FactoryObject& reference);
		static void ToggleEnabled(unsigned int refID, bool enabled);
		/**
		 * \brief Deletes an Object
		 */
		static void DestroyObject(FactoryObject& reference, bool silent);
		/**
		 * \brief Deletes a Window
		 */
		static void DeleteWindow(FactoryWindow& reference);
		/**
		 * \brief Returns the baseID of an Object
		 */
		static unsigned int GetBase(unsigned int refID);
		/**
		 * \brief Sets the name of an Object
		 */
		static void SetName(const FactoryObject& reference);
		/**
		 * \brief Puts an Actor into restrained / unrestrained state
		 */
		static void SetRestrained(const FactoryActor& reference, bool restrained);
		/**
		 * \brief Activates an Object
		 */
		static void Activate(const FactoryReference& reference, const FactoryReference& action);
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
		 * \brief Sets the lock level of an Object
		 */
		static void SetLock(const FactoryObject& reference, unsigned int key = 0);
		/**
		 * \brief Sets the owner of an Object
		 */
		static void SetOwner(const FactoryObject& reference, unsigned int key = 0);
		/**
		 * \brief Sets an actor value of an Actor
		 */
		static void SetActorValue(const FactoryActor& reference, bool base, unsigned char index, unsigned int key = 0);
		/**
		 * \brief Damages an actor value of an Actor
		 */
		static void DamageActorValue(const FactoryActor& reference, unsigned char index, double value, unsigned int key = 0);
		/**
		 * \brief Restores an actor value of an Actor
		 */
		static void RestoreActorValue(const FactoryActor& reference, unsigned char index, double value, unsigned int key = 0);
		/**
		 * \brief Sets the sneaking state of an Actor
		 */
		static void SetActorSneaking(const FactoryActor& reference, unsigned int key = 0);
		/**
		 * \brief Sets the alerted state of an Actor
		 */
		static void SetActorAlerted(const FactoryActor& reference, unsigned int key = 0);
		/**
		 * \brief Plays an animation on an Actor
		 */
		static void SetActorAnimation(const FactoryActor& reference, unsigned char anim, unsigned int key = 0);
		/**
		 * \brief Sets the moving animation of an Actor
		 */
		static void SetActorMovingAnimation(const FactoryActor& reference, unsigned int key = 0);
		/**
		 * \brief Sets the weapon animation of an Actor
		 */
		static void SetActorWeaponAnimation(const FactoryActor& reference, unsigned int key = 0);
		/**
		 * \brief Sets the idle animation of an Actor
		 */
		static void SetActorIdleAnimation(const FactoryActor& reference, const std::string& anim, unsigned int key = 0);
		/**
		 * \brief Sets the race of an Actor
		 */
		static void SetActorRace(const FactoryActor& reference, signed int delta_age, unsigned int key = 0);
		/**
		 * \brief Sets the sex of an Actor
		 */
		static void SetActorFemale(const FactoryActor& reference, unsigned int key = 0);
		/**
		 * \brief Kills an Actor
		 */
		static void KillActor(const FactoryActor& reference, unsigned short limbs, signed char cause, unsigned int key = 0);
		/**
		 * \brief Makes an Actor fire a weapon
		 */
		static void FireWeapon(const FactoryActor& reference, unsigned int weapon, unsigned int key = 0);
		/**
		 * \brief Adds an Item to a Container
		 */
		static void AddItem(const FactoryContainer& reference, const FactoryItem& item, unsigned int key = 0);
		static void AddItem(const FactoryContainer& reference, unsigned int baseID, unsigned int count, double condition, bool silent = false, unsigned int key = 0);
		/**
		 * \brief Removes an Item from a Container
		 */
		static void RemoveItem(const FactoryContainer& reference, const FactoryItem& item, unsigned int key = 0);
		static void RemoveItem(const FactoryContainer& reference, unsigned int baseID, unsigned int count, bool silent = false, unsigned int key = 0);
		/**
		 * \brief Sets the reference count of an Item
		 */
		static void SetRefCount(const FactoryItem& reference, unsigned int key = 0);
		/**
		 * \brief Sets the current health of an Item
		 */
		static void SetCurrentHealth(const FactoryItem& reference, unsigned int health, unsigned int key = 0);
		/**
		 * \brief Makes an Actor equip an Item
		 */
		static void EquipItem(const FactoryActor& reference, const FactoryItem& item, unsigned int key = 0);
		static void EquipItem(const FactoryActor& reference, unsigned int baseID, double condition, bool silent = false, bool stick = false, unsigned int key = 0);
		/**
		 * \brief Makes an Actor unequip an Item
		 */
		static void UnequipItem(const FactoryActor& reference, const FactoryItem& item, unsigned int key = 0);
		static void UnequipItem(const FactoryActor& reference, unsigned int baseID, bool silent = false, bool stick = false, unsigned int key = 0);
		/**
		 * \brief Updates the settings of a GUI window (pos)
		 */
		static void SetWindowPos(const FactoryWindow& reference);
		/**
		 * \brief Updates the settings of a GUI window (size)
		 */
		static void SetWindowSize(const FactoryWindow& reference);
		/**
		 * \brief Updates the settings of a GUI window (visible)
		 */
		static void SetWindowVisible(const FactoryWindow& reference);
		/**
		 * \brief Updates the settings of a GUI window (locked)
		 */
		static void SetWindowLocked(const FactoryWindow& reference);
		/**
		 * \brief Updates the settings of a GUI window (text)
		 */
		static void SetWindowText(const FactoryWindow& reference);
		/**
		 * \brief Updates the settings of a GUI edit (max length)
		 */
		static void SetEditMaxLength(const FactoryEdit& reference);
		/**
		 * \brief Updates the settings of a GUI edit (validation)
		 */
		static void SetEditValidation(const FactoryEdit& reference);
		/**
		 * \brief Updates the settings of a GUI checkbox (selected)
		 */
		static void SetCheckboxSelected(const FactoryCheckbox& reference);
		/**
		 * \brief Updates the window mode
		 */
		static void SetWindowMode();
		/**
		 * \brief Enable player controls
		 */
		static void EnablePlayerControls(bool movement = true, bool pipboy = true, bool fighting = true, bool pov = true, bool looking = true, bool rollover = true, bool sneaking = true);
		/**
		 * \brief Disable player controls
		 */
		static void DisablePlayerControls(bool movement = true, bool pipboy = true, bool fighting = true, bool pov = true, bool looking = false, bool rollover = false, bool sneaking = false);
		/**
		 * \brief Enables / disables a key
		 */
		static void ToggleKey(bool enabled, unsigned int scancode);
		/**
		 * \brief Sets the weather
		 */
		static void SetWeather(unsigned int weather);
		/**
		 * \brief Forces the respawn ('back to menu') for the player
		 */
		static void ForceRespawn();
		/**
		 * \brief Tests whether a cell is in the range of the player
		 */
		static bool IsInContext(unsigned int cell);
		/**
		 * \brief Retrieves reference IDs of context objects
		 */
		static std::vector<unsigned int> GetContext(unsigned int type);

		/**
		 * Network functions
		 */

		/**
		 * \brief Network function to handle Object name
		 */
		static void net_SetName(const FactoryObject& reference, const std::string& name);
		/**
		 * \brief Network function to handle Object position
		 */
		static void net_SetPos(const FactoryObject& reference, double X, double Y, double Z);
		/**
		 * \brief Network function to handle Object position
		 */
		static void net_SetAngle(const FactoryObject& reference, double X, double Y, double Z);
		/**
		 * \brief Network function to handle Object cell
		 */
		static void net_SetCell(FactoryObject& reference, FactoryPlayer& player, unsigned int cell, double X, double Y, double Z);
		/**
		 * \brief Network function to handle Object lock level
		 */
		static void net_SetLock(const FactoryObject& reference, unsigned int lock);
		/**
		 * \brief Network function to handle Object owner
		 */
		static void net_SetOwner(const FactoryObject& reference, unsigned int owner);
		/**
		 * \brief Network function to handle Object activate
		 */
		static void net_GetActivate(const FactoryReference& reference, const FactoryReference& action);
		/**
		 * \brief Network function to handle Item count
		 */
		static void net_SetItemCount(FactoryItem& reference, unsigned int count, bool silent);
		/**
		 * \brief Network function to handle Item condition
		 */
		static void net_SetItemCondition(FactoryItem& reference, double condition, unsigned int health);
		/**
		 * \brief Network function to handle Item equipped state
		 */
		static void net_SetItemEquipped(FactoryItem& reference, bool equipped, bool silent, bool stick);
		/**
		 * \brief Network function to handle Actor value
		 */
		static void net_SetActorValue(const FactoryActor& reference, bool base, unsigned char index, double value);
		/**
		 * \brief Network function to handle Actor state
		 */
		static void net_SetActorState(const FactoryActor& reference, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking, bool firing);
		/**
		 * \brief Network function to handle Actor race
		 */
		static void net_SetActorRace(const FactoryActor& reference, unsigned int race, signed int age, signed int delta_age);
		/**
		 * \brief Network function to handle Actor sex
		 */
		static void net_SetActorFemale(const FactoryActor& reference, bool female);
		/**
		 * \brief Network function to handle Actor death
		 */
		static void net_SetActorDead(FactoryActor& reference, bool dead, unsigned short limbs, signed char cause);
		/**
		 * \brief Network function to handle Actor fire weapon
		 */
		static void net_FireWeapon(const FactoryActor& reference, unsigned int weapon, double rate);
		/**
		 * \brief Network function to handle Actor idle animation
		 */
		static void net_SetActorIdle(const FactoryActor& reference, unsigned int idle, const std::string& name);
		/**
		 * \brief Network function to handle interior update
		 */
		static void net_UpdateInterior(const std::string& cell, bool spawn);
		/**
		 * \brief Network function to handle exterior update
		 */
		static void net_UpdateExterior(unsigned int baseID, signed int x, signed int y, bool spawn);
		/**
		 * \brief Network function to handle cell context update
		 */
		static void net_UpdateContext(Player::CellContext& context, bool spawn);
		/**
		 * \brief Network function to handle console update
		 */
		static void net_UpdateConsole(bool enabled);
		/**
		 * \brief Network function to handle GUI window pos
		 */
		static void net_UpdateWindowPos(const FactoryWindow& reference, const std::tuple<double, double, double, double>& pos);
		/**
		 * \brief Network function to handle GUI window size
		 */
		static void net_UpdateWindowSize(const FactoryWindow& reference, const std::tuple<double, double, double, double>& size);
		/**
		 * \brief Network function to handle GUI window visible
		 */
		static void net_UpdateWindowVisible(const FactoryWindow& reference, bool visible);
		/**
		 * \brief Network function to handle GUI window locked
		 */
		static void net_UpdateWindowLocked(const FactoryWindow& reference, bool locked);
		/**
		 * \brief Network function to handle GUI window text
		 */
		static void net_UpdateWindowText(const FactoryWindow& reference, const std::string& text);
		/**
		 * \brief Network function to handle GUI edit max length
		 */
		static void net_UpdateEditMaxLength(const FactoryEdit& reference, unsigned int length);
		/**
		 * \brief Network function to handle GUI edit validation
		 */
		static void net_UpdateEditValidation(const FactoryEdit& reference, const std::string& validation);
		/**
		 * \brief Network function to handle GUI checkbox selected
		 */
		static void net_UpdateCheckboxSelected(const FactoryCheckbox& reference, bool selected);
		/**
		 * \brief Network function to handle GUI window mode
		 */
		static void net_UpdateWindowMode(bool enabled);
		/**
		 * \brief Network function to handle UI message
		 */
		static void net_UIMessage(const std::string& message, unsigned char emoticon);
		/**
		 * \brief Network function to handle chat message
		 */
		static void net_ChatMessage(const std::string& message);
		/**
		 * \brief Network function to handle global value
		 */
		static void net_SetGlobalValue(unsigned int global, signed int value);
		/**
		 * \brief Network function to handle weather update
		 */
		static void net_SetWeather(unsigned int weather);
		/**
		 * \brief Network function to handle player base
		 */
		static void net_SetBase(unsigned int playerBase);
		/**
		 * \brief Network function to handle deleted static references
		 */
		static void net_SetDeletedStatic(DeletedObjects&& deletedStatic);

		/**
		 * Interface result functions
		 */

		/**
		 * \brief Handles GetPos command result
		 */
		static void GetPos(const FactoryObject& reference, double X, double Y, double Z);
		/**
		 * \brief Handles GetAngle command result
		 */
		static void GetAngle(const FactoryObject& reference, double X, double Y, double Z);
		/**
		 * \brief Handles GetParentCell command result
		 */
		static void GetParentCell(const FactoryPlayer& player, unsigned int cell);
		/**
		 * \brief Handles GetActorState command result
		 */
		static void GetActorState(const FactoryActor& reference, unsigned int idle, unsigned char moving, unsigned char weapon, unsigned char flags, bool sneaking);
		/**
		 * \brief Handles GetControl command result
		 */
		static void GetControl(const FactoryPlayer& reference, unsigned char control, unsigned char key);
		/**
		 * \brief Handles GetActivate command result
		 */
		static void GetActivate(const FactoryReference& reference, const FactoryReference& action);
		/**
		 * \brief Handles GUI message
		 */
		static void GetMessage(std::string message);
		/**
		 * \brief Handles GUI mode
		 */
		static void GetWindowMode(bool enabled);
		/**
		 * \brief Handles GUI click
		 */
		static void GetWindowClick(const std::string& name);
		/**
		 * \brief Handles GUI text change
		 */
		static void GetWindowText(const std::string& name, const std::string& text);
		/**
		 * \brief Handles GUI checkbox selected change
		 */
		static void GetCheckboxSelected(const std::string& name, bool selected);
};

template<> struct pTypesMap<pTypes::ID_GAME_AUTH> { typedef pGeneratorDefault<pTypes::ID_GAME_AUTH, std::string, std::string> type; };
template<> struct pTypesMap<pTypes::ID_GAME_LOAD> { typedef pGeneratorDefault<pTypes::ID_GAME_LOAD> type; };
template<> struct pTypesMap<pTypes::ID_GAME_MOD> { typedef pGeneratorDefault<pTypes::ID_GAME_MOD, std::string, unsigned int> type; };
template<> struct pTypesMap<pTypes::ID_GAME_START> { typedef pGeneratorDefault<pTypes::ID_GAME_START> type; };
template<> struct pTypesMap<pTypes::ID_GAME_END> { typedef pGeneratorDefault<pTypes::ID_GAME_END, Reason> type; };
template<> struct pTypesMap<pTypes::ID_GAME_MESSAGE> { typedef pGeneratorDefault<pTypes::ID_GAME_MESSAGE, std::string, unsigned char> type; };
template<> struct pTypesMap<pTypes::ID_GAME_CHAT> { typedef pGeneratorDefault<pTypes::ID_GAME_CHAT, std::string> type; };
template<> struct pTypesMap<pTypes::ID_GAME_GLOBAL> { typedef pGeneratorDefault<pTypes::ID_GAME_GLOBAL, unsigned int, signed int> type; };
template<> struct pTypesMap<pTypes::ID_GAME_WEATHER> { typedef pGeneratorDefault<pTypes::ID_GAME_WEATHER, Game::Weather> type; };
template<> struct pTypesMap<pTypes::ID_GAME_BASE> { typedef pGeneratorDefault<pTypes::ID_GAME_BASE, Game::PlayerBase> type; };
template<> struct pTypesMap<pTypes::ID_GAME_DELETED> { typedef pGeneratorDefault<pTypes::ID_GAME_DELETED, Game::DeletedObjects> type; };

#endif
