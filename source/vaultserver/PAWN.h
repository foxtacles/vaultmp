#ifndef PAWN_H
#define PAWN_H

#ifdef __WIN32__
#include <winsock2.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <map>

#define HAVE_STDINT_H
#include "amx/amx.h"
#include "amx/amxaux.h"

#include "boost/any.hpp"

#include "../Utils.h"
#include "../VaultException.h"
#include "../vaultmp.h"

/**
 * \brief Contains the PAWN scripting function wrappers
 */

class PAWN
{
	private:

		PAWN();

		static AMX_NATIVE_INFO vaultmp_functions[97];

		static cell vaultmp_timestamp(AMX* amx, const cell* params);
		static cell vaultmp_CreateTimer(AMX* amx, const cell* params);
		static cell vaultmp_CreateTimerEx(AMX* amx, const cell* params);
		static cell vaultmp_KillTimer(AMX* amx, const cell* params);
		static cell vaultmp_MakePublic(AMX* amx, const cell* params);
		static cell vaultmp_CallPublic(AMX* amx, const cell* params);

		static cell vaultmp_SetServerName(AMX* amx, const cell* params);
		static cell vaultmp_SetServerMap(AMX* amx, const cell* params);
		static cell vaultmp_SetServerRule(AMX* amx, const cell* params);
		static cell vaultmp_GetGameCode(AMX* amx, const cell* params);
		static cell vaultmp_GetMaximumPlayers(AMX* amx, const cell* params);
		static cell vaultmp_GetCurrentPlayers(AMX* amx, const cell* params);

		static cell vaultmp_ValueToString(AMX* amx, const cell* params);
		static cell vaultmp_AxisToString(AMX* amx, const cell* params);
		static cell vaultmp_AnimToString(AMX* amx, const cell* params);
		static cell vaultmp_BaseToString(AMX* amx, const cell* params);

		static cell vaultmp_UIMessage(AMX* amx, const cell* params);
		static cell vaultmp_ChatMessage(AMX* amx, const cell* params);
		static cell vaultmp_SetRespawn(AMX* amx, const cell* params);
		static cell vaultmp_SetSpawnCell(AMX* amx, const cell* params);
		static cell vaultmp_SetGameWeather(AMX* amx, const cell* params);
		static cell vaultmp_SetGameTime(AMX* amx, const cell* params);
		static cell vaultmp_SetGameYear(AMX* amx, const cell* params);
		static cell vaultmp_SetGameMonth(AMX* amx, const cell* params);
		static cell vaultmp_SetGameDay(AMX* amx, const cell* params);
		static cell vaultmp_SetGameHour(AMX* amx, const cell* params);
		static cell vaultmp_SetTimeScale(AMX* amx, const cell* params);
		static cell vaultmp_IsValid(AMX* amx, const cell* params);
		static cell vaultmp_IsObject(AMX* amx, const cell* params);
		static cell vaultmp_IsItem(AMX* amx, const cell* params);
		static cell vaultmp_IsContainer(AMX* amx, const cell* params);
		static cell vaultmp_IsActor(AMX* amx, const cell* params);
		static cell vaultmp_IsPlayer(AMX* amx, const cell* params);
		static cell vaultmp_IsCell(AMX* amx, const cell* params);
		static cell vaultmp_IsInterior(AMX* amx, const cell* params);
		static cell vaultmp_GetType(AMX* amx, const cell* params);
		static cell vaultmp_GetConnection(AMX* amx, const cell* params);
		static cell vaultmp_GetCount(AMX* amx, const cell* params);
		static cell vaultmp_GetList(AMX* amx, const cell* params);
		static cell vaultmp_GetGameWeather(AMX* amx, const cell* params);
		static cell vaultmp_GetGameTime(AMX* amx, const cell* params);
		static cell vaultmp_GetGameYear(AMX* amx, const cell* params);
		static cell vaultmp_GetGameMonth(AMX* amx, const cell* params);
		static cell vaultmp_GetGameDay(AMX* amx, const cell* params);
		static cell vaultmp_GetGameHour(AMX* amx, const cell* params);
		static cell vaultmp_GetTimeScale(AMX* amx, const cell* params);

		static cell vaultmp_GetReference(AMX* amx, const cell* params);
		static cell vaultmp_GetBase(AMX* amx, const cell* params);
		static cell vaultmp_GetName(AMX* amx, const cell* params);
		static cell vaultmp_GetPos(AMX* amx, const cell* params);
		static cell vaultmp_GetAngle(AMX* amx, const cell* params);
		static cell vaultmp_GetCell(AMX* amx, const cell* params);
		static cell vaultmp_IsNearPoint(AMX* amx, const cell* params);
		static cell vaultmp_GetItemContainer(AMX* amx, const cell* params);
		static cell vaultmp_GetItemCount(AMX* amx, const cell* params);
		static cell vaultmp_GetItemCondition(AMX* amx, const cell* params);
		static cell vaultmp_GetItemEquipped(AMX* amx, const cell* params);
		static cell vaultmp_GetItemSilent(AMX* amx, const cell* params);
		static cell vaultmp_GetItemStick(AMX* amx, const cell* params);
		static cell vaultmp_GetContainerItemCount(AMX* amx, const cell* params);
		static cell vaultmp_GetActorValue(AMX* amx, const cell* params);
		static cell vaultmp_GetActorBaseValue(AMX* amx, const cell* params);
		static cell vaultmp_GetActorIdleAnimation(AMX* amx, const cell* params);
		static cell vaultmp_GetActorMovingAnimation(AMX* amx, const cell* params);
		static cell vaultmp_GetActorWeaponAnimation(AMX* amx, const cell* params);
		static cell vaultmp_GetActorAlerted(AMX* amx, const cell* params);
		static cell vaultmp_GetActorSneaking(AMX* amx, const cell* params);
		static cell vaultmp_GetActorDead(AMX* amx, const cell* params);
		static cell vaultmp_GetActorBaseRace(AMX* amx, const cell* params);
		static cell vaultmp_GetActorBaseSex(AMX* amx, const cell* params);
		static cell vaultmp_IsActorJumping(AMX* amx, const cell* params);
		static cell vaultmp_GetPlayerRespawn(AMX* amx, const cell* params);
		static cell vaultmp_GetPlayerSpawnCell(AMX* amx, const cell* params);

		static cell vaultmp_DestroyObject(AMX* amx, const cell* params);
		static cell vaultmp_SetPos(AMX* amx, const cell* params);
		static cell vaultmp_SetAngle(AMX* amx, const cell* params);
		static cell vaultmp_SetCell(AMX* amx, const cell* params);
		static cell vaultmp_CreateItem(AMX* amx, const cell* params);
		static cell vaultmp_SetItemCount(AMX* amx, const cell* params);
		static cell vaultmp_SetItemCondition(AMX* amx, const cell* params);
		static cell vaultmp_CreateContainer(AMX* amx, const cell* params);
		static cell vaultmp_AddItem(AMX* amx, const cell* params);
		static cell vaultmp_RemoveItem(AMX* amx, const cell* params);
		static cell vaultmp_RemoveAllItems(AMX* amx, const cell* params);
		static cell vaultmp_CreateActor(AMX* amx, const cell* params);
		static cell vaultmp_SetActorValue(AMX* amx, const cell* params);
		static cell vaultmp_SetActorBaseValue(AMX* amx, const cell* params);
		static cell vaultmp_EquipItem(AMX* amx, const cell* params);
		static cell vaultmp_UnequipItem(AMX* amx, const cell* params);
		static cell vaultmp_PlayIdle(AMX* amx, const cell* params);
		static cell vaultmp_KillActor(AMX* amx, const cell* params);
		static cell vaultmp_SetActorBaseRace(AMX* amx, const cell* params);
		static cell vaultmp_AgeActorBaseRace(AMX* amx, const cell* params);
		static cell vaultmp_SetActorBaseSex(AMX* amx, const cell* params);
		static cell vaultmp_SetPlayerRespawn(AMX* amx, const cell* params);
		static cell vaultmp_SetPlayerSpawnCell(AMX* amx, const cell* params);

	public:
		static int LoadProgram(AMX* amx, char* filename, void* memblock);
		static int Register(AMX* amx, const AMX_NATIVE_INFO* list, int number);
		static int Exec(AMX* amx, cell* retval, int index);
		static int FreeProgram(AMX* amx);
		static bool IsCallbackPresent(AMX* amx, const char* name);
		static cell Call(AMX* amx, const char* name, const char* argl, int buf, ...);
		static cell Call(AMX* amx, const char* name, const char* argl, const std::vector<boost::any>& args);

		static int CoreInit(AMX* amx);
		static int ConsoleInit(AMX* amx);
		static int FloatInit(AMX* amx);
		static int TimeInit(AMX* amx);
		static int StringInit(AMX* amx);
		static int FileInit(AMX* amx);

		static int RegisterVaultmpFunctions(AMX* amx);

};

#endif
