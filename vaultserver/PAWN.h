#ifndef PAWN_H
#define PAWN_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <map>

#define HAVE_STDINT_H
#include "amx/amx.h"
#include "amx/amxaux.h"

#include "Dedicated.h"
#include "../Utils.h"
#include "../vaultmp.h"
#include "../VaultException.h"

class PAWN
{
private:

    PAWN();

    static AMX_NATIVE_INFO vaultmp_functions[6];

    static cell vaultmp_timestamp(AMX* amx, const cell* params);
    static cell vaultmp_SetServerName(AMX* amx, const cell* params);
    static cell vaultmp_SetServerMap(AMX* amx, const cell* params);
    static cell vaultmp_SetServerRule(AMX* amx, const cell* params);
    static cell vaultmp_GetGameCode(AMX* amx, const cell* params);

    /*static cell vaultmp_GetPlayerName(AMX* amx, const cell* params);
    static cell vaultmp_GetPlayerPos(AMX* amx, const cell* params);
    static cell vaultmp_GetPlayerZAngle(AMX* amx, const cell* params);
    static cell vaultmp_GetPlayerHealth(AMX* amx, const cell* params);
    static cell vaultmp_GetPlayerBaseHealth(AMX* amx, const cell* params);
    static cell vaultmp_GetPlayerCondition(AMX* amx, const cell* params);
    static cell vaultmp_IsPlayerDead(AMX* amx, const cell* params);
    static cell vaultmp_GetPlayerMoving(AMX* amx, const cell* params);
    static cell vaultmp_IsPlayerAlerted(AMX* amx, const cell* params);
    static cell vaultmp_GetPlayerCell(AMX* amx, const cell* params);*/




public:
    static int LoadProgram(AMX* amx, char* filename, void* memblock);
    static int Register(AMX* amx, const AMX_NATIVE_INFO* list, int number);
    static int Exec(AMX* amx, cell* retval, int index);
    static int Call(AMX* amx, const char* name, const char* argl, int buf, ...);
    static int FreeProgram(AMX* amx);

    static int CoreInit(AMX* amx);
    static int ConsoleInit(AMX* amx);
    static int FloatInit(AMX* amx);
    static int TimeInit(AMX* amx);
    static int StringInit(AMX* amx);
    static int FileInit(AMX* amx);

    static int RegisterVaultmpFunctions(AMX* amx);

};

#endif
