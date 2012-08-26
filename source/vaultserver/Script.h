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

using namespace std;
using namespace Values;

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

		static vector<Script*> scripts;

		static void GetArguments(vector<boost::any>& params, va_list args, string def);

		void* handle;
		bool cpp_script;

		const char* vaultprefix;
		void (*fexec)();
		void (*fOnSpawn)(NetworkID);
		void (*fOnCellChange)(NetworkID, unsigned int);
		void (*fOnContainerItemChange)(NetworkID, unsigned int, signed int, double);
		void (*fOnActorValueChange)(NetworkID, unsigned char, double);
		void (*fOnActorBaseValueChange)(NetworkID, unsigned char, double);
		void (*fOnActorAlert)(NetworkID, bool);
		void (*fOnActorSneak)(NetworkID, bool);
		void (*fOnActorDeath)(NetworkID, unsigned short, signed char);
		void (*fOnActorEquipItem)(NetworkID, unsigned int, double);
		void (*fOnActorUnequipItem)(NetworkID, unsigned int, double);
		void (*fOnActorDropItem)(NetworkID, unsigned int, unsigned int, double);
		void (*fOnActorPickupItem)(NetworkID, unsigned int, unsigned int, double);
		void (*fOnPlayerDisconnect)(NetworkID, Reason);
		unsigned int (*fOnPlayerRequestGame)(NetworkID);
		bool (*fOnPlayerChat)(NetworkID, char*);
		bool (*fOnClientAuthenticate)(string, string);

		Script(const Script&) = delete;
		Script& operator=(const Script&) = delete;

	public:
		static void LoadScripts(char* scripts, char* base);
		static void UnloadScripts();

		static NetworkID CreateTimer(ScriptFunc timer, unsigned int interval);
		static NetworkID CreateTimerEx(ScriptFunc timer, unsigned int interval, const char* def, ...);
		static NetworkID CreateTimerPAWN(ScriptFuncPAWN timer, AMX* amx, unsigned int interval);
		static NetworkID CreateTimerPAWNEx(ScriptFuncPAWN timer, AMX* amx, unsigned int interval, const char* def, const vector<boost::any>& args);
		static void KillTimer(NetworkID id = 0);
		static void MakePublic(ScriptFunc _public, const char* name, const char* def);
		static void MakePublicPAWN(ScriptFuncPAWN _public, AMX* amx, const char* name, const char* def);
		static unsigned long long CallPublic(const char* name, ...);
		static unsigned long long CallPublicPAWN(const char* name, const vector<boost::any>& args);

		static unsigned long long Timer_Respawn(NetworkID id);

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
		static void OnPlayerDisconnect(const FactoryObject& reference, Reason reason);
		static unsigned int OnPlayerRequestGame(const FactoryObject& reference);
		static bool OnPlayerChat(const FactoryObject& reference, string& message);
		static bool OnClientAuthenticate(string name, string pwd);

		static const char* ValueToString(unsigned char index);
		static const char* AxisToString(unsigned char index);
		static const char* AnimToString(unsigned char index);
		static const char* BaseToString(unsigned int baseID);

		static bool UIMessage(NetworkID id, const char* message);
		static bool ChatMessage(NetworkID id, const char* message);
		static void SetRespawn(unsigned int respawn);
		static void SetSpawnCell(unsigned int cell);
		static bool IsValid(NetworkID id);
		static bool IsObject(NetworkID id);
		static bool IsItem(NetworkID id);
		static bool IsContainer(NetworkID id);
		static bool IsActor(NetworkID id);
		static bool IsPlayer(NetworkID id);
		static bool IsInterior(unsigned int cell);
		static unsigned int GetConnection(NetworkID id);
		static unsigned int GetList(unsigned char type, NetworkID** data);

		static unsigned int GetReference(NetworkID id);
		static unsigned int GetBase(NetworkID id);
		static const char* GetName(NetworkID id);
		static void GetPos(NetworkID id, double* X, double* Y, double* Z);
		static void GetAngle(NetworkID id, double* X, double* Y, double* Z);
		static unsigned int GetCell(NetworkID id);
		static bool IsNearPoint(NetworkID id, double X, double Y, double Z, double R);
		static unsigned int GetItemCount(NetworkID id);
		static double GetItemCondition(NetworkID id);
		static bool GetItemEquipped(NetworkID id);
		static bool GetItemSilent(NetworkID id);
		static bool GetItemStick(NetworkID id);
		static unsigned int GetContainerItemCount(NetworkID id, unsigned int baseID);
		static double GetActorValue(NetworkID id, unsigned char index);
		static double GetActorBaseValue(NetworkID id, unsigned char index);
		static unsigned char GetActorMovingAnimation(NetworkID id);
		static bool GetActorAlerted(NetworkID id);
		static bool GetActorSneaking(NetworkID id);
		static bool GetActorDead(NetworkID id);
		static bool IsActorJumping(NetworkID id);
		static unsigned int GetPlayerRespawn(NetworkID id);
		static unsigned int GetPlayerSpawnCell(NetworkID id);

		static bool SetPos(NetworkID id, double X, double Y, double Z);
		static bool SetCell(NetworkID id, unsigned int cell, double X, double Y, double Z);
		static bool AddItem(NetworkID id, unsigned int baseID, unsigned int count, double condition, bool silent);
		static unsigned int RemoveItem(NetworkID id, unsigned int baseID, unsigned int count, bool silent);
		static void RemoveAllItems(NetworkID id);
		static void SetActorValue(NetworkID id, unsigned char index, double value);
		static void SetActorBaseValue(NetworkID id, unsigned char index, double value);
		static bool EquipItem(NetworkID id, unsigned int baseID, bool silent, bool stick);
		static bool UnequipItem(NetworkID id, unsigned int baseID, bool silent, bool stick);
		static void KillActor(NetworkID id, unsigned short limbs, signed char cause);
		static void SetPlayerRespawn(NetworkID id, unsigned int respawn);
		static void SetPlayerSpawnCell(NetworkID id, unsigned int cell);

};

#endif
