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

		static AMX_NATIVE_INFO vaultmp_functions[46];

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

		static cell vaultmp_ValueToString(AMX* amx, const cell* params);
		static cell vaultmp_AxisToString(AMX* amx, const cell* params);
		static cell vaultmp_AnimToString(AMX* amx, const cell* params);

		static cell vaultmp_UIMessage(AMX* amx, const cell* params);
		static cell vaultmp_SetRespawn(AMX* amx, const cell* params);
		static cell vaultmp_IsValid(AMX* amx, const cell* params);
		static cell vaultmp_IsObject(AMX* amx, const cell* params);
		static cell vaultmp_IsItem(AMX* amx, const cell* params);
		static cell vaultmp_IsContainer(AMX* amx, const cell* params);
		static cell vaultmp_IsActor(AMX* amx, const cell* params);
		static cell vaultmp_IsPlayer(AMX* amx, const cell* params);

		static cell vaultmp_GetType(AMX* amx, const cell* params);
		static cell vaultmp_GetReference(AMX* amx, const cell* params);
		static cell vaultmp_GetBase(AMX* amx, const cell* params);
		static cell vaultmp_GetName(AMX* amx, const cell* params);
		static cell vaultmp_GetPos(AMX* amx, const cell* params);
		static cell vaultmp_GetAngle(AMX* amx, const cell* params);
		static cell vaultmp_GetCell(AMX* amx, const cell* params);
		static cell vaultmp_GetContainerItemCount(AMX* amx, const cell* params);
		static cell vaultmp_GetActorValue(AMX* amx, const cell* params);
		static cell vaultmp_GetActorBaseValue(AMX* amx, const cell* params);
		static cell vaultmp_GetActorMovingAnimation(AMX* amx, const cell* params);
		static cell vaultmp_GetActorAlerted(AMX* amx, const cell* params);
		static cell vaultmp_GetActorSneaking(AMX* amx, const cell* params);
		static cell vaultmp_GetActorDead(AMX* amx, const cell* params);
		static cell vaultmp_IsActorJumping(AMX* amx, const cell* params);

		static cell vaultmp_AddItem(AMX* amx, const cell* params);
		static cell vaultmp_RemoveItem(AMX* amx, const cell* params);
		static cell vaultmp_RemoveAllItems(AMX* amx, const cell* params);
		static cell vaultmp_SetActorValue(AMX* amx, const cell* params);
		static cell vaultmp_SetActorBaseValue(AMX* amx, const cell* params);
		static cell vaultmp_EquipItem(AMX* amx, const cell* params);
		static cell vaultmp_UnequipItem(AMX* amx, const cell* params);
		static cell vaultmp_KillActor(AMX* amx, const cell* params);

		static cell vaultmp_SetPlayerRespawn(AMX* amx, const cell* params);

	public:
		static int LoadProgram(AMX* amx, char* filename, void* memblock);
		static int Register(AMX* amx, const AMX_NATIVE_INFO* list, int number);
		static int Exec(AMX* amx, cell* retval, int index);
		static int FreeProgram(AMX* amx);
		static bool IsCallbackPresent(AMX* amx, const char* name);
		static cell Call(AMX* amx, const char* name, const char* argl, int buf, ...);
		static cell Call(AMX* amx, const char* name, const char* argl, const vector<boost::any>& args);

		static int CoreInit(AMX* amx);
		static int ConsoleInit(AMX* amx);
		static int FloatInit(AMX* amx);
		static int TimeInit(AMX* amx);
		static int StringInit(AMX* amx);
		static int FileInit(AMX* amx);

		static int RegisterVaultmpFunctions(AMX* amx);

};

#endif
