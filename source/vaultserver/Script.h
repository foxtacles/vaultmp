#ifndef SCRIPT_H
#define SCRIPT_H

#include "vaultserver.h"
#include "RakNet.h"
#include "Data.h"
#include "ItemList.h"
#include "ScriptFunction.h"
#include "GameFactory.h"
#include "Dedicated.h"
#include "PAWN.h"
#include "boost/any.hpp"

#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>

/**
 * \brief Maintains communication with a script
 *
 * A script can be either a C++ or PAWN script
 */

template<typename T> struct sizeof_void { enum { value = sizeof(T) }; };
template<> struct sizeof_void<void> { enum { value = 0 }; };

template<typename T, size_t t> struct TypeChar { static_assert(!t, "Unsupported type in variadic type list"); };
template<typename T> struct TypeChar<T*, sizeof(void*)> { enum { value = 'p' }; };
template<> struct TypeChar<double*, sizeof(double*)> { enum { value = 'd' }; };
template<> struct TypeChar<RakNet::NetworkID**, sizeof(RakNet::NetworkID**)> { enum { value = 'n' }; };
template<typename T> struct TypeChar<T, sizeof(uint8_t)> { enum { value = std::is_signed<T>::value ? 'q' : 'i' }; };
template<typename T> struct TypeChar<T, sizeof(uint16_t)> { enum { value = std::is_signed<T>::value ? 'q' : 'i' }; };
template<typename T> struct TypeChar<T, sizeof(uint32_t)> { enum { value = std::is_signed<T>::value ? 'q' : 'i' }; };
template<typename T> struct TypeChar<T, sizeof(uint64_t)> { enum { value = std::is_signed<T>::value ? 'w' : 'l' }; };
template<> struct TypeChar<double, sizeof(double)> { enum { value = 'f' }; };
template<> struct TypeChar<char*, sizeof(char*)> { enum { value = 's' }; };
template<> struct TypeChar<const char*, sizeof(const char*)> { enum { value = 's' }; };
template<> struct TypeChar<void, sizeof_void<void>::value> { enum { value = 'v' }; };

template<const char t> struct CharType { static_assert(!t, "Unsupported type in variadic type list"); };
template<> struct CharType<'p'> { typedef void* type; };
template<> struct CharType<'d'> { typedef double* type; };
template<> struct CharType<'n'> { typedef RakNet::NetworkID** type; };
template<> struct CharType<'q'> { typedef signed int type; };
template<> struct CharType<'i'> { typedef unsigned int type; };
template<> struct CharType<'w'> { typedef signed long long type; };
template<> struct CharType<'l'> { typedef unsigned long long type; };
template<> struct CharType<'f'> { typedef double type; };
template<> struct CharType<'s'> { typedef const char* type; };
template<> struct CharType<'v'> { typedef void type; };

template<typename... Types>
struct TypeString {
	static constexpr char value[sizeof...(Types) + 1] = {
		TypeChar<typeof(Types), sizeof(Types)>::value...
	};
};

template<typename R, typename... Types>
using Function = R(*)(Types...);

template<typename R>
using FunctionEllipsis = R(*)(...);

struct ScriptIdentity
{
	const char* types;
	const char ret;
	const unsigned int numargs;

	constexpr bool matches(const char* types, const unsigned int N = 0) {
		return N < numargs ? this->types[N] == types[N] && matches(types, N + 1) : this->types[N] == types[N];
	}

	template<typename R, typename... Types>
	constexpr ScriptIdentity(Function<R, Types...>) : types(TypeString<Types...>::value), ret(TypeChar<R, sizeof_void<R>::value>::value), numargs(sizeof(TypeString<Types...>::value) - 1) {}
};

struct ScriptFunctionPointer : public ScriptIdentity
{
	Function<void> addr;

	template<typename R, typename... Types>
	constexpr ScriptFunctionPointer(Function<R, Types...> addr) : ScriptIdentity(addr), addr(reinterpret_cast<Function<void>>(addr)) {}
};

struct ScriptFunctionData
{
	const char* name;
	const ScriptFunctionPointer func;

	constexpr ScriptFunctionData(const char* name, ScriptFunctionPointer func) : name(name), func(func) {}
};

struct ScriptCallbackData
{
	const char* name;
	const unsigned int index;
	const ScriptIdentity callback;

	template<size_t N>
	constexpr ScriptCallbackData(const char(&name)[N], ScriptIdentity callback) : name(name), index(Utils::hash(name)), callback(callback) {}
};

class Script
{
	private:
		Script(char* path);

#ifdef __WIN32__
		typedef HMODULE lib_t;
#else
		typedef void* lib_t;
#endif

		union
		{
			lib_t lib;
			AMX* amx;
		};

		bool cpp_script;
		std::unordered_map<unsigned int, FunctionEllipsis<void>> callbacks_;

		static void GetArguments(std::vector<boost::any>& params, va_list args, const std::string& def);

		template<typename R>
		R GetScript(const char* name)
		{
			if (cpp_script)
			{
#ifdef __WIN32__
				return reinterpret_cast<R>(GetProcAddress(lib, name));
#else
				return reinterpret_cast<R>(dlsym(lib, name));
#endif
			}
			else
				return reinterpret_cast<R>(PAWN::IsCallbackPresent(amx, name));
		}

		typedef std::vector<std::unique_ptr<Script>> ScriptList;
		typedef std::unordered_map<RakNet::NetworkID, std::unique_ptr<ItemList>> ScriptItemLists;
		typedef std::unordered_map<unsigned int, std::vector<unsigned int>> DeletedObjects;
		typedef std::pair<std::chrono::system_clock::time_point, double> GameTime;
		typedef unsigned int GameWeather;

		static ScriptList scripts;
		static ScriptItemLists scriptIL;
		static DeletedObjects deletedStatic;
		static GameTime time;
		static GameWeather weather;

		Script(const Script&) = delete;
		Script& operator=(const Script&) = delete;

	public:
		~Script();

		static void LoadScripts(char* scripts, char* base);
		static void Initialize();
		static void UnloadScripts();

		static RakNet::NetworkID CreateTimer(ScriptFunc timer, unsigned int interval);
		static RakNet::NetworkID CreateTimerEx(ScriptFunc timer, unsigned int interval, const char* def, ...);
		static RakNet::NetworkID CreateTimerPAWN(ScriptFuncPAWN timer, AMX* amx, unsigned int interval);
		static RakNet::NetworkID CreateTimerPAWNEx(ScriptFuncPAWN timer, AMX* amx, unsigned int interval, const char* def, const std::vector<boost::any>& args);
		static void SetupObject(FactoryObject& object, FactoryObject& reference, unsigned int cell, double X, double Y, double Z);
		static void SetupItem(FactoryItem& item, FactoryObject& reference, unsigned int cell, double X, double Y, double Z);
		static void SetupContainer(FactoryContainer& container, FactoryObject& reference, unsigned int cell, double X, double Y, double Z);
		static void SetupActor(FactoryActor& actor, FactoryObject& reference, unsigned int cell, double X, double Y, double Z);
		static void SetupWindow(FactoryWindow& window, double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text);
		static void SetupButton(FactoryButton& button, double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text);
		static void SetupText(FactoryText& text, double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text_);
		static void SetupEdit(FactoryEdit& edit, double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text);
		static void KillTimer(RakNet::NetworkID id = 0);
		static void MakePublic(ScriptFunc _public, const char* name, const char* def);
		static void MakePublicPAWN(ScriptFuncPAWN _public, AMX* amx, const char* name, const char* def);
		static unsigned long long CallPublic(const char* name, ...);
		static unsigned long long CallPublicPAWN(const char* name, const std::vector<boost::any>& args);
		static const DeletedObjects& GetDeletedStatic() { return deletedStatic; }

		static unsigned long long Timer_Respawn(RakNet::NetworkID id);
		static unsigned long long Timer_GameTime();

		static const char* ValueToString(unsigned char index);
		static const char* AxisToString(unsigned char index);
		static const char* AnimToString(unsigned char index);
		static const char* BaseToString(unsigned int baseID);

		static bool Kick(RakNet::NetworkID id);
		static bool UIMessage(RakNet::NetworkID id, const char* message, unsigned char emoticon);
		static bool ChatMessage(RakNet::NetworkID id, const char* message);
		static void SetSpawnCell(unsigned int cell);
		static void SetGameWeather(unsigned int weather);
		static void SetGameTime(signed long long time);
		static void SetGameYear(unsigned int year);
		static void SetGameMonth(unsigned int month);
		static void SetGameDay(unsigned int day);
		static void SetGameHour(unsigned int hour);
		static void SetTimeScale(double scale);
		static bool IsValid(RakNet::NetworkID id);
		static bool IsObject(RakNet::NetworkID id);
		static bool IsItem(RakNet::NetworkID id);
		static bool IsContainer(RakNet::NetworkID id);
		static bool IsActor(RakNet::NetworkID id);
		static bool IsPlayer(RakNet::NetworkID id);
		static bool IsInterior(unsigned int cell);
		static bool IsItemList(RakNet::NetworkID id);
		static bool IsWindow(RakNet::NetworkID id);
		static bool IsButton(RakNet::NetworkID id);
		static bool IsText(RakNet::NetworkID id);
		static bool IsEdit(RakNet::NetworkID id);
		static bool IsChatbox(RakNet::NetworkID id);
		static unsigned int GetConnection(RakNet::NetworkID id);
		static unsigned int GetList(unsigned int type, RakNet::NetworkID** data);
		static unsigned int GetGameWeather();
		static signed long long GetGameTime();
		static unsigned int GetGameYear();
		static unsigned int GetGameMonth();
		static unsigned int GetGameDay();
		static unsigned int GetGameHour();
		static double GetTimeScale();

		static RakNet::NetworkID GetID(unsigned int refID);
		static unsigned int GetReference(RakNet::NetworkID id);
		static unsigned int GetBase(RakNet::NetworkID id);
		static void GetPos(RakNet::NetworkID id, double* X, double* Y, double* Z);
		static void GetAngle(RakNet::NetworkID id, double* X, double* Y, double* Z);
		static unsigned int GetCell(RakNet::NetworkID id);
		static unsigned int GetLock(RakNet::NetworkID id);
		static unsigned int GetOwner(RakNet::NetworkID id);
		static const char* GetBaseName(RakNet::NetworkID id);
		static bool IsNearPoint(RakNet::NetworkID id, double X, double Y, double Z, double R);
		static RakNet::NetworkID GetItemContainer(RakNet::NetworkID id);
		static unsigned int GetItemCount(RakNet::NetworkID id);
		static double GetItemCondition(RakNet::NetworkID id);
		static bool GetItemEquipped(RakNet::NetworkID id);
		static bool GetItemSilent(RakNet::NetworkID id);
		static bool GetItemStick(RakNet::NetworkID id);
		static unsigned int GetContainerItemCount(RakNet::NetworkID id, unsigned int baseID);
		static unsigned int GetContainerItemList(RakNet::NetworkID id, RakNet::NetworkID** data);
		static double GetActorValue(RakNet::NetworkID id, unsigned char index);
		static double GetActorBaseValue(RakNet::NetworkID id, unsigned char index);
		static unsigned int GetActorIdleAnimation(RakNet::NetworkID id);
		static unsigned char GetActorMovingAnimation(RakNet::NetworkID id);
		static unsigned char GetActorWeaponAnimation(RakNet::NetworkID id);
		static bool GetActorAlerted(RakNet::NetworkID id);
		static bool GetActorSneaking(RakNet::NetworkID id);
		static bool GetActorDead(RakNet::NetworkID id);
		static unsigned int GetActorBaseRace(RakNet::NetworkID id);
		static bool GetActorBaseSex(RakNet::NetworkID id);
		static bool IsActorJumping(RakNet::NetworkID id);
		static unsigned int GetPlayerRespawnTime(RakNet::NetworkID id);
		static unsigned int GetPlayerSpawnCell(RakNet::NetworkID id);
		static bool GetPlayerConsoleEnabled(RakNet::NetworkID id);
		static unsigned int GetPlayerWindowCount(RakNet::NetworkID id);
		static unsigned int GetPlayerWindowList(RakNet::NetworkID id, RakNet::NetworkID** data);
		static RakNet::NetworkID GetPlayerChatboxWindow(RakNet::NetworkID id);

		static RakNet::NetworkID CreateObject(unsigned int baseID, RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z);
		static bool DestroyObject(RakNet::NetworkID id);
		static bool SetPos(RakNet::NetworkID id, double X, double Y, double Z);
		static bool SetAngle(RakNet::NetworkID id, double X, double Y, double Z);
		static bool SetCell(RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z);
		static bool SetCell_(RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z, bool nosend);
		static bool SetLock(RakNet::NetworkID id, RakNet::NetworkID actor, unsigned int lock);
		static bool SetOwner(RakNet::NetworkID id, unsigned int owner);
		static bool SetBaseName(RakNet::NetworkID id, const char* name);
		static RakNet::NetworkID CreateItem(unsigned int baseID, RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z);
		static RakNet::NetworkID SetItemContainer(RakNet::NetworkID id, RakNet::NetworkID container);
		static bool SetItemCount(RakNet::NetworkID id, unsigned int count);
		static bool SetItemCondition(RakNet::NetworkID id, double condition);
		static bool SetItemEquipped(RakNet::NetworkID id, bool equipped, bool silent, bool stick);
		static RakNet::NetworkID CreateContainer(unsigned int baseID, RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z);
		static RakNet::NetworkID CreateItemList(RakNet::NetworkID source, unsigned int baseID);
		static RakNet::NetworkID AddItem(RakNet::NetworkID id, unsigned int baseID, unsigned int count, double condition, bool silent);
		static void AddItemList(RakNet::NetworkID id, RakNet::NetworkID source, unsigned int baseID);
		static unsigned int RemoveItem(RakNet::NetworkID id, unsigned int baseID, unsigned int count, bool silent);
		static void RemoveAllItems(RakNet::NetworkID id);
		static RakNet::NetworkID CreateActor(unsigned int baseID, RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z);
		static void SetActorValue(RakNet::NetworkID id, unsigned char index, double value);
		static void SetActorBaseValue(RakNet::NetworkID id, unsigned char index, double value);
		static bool EquipItem(RakNet::NetworkID id, unsigned int baseID, bool silent, bool stick);
		static bool UnequipItem(RakNet::NetworkID id, unsigned int baseID, bool silent, bool stick);
		static bool PlayIdle(RakNet::NetworkID id, unsigned int idle);
		static bool SetActorMovingAnimation(RakNet::NetworkID id, unsigned char anim);
		static bool SetActorWeaponAnimation(RakNet::NetworkID id, unsigned char anim);
		static bool SetActorAlerted(RakNet::NetworkID id, bool alerted);
		static bool SetActorSneaking(RakNet::NetworkID id, bool sneaking);
		static bool FireWeapon(RakNet::NetworkID id);
		static void KillActor(RakNet::NetworkID id, RakNet::NetworkID actor, unsigned short limbs, signed char cause);
		static bool SetActorBaseRace(RakNet::NetworkID id, unsigned int race);
		static bool AgeActorBaseRace(RakNet::NetworkID id, signed int age);
		static bool SetActorBaseSex(RakNet::NetworkID id, bool female);
		static void SetPlayerRespawnTime(RakNet::NetworkID id, unsigned int respawn);
		static void SetPlayerSpawnCell(RakNet::NetworkID id, unsigned int cell);
		static void SetPlayerConsoleEnabled(RakNet::NetworkID id, bool enabled);
		static bool AttachWindow(RakNet::NetworkID id, RakNet::NetworkID window);
		static bool DetachWindow(RakNet::NetworkID id, RakNet::NetworkID window);
		static void ForceWindowMode(RakNet::NetworkID id, bool enabled);

		static RakNet::NetworkID GetParentWindow(RakNet::NetworkID id);
		static RakNet::NetworkID GetWindowRoot(RakNet::NetworkID id);
		static unsigned int GetWindowChildCount(RakNet::NetworkID id);
		static unsigned int GetWindowChildList(RakNet::NetworkID id, RakNet::NetworkID** data);
		static void GetWindowPos(RakNet::NetworkID id, double* X, double* Y, double* offset_X, double* offset_Y);
		static void GetWindowSize(RakNet::NetworkID id, double* X, double* Y, double* offset_X, double* offset_Y);
		static bool GetWindowVisible(RakNet::NetworkID id);
		static bool GetWindowLocked(RakNet::NetworkID id);
		static const char* GetWindowText(RakNet::NetworkID id);
		static unsigned int GetEditMaxLength(RakNet::NetworkID id);
		static const char* GetEditValidation(RakNet::NetworkID id);

		static RakNet::NetworkID (CreateWindow)(double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text);
		static bool DestroyWindow(RakNet::NetworkID id);
		static bool AddChildWindow(RakNet::NetworkID id, RakNet::NetworkID child);
		static bool RemoveChildWindow(RakNet::NetworkID id, RakNet::NetworkID child);
		static bool SetWindowPos(RakNet::NetworkID id, double X, double Y, double offset_X, double offset_Y);
		static bool SetWindowSize(RakNet::NetworkID id, double X, double Y, double offset_X, double offset_Y);
		static bool SetWindowVisible(RakNet::NetworkID id, bool visible);
		static bool SetWindowLocked(RakNet::NetworkID id, bool locked);
		static bool SetWindowText(RakNet::NetworkID id, const char* text);
		static RakNet::NetworkID CreateButton(double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text);
		static RakNet::NetworkID CreateText(double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text);
		static RakNet::NetworkID CreateEdit(double posX, double posY, double offset_posX, double offset_posY, double sizeX, double sizeY, double offset_sizeX, double offset_sizeY, bool visible, bool locked, const char* text);
		static bool SetEditMaxLength(RakNet::NetworkID id, unsigned int length);
		static bool SetEditValidation(RakNet::NetworkID id, const char* validation);

		static constexpr ScriptFunctionData functions[] {
			{"timestamp", Utils::timestamp},
			{"CreateTimer", Script::CreateTimer},
			{"CreateTimerEx", reinterpret_cast<void(*)()>(Script::CreateTimerEx)},
			{"KillTimer", Script::KillTimer},
			{"MakePublic", Script::MakePublic},
			{"CallPublic", reinterpret_cast<void(*)()>(Script::CallPublic)},

			{"SetServerName", Dedicated::SetServerName},
			{"SetServerMap", Dedicated::SetServerMap},
			{"SetServerRule", Dedicated::SetServerRule},
			{"GetMaximumPlayers", Dedicated::GetMaximumPlayers},
			{"GetCurrentPlayers", Dedicated::GetCurrentPlayers},

			{"ValueToString", Script::ValueToString},
			{"AxisToString", Script::AxisToString},
			{"AnimToString", Script::AnimToString},
			{"BaseToString", Script::BaseToString},

			{"Kick", Script::Kick},
			{"UIMessage", Script::UIMessage},
			{"ChatMessage", Script::ChatMessage},
			{"SetRespawnTime", Player::SetRespawnTime},
			{"SetSpawnCell", Script::SetSpawnCell},
			{"SetConsoleEnabled", Player::SetConsoleEnabled},
			{"SetGameWeather", Script::SetGameWeather},
			{"SetGameTime", Script::SetGameTime},
			{"SetGameYear", Script::SetGameYear},
			{"SetGameMonth", Script::SetGameMonth},
			{"SetGameDay", Script::SetGameDay},
			{"SetGameHour", Script::SetGameHour},
			{"SetTimeScale", Script::SetTimeScale},
			{"IsValid", Script::IsValid},
			{"IsObject", Script::IsObject},
			{"IsItem", Script::IsItem},
			{"IsContainer", Script::IsContainer},
			{"IsActor", Script::IsActor},
			{"IsPlayer", Script::IsPlayer},
			{"IsCell", DB::Record::IsValidCell},
			{"IsInterior", Script::IsInterior},
			{"IsItemList", Script::IsItemList},
			{"IsWindow", Script::IsWindow},
			{"IsButton", Script::IsButton},
			{"IsText", Script::IsText},
			{"IsEdit", Script::IsEdit},
			{"IsChatbox", Script::IsChatbox},
			{"GetType", (unsigned int(*)(RakNet::NetworkID)) GameFactory::GetType},
			{"GetConnection", Script::GetConnection},
			{"GetCount", GameFactory::GetObjectCount},
			{"GetList", Script::GetList},
			{"GetRespawnTime", Player::GetRespawnTime},
			{"GetSpawnCell", Player::GetSpawnCell},
			{"GetConsoleEnabled", Player::GetConsoleEnabled},
			{"GetGameWeather", Script::GetGameWeather},
			{"GetGameTime", Script::GetGameTime},
			{"GetGameYear", Script::GetGameYear},
			{"GetGameMonth", Script::GetGameMonth},
			{"GetGameDay", Script::GetGameDay},
			{"GetGameHour", Script::GetGameHour},
			{"GetTimeScale", Script::GetTimeScale},

			{"GetID", Script::GetID},
			{"GetReference", Script::GetReference},
			{"GetBase", Script::GetBase},
			{"GetPos", Script::GetPos},
			{"GetAngle", Script::GetAngle},
			{"GetCell", Script::GetCell},
			{"GetLock", Script::GetLock},
			{"GetOwner", Script::GetOwner},
			{"GetBaseName", Script::GetBaseName},
			{"IsNearPoint", Script::IsNearPoint},
			{"GetItemContainer", Script::GetItemContainer},
			{"GetItemCount", Script::GetItemCount},
			{"GetItemCondition", Script::GetItemCondition},
			{"GetItemEquipped", Script::GetItemEquipped},
			{"GetItemSilent", Script::GetItemSilent},
			{"GetItemStick", Script::GetItemStick},
			{"GetContainerItemCount", Script::GetContainerItemCount},
			{"GetContainerItemList", Script::GetContainerItemList},
			{"GetActorValue", Script::GetActorValue},
			{"GetActorBaseValue", Script::GetActorBaseValue},
			{"GetActorIdleAnimation", Script::GetActorIdleAnimation},
			{"GetActorMovingAnimation", Script::GetActorMovingAnimation},
			{"GetActorWeaponAnimation", Script::GetActorWeaponAnimation},
			{"GetActorAlerted", Script::GetActorAlerted},
			{"GetActorSneaking", Script::GetActorSneaking},
			{"GetActorDead", Script::GetActorDead},
			{"GetActorBaseRace", Script::GetActorBaseRace},
			{"GetActorBaseSex", Script::GetActorBaseSex},
			{"IsActorJumping", Script::IsActorJumping},
			{"GetPlayerRespawnTime", Script::GetPlayerRespawnTime},
			{"GetPlayerSpawnCell", Script::GetPlayerSpawnCell},
			{"GetPlayerConsoleEnabled", Script::GetPlayerConsoleEnabled},
			{"GetPlayerWindowCount", Script::GetPlayerWindowCount},
			{"GetPlayerWindowList", Script::GetPlayerWindowList},
			{"GetPlayerChatboxWindow", Script::GetPlayerChatboxWindow},

			{"CreateObject", Script::CreateObject},
			{"DestroyObject", Script::DestroyObject},
			{"SetPos", Script::SetPos},
			{"SetAngle", Script::SetAngle},
			{"SetCell", Script::SetCell},
			{"SetLock", Script::SetLock},
			{"SetOwner", Script::SetOwner},
			{"SetBaseName", Script::SetBaseName},
			{"CreateItem", Script::CreateItem},
			{"SetItemContainer", Script::SetItemContainer},
			{"SetItemCount", Script::SetItemCount},
			{"SetItemCondition", Script::SetItemCondition},
			{"SetItemEquipped", Script::SetItemEquipped},
			{"CreateContainer", Script::CreateContainer},
			{"CreateItemList", Script::CreateItemList},
			{"AddItem", Script::AddItem},
			{"RemoveItem", Script::RemoveItem},
			{"RemoveAllItems", Script::RemoveAllItems},
			{"AddItemList", Script::AddItemList},
			{"CreateActor", Script::CreateActor},
			{"SetActorValue", Script::SetActorValue},
			{"SetActorBaseValue", Script::SetActorBaseValue},
			{"EquipItem", Script::EquipItem},
			{"UnequipItem", Script::UnequipItem},
			{"PlayIdle", Script::PlayIdle},
			{"SetActorMovingAnimation", Script::SetActorMovingAnimation},
			{"SetActorWeaponAnimation", Script::SetActorWeaponAnimation},
			{"SetActorAlerted", Script::SetActorAlerted},
			{"SetActorSneaking", Script::SetActorSneaking},
			{"FireWeapon", Script::FireWeapon},
			{"KillActor", Script::KillActor},
			{"SetActorBaseRace", Script::SetActorBaseRace},
			{"AgeActorBaseRace", Script::AgeActorBaseRace},
			{"SetActorBaseSex", Script::SetActorBaseSex},
			{"SetPlayerRespawnTime", Script::SetPlayerRespawnTime},
			{"SetPlayerSpawnCell", Script::SetPlayerSpawnCell},
			{"SetPlayerConsoleEnabled", Script::SetPlayerConsoleEnabled},
			{"AttachWindow", Script::AttachWindow},
			{"DetachWindow", Script::DetachWindow},
			{"ForceWindowMode", Script::ForceWindowMode},

			{"GetParentWindow", Script::GetParentWindow},
			{"GetWindowRoot", Script::GetWindowRoot},
			{"GetWindowChildCount", Script::GetWindowChildCount},
			{"GetWindowChildList", Script::GetWindowChildList},
			{"GetWindowPos", Script::GetWindowPos},
			{"GetWindowSize", Script::GetWindowSize},
			{"GetWindowVisible", Script::GetWindowVisible},
			{"GetWindowLocked", Script::GetWindowLocked},
			{"GetWindowText", Script::GetWindowText},
			{"GetEditMaxLength", Script::GetEditMaxLength},
			{"GetEditValidation", Script::GetEditValidation},

			{"CreateWindow", Script::CreateWindow},
			{"DestroyWindow", Script::DestroyWindow},
			{"AddChildWindow", Script::AddChildWindow},
			{"RemoveChildWindow", Script::RemoveChildWindow},
			{"SetWindowPos", Script::SetWindowPos},
			{"SetWindowSize", Script::SetWindowSize},
			{"SetWindowVisible", Script::SetWindowVisible},
			{"SetWindowLocked", Script::SetWindowLocked},
			{"SetWindowText", Script::SetWindowText},
			{"CreateButton", Script::CreateButton},
			{"CreateText", Script::CreateText},
			{"CreateEdit", Script::CreateEdit},
			{"SetEditMaxLength", Script::SetEditMaxLength},
			{"SetEditValidation", Script::SetEditValidation},
		};

		static constexpr ScriptCallbackData callbacks[] {
			{"OnSpawn", Function<void, RakNet::NetworkID>()},
			{"OnActivate", Function<void, RakNet::NetworkID, RakNet::NetworkID>()},
			{"OnCellChange", Function<void, RakNet::NetworkID, unsigned int>()},
			{"OnLockChange", Function<void, RakNet::NetworkID, RakNet::NetworkID, unsigned int>()},
			{"OnContainerItemChange", Function<void, RakNet::NetworkID, unsigned int, signed int, double>()},
			{"OnActorValueChange", Function<void, RakNet::NetworkID, unsigned char, double>()},
			{"OnActorBaseValueChange", Function<void, RakNet::NetworkID, unsigned char, double>()},
			{"OnActorAlert", Function<void, RakNet::NetworkID, bool>()},
			{"OnActorSneak", Function<void, RakNet::NetworkID, bool>()},
			{"OnActorDeath", Function<void, RakNet::NetworkID, RakNet::NetworkID, unsigned short, signed char>()},
			{"OnActorEquipItem", Function<void, RakNet::NetworkID, unsigned int, double>()},
			{"OnActorUnequipItem", Function<void, RakNet::NetworkID, unsigned int, double>()},
			{"OnActorPunch", Function<void, RakNet::NetworkID, bool>()},
			{"OnActorFireWeapon", Function<void, RakNet::NetworkID, unsigned int>()},
			{"OnPlayerDisconnect", Function<void, RakNet::NetworkID, Reason>()},
			{"OnPlayerRequestGame", Function<unsigned int, RakNet::NetworkID>()},
			{"OnPlayerChat", Function<bool, RakNet::NetworkID, char*>()},
			{"OnWindowMode", Function<void, RakNet::NetworkID, bool>()},
			{"OnWindowClick", Function<void, RakNet::NetworkID, RakNet::NetworkID>()},
			{"OnWindowTextChange", Function<void, RakNet::NetworkID, RakNet::NetworkID, const char*>()},
			{"OnClientAuthenticate", Function<bool, const char*, const char*>()},
			{"OnGameYearChange", Function<void, unsigned int>()},
			{"OnGameMonthChange", Function<void, unsigned int>()},
			{"OnGameDayChange", Function<void, unsigned int>()},
			{"OnGameHourChange", Function<void, unsigned int>()},
			{"OnServerInit", Function<void>()},
			{"OnServerExit", Function<void>()},
		};

		static constexpr ScriptCallbackData const& CBD(const unsigned int I, const unsigned int N = 0) {
			return callbacks[N].index == I ? callbacks[N] : CBD(I, N + 1);
		}

		template<unsigned int I>
		using CBR = typename CharType<CBD(I).callback.ret>::type;

		template<size_t N>
		static constexpr unsigned int CBI(const char(&str)[N]) {
			return Utils::hash(str);
		}

		template<unsigned int I, bool B = false, typename... Args>
		static unsigned int Call(CBR<I>& result, Args&&... args) {
			constexpr ScriptCallbackData const& data = CBD(I);
			static_assert(data.callback.matches(TypeString<typename std::remove_reference<Args>::type...>::value), "Wrong number or types of arguments");

			unsigned int count = 0;

			for (auto& script : scripts)
			{
				if (!script->callbacks_.count(I))
					script->callbacks_.emplace(I, script->GetScript<FunctionEllipsis<void>>(data.name));

				auto callback = script->callbacks_[I];

				if (!callback)
					continue;

				if (script->cpp_script)
					result = reinterpret_cast<FunctionEllipsis<CBR<I>>>(callback)(std::forward<Args>(args)...);
				else
					result = static_cast<CBR<I>>(PAWN::Call(script->amx, data.name, data.callback.types, B, std::forward<Args>(args)...));

				++count;
			}

			return count;
		}

		template<unsigned int I, bool B = false, typename... Args>
		static unsigned int Call(Args&&... args) {
			constexpr ScriptCallbackData const& data = CBD(I);
			static_assert(data.callback.matches(TypeString<typename std::remove_reference<Args>::type...>::value), "Wrong number or types of arguments");

			unsigned int count = 0;

			for (auto& script : scripts)
			{
				if (!script->callbacks_.count(I))
					script->callbacks_.emplace(I, script->GetScript<FunctionEllipsis<void>>(data.name));

				auto callback = script->callbacks_[I];

				if (!callback)
					continue;

				if (script->cpp_script)
					reinterpret_cast<FunctionEllipsis<CBR<I>>>(callback)(std::forward<Args>(args)...);
				else
					PAWN::Call(script->amx, data.name, data.callback.types, B, std::forward<Args>(args)...);

				++count;
			}

			return count;
		}
};

#endif
