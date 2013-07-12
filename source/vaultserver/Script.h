#ifndef SCRIPT_H
#define SCRIPT_H

#ifdef __WIN32__
#include <winsock2.h>
#else
#include <dlfcn.h>
#endif
#include <vector>
#include <string>

#include "boost/any.hpp"

#include "ScriptFunction.h"
#include "Public.h"
#include "PAWN.h"
#include "Dedicated.h"
#include "time/time64.h"
#include "Record.h"
#include "API.h"
#include "GameFactory.h"
#include "Utils.h"
#include "vaultmp.h"
#include "VaultException.h"

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
template<typename T> struct TypeChar<T, sizeof(uint8_t)> { enum { value = 'i' }; };
template<typename T> struct TypeChar<T, sizeof(uint16_t)> { enum { value = 'i' }; };
template<typename T> struct TypeChar<T, sizeof(uint32_t)> { enum { value = 'i' }; };
template<typename T> struct TypeChar<T, sizeof(uint64_t)> { enum { value = 'l' }; };
template<> struct TypeChar<double, sizeof(double)> { enum { value = 'f' }; };
template<> struct TypeChar<const char*, sizeof(const char*)> { enum { value = 's' }; };
template<> struct TypeChar<void, sizeof_void<void>::value> { enum { value = 'v' }; };

template<const char t> struct CharType { static_assert(!t, "Unsupported type in variadic type list"); };
template<> struct CharType<'p'> { typedef void* type; };
template<> struct CharType<'d'> { typedef double* type; };
template<> struct CharType<'n'> { typedef RakNet::NetworkID** type; };
template<> struct CharType<'i'> { typedef unsigned int type; };
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

class ScriptFunctionPointer
{
	template<typename R, typename... Types>
	using ScriptFunctionPointer_ = R(*)(Types...);

	public:
		ScriptFunctionPointer_<void> addr;
		const char* types;
		const char ret;
		const unsigned int numargs;

		template<typename R, typename... Types>
		constexpr ScriptFunctionPointer(ScriptFunctionPointer_<R, Types...> addr) : addr(reinterpret_cast<ScriptFunctionPointer_<void>>(addr)), types(TypeString<Types...>::value), ret(TypeChar<R, sizeof_void<R>::value>::value), numargs(sizeof(TypeString<Types...>::value) - 1) {}
};

struct ScriptFunctionData
{
	const char* name;
	ScriptFunctionPointer func;

	constexpr ScriptFunctionData(const char* name, ScriptFunctionPointer func) : name(name), func(func) {}
};

class Script
{
	private:
		Script(char* path);
		~Script();

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

		static void GetArguments(std::vector<boost::any>& params, va_list args, const std::string& def);

		void (*fexec)();
		void (*fOnSpawn)(RakNet::NetworkID);
		void (*fOnCellChange)(RakNet::NetworkID, unsigned int);
		void (*fOnLockChange)(RakNet::NetworkID, RakNet::NetworkID, unsigned int);
		void (*fOnContainerItemChange)(RakNet::NetworkID, unsigned int, signed int, double);
		void (*fOnActorValueChange)(RakNet::NetworkID, unsigned char, double);
		void (*fOnActorBaseValueChange)(RakNet::NetworkID, unsigned char, double);
		void (*fOnActorAlert)(RakNet::NetworkID, bool);
		void (*fOnActorSneak)(RakNet::NetworkID, bool);
		void (*fOnActorDeath)(RakNet::NetworkID, RakNet::NetworkID, unsigned short, signed char);
		void (*fOnActorEquipItem)(RakNet::NetworkID, unsigned int, double);
		void (*fOnActorUnequipItem)(RakNet::NetworkID, unsigned int, double);
		void (*fOnActorDropItem)(RakNet::NetworkID, unsigned int, unsigned int, double);
		void (*fOnActorPickupItem)(RakNet::NetworkID, unsigned int, unsigned int, double, unsigned int);
		void (*fOnActorPunch)(RakNet::NetworkID, bool);
		void (*fOnActorFireWeapon)(RakNet::NetworkID, unsigned int);
		void (*fOnPlayerDisconnect)(RakNet::NetworkID, Reason);
		unsigned int (*fOnPlayerRequestGame)(RakNet::NetworkID);
		bool (*fOnPlayerChat)(RakNet::NetworkID, char*);
		bool (*fOnClientAuthenticate)(const char*, const char*);
		void (*fOnGameYearChange)(unsigned int);
		void (*fOnGameMonthChange)(unsigned int);
		void (*fOnGameDayChange)(unsigned int);
		void (*fOnGameHourChange)(unsigned int);
		void (*fOnServerInit)();
		void (*fOnServerExit)();

		typedef std::vector<Script*> ScriptList;
		typedef std::unordered_map<RakNet::NetworkID, std::unique_ptr<ItemList>> ScriptItemLists;
		typedef std::unordered_map<unsigned int, std::vector<unsigned int>> DeletedObjects;
		typedef std::pair<std::chrono::system_clock::time_point, double> GameTime;
		typedef unsigned int GameWeather;

		static ScriptList scripts;
		static ScriptItemLists scriptIL;
		static DeletedObjects deletedStatic;
		static GameTime gameTime;
		static GameWeather gameWeather;

		Script(const Script&) = delete;
		Script& operator=(const Script&) = delete;

	public:
		static void LoadScripts(char* scripts, char* base);
		static void Initialize();
		static void Run();
		static void UnloadScripts();

		static RakNet::NetworkID CreateTimer(ScriptFunc timer, unsigned int interval);
		static RakNet::NetworkID CreateTimerEx(ScriptFunc timer, unsigned int interval, const char* def, ...);
		static RakNet::NetworkID CreateTimerPAWN(ScriptFuncPAWN timer, AMX* amx, unsigned int interval);
		static RakNet::NetworkID CreateTimerPAWNEx(ScriptFuncPAWN timer, AMX* amx, unsigned int interval, const char* def, const std::vector<boost::any>& args);
		static void SetupObject(FactoryObject<Object>& object, FactoryObject<Object>& reference, unsigned int cell, double X, double Y, double Z);
		static void SetupItem(FactoryObject<Item>& item, FactoryObject<Object>& reference, unsigned int cell, double X, double Y, double Z);
		static void SetupContainer(FactoryObject<Container>& container, FactoryObject<Object>& reference, unsigned int cell, double X, double Y, double Z);
		static void SetupActor(FactoryObject<Actor>& actor, FactoryObject<Object>& reference, unsigned int cell, double X, double Y, double Z);
		static void KillTimer(RakNet::NetworkID id = 0);
		static void MakePublic(ScriptFunc _public, const char* name, const char* def);
		static void MakePublicPAWN(ScriptFuncPAWN _public, AMX* amx, const char* name, const char* def);
		static unsigned long long CallPublic(const char* name, ...);
		static unsigned long long CallPublicPAWN(const char* name, const std::vector<boost::any>& args);
		static const DeletedObjects& GetDeletedStatic() { return deletedStatic; }

		static unsigned long long Timer_Respawn(RakNet::NetworkID id);
		static unsigned long long Timer_GameTime();

		static void OnSpawn(RakNet::NetworkID id);
		static void OnCellChange(RakNet::NetworkID id, unsigned int cell);
		static void OnLockChange(RakNet::NetworkID id, RakNet::NetworkID player, unsigned int lock);
		static void OnContainerItemChange(RakNet::NetworkID id, unsigned int baseID, signed int count, double condition);
		static void OnActorValueChange(RakNet::NetworkID id, unsigned char index, bool base, double value);
		static void OnActorAlert(RakNet::NetworkID id, bool alerted);
		static void OnActorSneak(RakNet::NetworkID id, bool sneaking);
		static void OnActorDeath(RakNet::NetworkID id, RakNet::NetworkID killer, unsigned short limbs, signed char cause);
		static void OnActorEquipItem(RakNet::NetworkID id, unsigned int baseID, double condition);
		static void OnActorUnequipItem(RakNet::NetworkID id, unsigned int baseID, double condition);
		static void OnActorDropItem(RakNet::NetworkID id, unsigned int baseID, unsigned int count, double condition);
		static void OnActorPickupItem(RakNet::NetworkID id, unsigned int baseID, unsigned int count, double condition, unsigned int owner);
		static void OnActorPunch(RakNet::NetworkID id, bool power);
		static void OnActorFireWeapon(RakNet::NetworkID id, unsigned int weapon);
		static void OnPlayerDisconnect(RakNet::NetworkID id, Reason reason);
		static unsigned int OnPlayerRequestGame(RakNet::NetworkID id);
		static bool OnPlayerChat(RakNet::NetworkID id, std::string& message);
		static bool OnClientAuthenticate(const std::string& name, const std::string& pwd);
		static void OnGameYearChange(unsigned int year);
		static void OnGameMonthChange(unsigned int month);
		static void OnGameDayChange(unsigned int day);
		static void OnGameHourChange(unsigned int hour);
		static void OnServerInit();
		static void OnServerExit();

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

		static RakNet::NetworkID CreateObject(unsigned int baseID, RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z);
		static bool DestroyObject(RakNet::NetworkID id);
		static bool SetPos(RakNet::NetworkID id, double X, double Y, double Z);
		static bool SetAngle(RakNet::NetworkID id, double X, double Y, double Z);
		static bool SetCell(RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z);
		static bool SetCell_(RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z, bool nosend);
		static bool SetLock(RakNet::NetworkID id, unsigned int lock);
		static bool SetOwner(RakNet::NetworkID id, unsigned int owner);
		static bool SetBaseName(RakNet::NetworkID id, const char* name);
		static RakNet::NetworkID CreateItem(unsigned int baseID, RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z);
		static bool SetItemCount(RakNet::NetworkID id, unsigned int count);
		static bool SetItemCondition(RakNet::NetworkID id, double condition);
		static RakNet::NetworkID CreateContainer(unsigned int baseID, RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z);
		static RakNet::NetworkID CreateItemList(RakNet::NetworkID source, unsigned int baseID);
		static bool AddItem(RakNet::NetworkID id, unsigned int baseID, unsigned int count, double condition, bool silent);
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
		static void KillActor(RakNet::NetworkID id, unsigned short limbs, signed char cause);
		static bool SetActorBaseRace(RakNet::NetworkID id, unsigned int race);
		static bool AgeActorBaseRace(RakNet::NetworkID id, signed int age);
		static bool SetActorBaseSex(RakNet::NetworkID id, bool female);
		static void SetPlayerRespawnTime(RakNet::NetworkID id, unsigned int respawn);
		static void SetPlayerSpawnCell(RakNet::NetworkID id, unsigned int cell);
		static void SetPlayerConsoleEnabled(RakNet::NetworkID id, bool enabled);

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

			{"CreateObject", Script::CreateObject},
			{"DestroyObject", Script::DestroyObject},
			{"SetPos", Script::SetPos},
			{"SetAngle", Script::SetAngle},
			{"SetCell", Script::SetCell},
			{"SetLock", Script::SetLock},
			{"SetOwner", Script::SetOwner},
			{"SetBaseName", Script::SetBaseName},
			{"CreateItem", Script::CreateItem},
			{"SetItemCount", Script::SetItemCount},
			{"SetItemCondition", Script::SetItemCondition},
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
		};
};

#endif
