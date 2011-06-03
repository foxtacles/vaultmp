#include <windows.h>
#include <stdio.h>

#include "vaultserver.h"
#include "Dedicated.h"
#include "Script.h"
#include "Functions.h"
#include "Utils.h"

DWORD WINAPI InputThread(LPVOID data)
{
    char input[64];

    do
    {
        fgets(input, sizeof(input), stdin);
        if (strlen(input) > 0 && input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0';

        /* char* command = strtok(input, " ");
        char* param = strtok(NULL, " ");

        if (param != NULL)
        {
            if (strcmp(command, "connections") == 0)
                Dedicated::SetServerConnections(atoi(param));
        } */

    } while (strcmp(input, "exit") != 0);

    Dedicated::TerminateThread();

    return ((DWORD) data);
}

int main(int argc, char* argv[])
{
    printf("Vault-Tec Dedicated Server version %s \n----------------------------------------------------------\n", DEDICATED_VERSION);

    AMX* vaultscript = NULL;

    bool query = false;
    int announce = 0;
    int script = 0;
    int port = 0;
    int connections = 0;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "-query") == 0)
            query = true;
        else if (i + 1 < argc)
        {
            if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "-announce") == 0)
                announce = i + 1;
            else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "-script") == 0)
                script = i + 1;
            else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "-port") == 0)
                port = atoi(argv[i + 1]);
            else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "-connections") == 0)
                connections = atoi(argv[i + 1]);
        }
    }

    if (script != 0)
    {
        vaultscript = new AMX();

        cell ret = 0;
        int err = 0;

        err = Script::LoadProgram(vaultscript, argv[script], NULL);
        if (err != AMX_ERR_NONE)
            Script::ErrorExit(vaultscript, err);

        Script::CoreInit(vaultscript);
        Script::ConsoleInit(vaultscript);
        Script::FloatInit(vaultscript);
        Script::StringInit(vaultscript);
        Script::FileInit(vaultscript);
        Script::TimeInit(vaultscript);

        err = Functions::RegisterVaultmpFunctions(vaultscript);
        if (err != AMX_ERR_NONE)
            Script::ErrorExit(vaultscript, err);

        err = Script::Exec(vaultscript, &ret, AMX_EXEC_MAIN);
        if (err != AMX_ERR_NONE)
            Script::ErrorExit(vaultscript, err);
    }

    Utils::timestamp();
    printf("Initalizing RakNet...\n");

    HANDLE hDedicatedThread = Dedicated::InitalizeServer(port, connections, vaultscript, announce ? argv[announce] : 0, query);
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
