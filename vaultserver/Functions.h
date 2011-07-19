#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <windows.h>
#include <string>

#define HAVE_STDINT_H
#include "amx/amx.h"
#include "amx/amxaux.h"

#include "Utils.h"
#include "Script.h"
#include "Dedicated.h"
#include "../Player.h"

class Functions {

      private:
              static AMX_NATIVE_INFO vaultmp_functions[16];

              static cell vaultmp_GetPlayerName(AMX* amx, const cell* params);
              static cell vaultmp_GetPlayerPos(AMX* amx, const cell* params);
              static cell vaultmp_GetPlayerZAngle(AMX* amx, const cell* params);
              static cell vaultmp_GetPlayerHealth(AMX* amx, const cell* params);
              static cell vaultmp_GetPlayerBaseHealth(AMX* amx, const cell* params);
              static cell vaultmp_GetPlayerCondition(AMX* amx, const cell* params);
              static cell vaultmp_IsPlayerDead(AMX* amx, const cell* params);
              static cell vaultmp_GetPlayerMoving(AMX* amx, const cell* params);
              static cell vaultmp_IsPlayerAlerted(AMX* amx, const cell* params);
              static cell vaultmp_GetPlayerCell(AMX* amx, const cell* params);
              static cell vaultmp_SetServerName(AMX* amx, const cell* params);
              static cell vaultmp_SetServerMap(AMX* amx, const cell* params);
              static cell vaultmp_SetServerRule(AMX* amx, const cell* params);
              static cell vaultmp_IsNewVegas(AMX* amx, const cell* params);
              static cell vaultmp_timestamp(AMX* amx, const cell* params);

      public:
              static int RegisterVaultmpFunctions(AMX* amx);

};

#endif
