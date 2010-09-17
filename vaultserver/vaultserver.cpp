#include <windows.h>
#include <stdio.h>
#include <time.h>

#include "vaultserver.h"
#include "Dedicated.h"
#include "Script.h"
#include "Utils.h"

DWORD WINAPI InputThread(LPVOID data)
{
      char input[64];

      do
      {
          fgets(input, 64, stdin);
          if (strlen(input) > 0 && input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0';

      } while (strcmp(input, "exit") != 0);

      Dedicated::TerminateThread();

      return ((DWORD) data);
}

int main(int argc, char* argv[])
{
    printf("Vault-Tec Dedicated Server version %s \n----------------------------------------------------------\n", DEDICATED_VERSION);

    AMX* vaultscript = NULL;

    if (argc > 1)
    {
        vaultscript = new AMX();

        cell ret = 0;
        int err = 0;

        err = Script::LoadProgram(vaultscript, argv[1], NULL);
        if (err != AMX_ERR_NONE)
            Script::ErrorExit(vaultscript, err);

        Script::CoreInit(vaultscript);
        Script::ConsoleInit(vaultscript);
        Script::FloatInit(vaultscript);
        Script::StringInit(vaultscript);
        Script::FileInit(vaultscript);
        err = Script::TimeInit(vaultscript);
        if (err != AMX_ERR_NONE)
            Script::ErrorExit(vaultscript, err);

        err = Script::Exec(vaultscript, &ret, AMX_EXEC_MAIN);
        if (err != AMX_ERR_NONE)
            Script::ErrorExit(vaultscript, err);
    }

    Utils::timestamp();
    printf("Initalizing RakNet...\n");

    HANDLE hDedicatedThread = Dedicated::InitalizeServer(vaultscript);
    HANDLE hInputThread;
    DWORD InputID;

    hInputThread = CreateThread(NULL, 0, InputThread, (LPVOID) 0, 0, &InputID);

    HANDLE threads[2];
    threads[0] = hDedicatedThread;
    threads[1] = hInputThread;

    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    CloseHandle(hDedicatedThread);
    CloseHandle(hInputThread);

    if (vaultscript != NULL)
    {
        Script::FreeProgram(vaultscript);
        delete vaultscript;
    }

    return 0;
}
