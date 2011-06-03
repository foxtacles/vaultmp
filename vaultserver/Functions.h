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


      public:
              static int RegisterVaultmpFunctions(AMX* amx);
              static AMX_NATIVE_INFO vaultmp_functions[2];
              static cell vaultmp_GetPlayerName(AMX* amx, const cell* params);
};

#endif
