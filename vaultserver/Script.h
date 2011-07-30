#ifndef SCRIPT_H
#define SCRIPT_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define HAVE_STDINT_H
#include "amx/amx.h"
#include "amx/amxaux.h"

#include "../Utils.h"

class Script {

      public:
              static void ErrorExit(AMX* amx, int errorcode);
              static int LoadProgram(AMX* amx, char* filename, void* memblock);
              static int Register(AMX* amx, const AMX_NATIVE_INFO* list, int number);
              static int Exec(AMX* amx, cell* retval, int index);
              static int Call(AMX* amx, char name[], char argl[], void* args[], int buf = -1);
              static int FreeProgram(AMX* amx);

              static int CoreInit(AMX* amx);
              static int ConsoleInit(AMX* amx);
              static int FloatInit(AMX* amx);
              static int TimeInit(AMX* amx);
              static int StringInit(AMX* amx);
              static int FileInit(AMX* amx);

};

#endif
