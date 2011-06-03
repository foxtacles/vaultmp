#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <windows.h>
#include <string>

#define HAVE_STDINT_H
#include "amx/amx.h"
#include "amx/amxaux.h"

#include "Script.h"
#include "../Player.h"

class Functions {

      private:
              static AMX_NATIVE_INFO vaultmp_functions[3];

              static cell vaultmp_GetPlayerName(AMX* amx, const cell* params);
              static cell vaultmp_GetPlayerPos(AMX* amx, const cell* params);

      public:
              static int RegisterVaultmpFunctions(AMX* amx);

};

#endif
