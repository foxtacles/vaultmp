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
#include "../API.h"
#include "../Utils.h"
#include "../vaultmp.h"
#include "../VaultException.h"

#ifdef __WIN32__
#define GetScript(a,b) (b = (decltype(b)) GetProcAddress((HINSTANCE)this->handle,a))
#define SetScript(a,b) *((decltype(b)*)(GetProcAddress((HINSTANCE)this->handle,a)?GetProcAddress((HINSTANCE)this->handle,a):throw VaultException("Script variable not found: %s", a)))=b;
#else
#define GetScript(a,b) (b = (decltype(b)) dlsym(this->handle,a))
#define SetScript(a,b) *((decltype(b)*)(dlsym(this->handle,a)?dlsym(this->handle,a):throw VaultException("Script function pointer not found: %s", a)))=b;
#endif

/**
 * \brief Maintains communication with a script
 *
 * A script can be either a C++ or PAWN script
 */

class Script
{
	private:
		Script(char* path);
		~Script();

		void* handle;
		bool cpp_script;

		static std::vector<Script*> scripts;

		static void GetArguments(std::vector<boost::any>& params, va_list args, const std::string& def);

		const char* vaultprefix;
		void (*fexec)();
		void (*fOnSpawn)(RakNet::NetworkID);
		void (*fOnCellChange)(RakNet::NetworkID, unsigned int);
		void (*fOnContainerItemChange)(RakNet::NetworkID, unsigned int, signed int, double);
		void (*fOnActorValueChange)(RakNet::NetworkID, unsigned char, double);
		void (*fOnActorBaseValueChange)(RakNet::NetworkID, unsigned char, double);
		void (*fOnActorAlert)(RakNet::NetworkID, bool);
		void (*fOnActorSneak)(RakNet::NetworkID, bool);
		void (*fOnActorDeath)(RakNet::NetworkID, unsigned short, signed char);
		void (*fOnActorEquipItem)(RakNet::NetworkID, unsigned int, double);
		void (*fOnActorUnequipItem)(RakNet::NetworkID, unsigned int, double);
		void (*fOnActorDropItem)(RakNet::NetworkID, unsigned int, unsigned int, double);
		void (*fOnActorPickupItem)(RakNet::NetworkID, unsigned int, unsigned int, double);
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

		Script(const Script&) = delete;
		Script& operator=(const Script&) = delete;

	public:
		static std::pair<std::chrono::system_clock::time_point, double> gameTime;
		static unsigned int gameWeather;

		static void LoadScripts(char* scripts, char* base);
		static void Run();
		static void UnloadScripts();

		static RakNet::NetworkID CreateTimer(ScriptFunc timer, unsigned int interval);
		static RakNet::NetworkID CreateTimerEx(ScriptFunc timer, unsigned int interval, const char* def, ...);
		static RakNet::NetworkID CreateTimerPAWN(ScriptFuncPAWN timer, AMX* amx, unsigned int interval);
		static RakNet::NetworkID CreateTimerPAWNEx(ScriptFuncPAWN timer, AMX* amx, unsigned int interval, const char* def, const std::vector<boost::any>& args);
		static void KillTimer(RakNet::NetworkID id = 0);
		static void MakePublic(ScriptFunc _public, const char* name, const char* def);
		static void MakePublicPAWN(ScriptFuncPAWN _public, AMX* amx, const char* name, const char* def);
		static unsigned long long CallPublic(const char* name, ...);
		static unsigned long long CallPublicPAWN(const char* name, const std::vector<boost::any>& args);

		static unsigned long long Timer_Respawn(RakNet::NetworkID id);
		static unsigned long long Timer_GameTime();

		static void OnSpawn(const FactoryObject& reference);
		static void OnCellChange(const FactoryObject& reference, unsigned int cell);
		static void OnContainerItemChange(const FactoryObject& reference, unsigned int baseID, signed int count, double condition);
		static void OnActorValueChange(const FactoryObject& reference, unsigned char index, bool base, double value);
		static void OnActorAlert(const FactoryObject& reference, bool alerted);
		static void OnActorSneak(const FactoryObject& reference, bool sneaking);
		static void OnActorDeath(const FactoryObject& reference, unsigned short limbs, signed char cause);
		static void OnActorEquipItem(const FactoryObject& reference, unsigned int baseID, double condition);
		static void OnActorUnequipItem(const FactoryObject& reference, unsigned int baseID, double condition);
		static void OnActorDropItem(const FactoryObject& reference, unsigned int baseID, unsigned int count, double condition);
		static void OnActorPickupItem(const FactoryObject& reference, unsigned int baseID, unsigned int count, double condition);
		static void OnActorPunch(const FactoryObject& reference, bool power);
		static void OnActorFireWeapon(const FactoryObject& reference, unsigned int weapon);
		static void OnPlayerDisconnect(const FactoryObject& reference, Reason reason);
		static unsigned int OnPlayerRequestGame(const FactoryObject& reference);
		static bool OnPlayerChat(const FactoryObject& reference, std::string& message);
		static bool OnClientAuthenticate(const std::string& name, const std::string& pwd);
		static void OnGameYearChange(unsigned int year);
		static void OnGameMonthChange(unsigned int month);
		static void OnGameDayChange(unsigned int day);
		static void OnGameHourChange(unsigned int hour);

		static const char* ValueToString(unsigned char index);
		static const char* AxisToString(unsigned char index);
		static const char* AnimToString(unsigned char index);
		static const char* BaseToString(unsigned int baseID);

		static bool UIMessage(RakNet::NetworkID id, const char* message);
		static bool ChatMessage(RakNet::NetworkID id, const char* message);
		static void SetRespawn(unsigned int respawn);
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
		static unsigned int GetConnection(RakNet::NetworkID id);
		static unsigned int GetList(unsigned char type, RakNet::NetworkID** data);
		static unsigned int GetGameWeather();
		static signed long long GetGameTime();
		static unsigned int GetGameYear();
		static unsigned int GetGameMonth();
		static unsigned int GetGameDay();
		static unsigned int GetGameHour();
		static double GetTimeScale();

		static unsigned int GetReference(RakNet::NetworkID id);
		static unsigned int GetBase(RakNet::NetworkID id);
		static const char* GetName(RakNet::NetworkID id);
		static void GetPos(RakNet::NetworkID id, double* X, double* Y, double* Z);
		static void GetAngle(RakNet::NetworkID id, double* X, double* Y, double* Z);
		static unsigned int GetCell(RakNet::NetworkID id);
		static bool IsNearPoint(RakNet::NetworkID id, double X, double Y, double Z, double R);
		static RakNet::NetworkID GetItemContainer(RakNet::NetworkID id);
		static unsigned int GetItemCount(RakNet::NetworkID id);
		static double GetItemCondition(RakNet::NetworkID id);
		static bool GetItemEquipped(RakNet::NetworkID id);
		static bool GetItemSilent(RakNet::NetworkID id);
		static bool GetItemStick(RakNet::NetworkID id);
		static unsigned int GetContainerItemCount(RakNet::NetworkID id, unsigned int baseID);
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
		static unsigned int GetPlayerRespawn(RakNet::NetworkID id);
		static unsigned int GetPlayerSpawnCell(RakNet::NetworkID id);

		static bool SetPos(RakNet::NetworkID id, double X, double Y, double Z);
		static bool SetCell(RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z);
		static bool AddItem(RakNet::NetworkID id, unsigned int baseID, unsigned int count, double condition, bool silent);
		static unsigned int RemoveItem(RakNet::NetworkID id, unsigned int baseID, unsigned int count, bool silent);
		static void RemoveAllItems(RakNet::NetworkID id);
		static void SetActorValue(RakNet::NetworkID id, unsigned char index, double value);
		static void SetActorBaseValue(RakNet::NetworkID id, unsigned char index, double value);
		static bool EquipItem(RakNet::NetworkID id, unsigned int baseID, bool silent, bool stick);
		static bool UnequipItem(RakNet::NetworkID id, unsigned int baseID, bool silent, bool stick);
		static bool PlayIdle(RakNet::NetworkID id, unsigned int idle);
		static void KillActor(RakNet::NetworkID id, unsigned short limbs, signed char cause);
		static bool SetActorBaseRace(RakNet::NetworkID id, unsigned int race);
		static bool AgeActorBaseRace(RakNet::NetworkID id, signed int age);
		static bool SetActorBaseSex(RakNet::NetworkID id, bool female);
		static void SetPlayerRespawn(RakNet::NetworkID id, unsigned int respawn);
		static void SetPlayerSpawnCell(RakNet::NetworkID id, unsigned int cell);

};

#endif
