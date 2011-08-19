#ifdef __WIN32__
#include <windows.h>
#endif
#include <stdio.h>

#include "vaultserver.h"
#include "Dedicated.h"
#include "Script.h"
#include "Functions.h"
#include "../Utils.h"

#ifdef __WIN32__
DWORD WINAPI InputThread(LPVOID data)
#else
void* InputThread(void* data)
#endif
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

    }
    while (strcmp(input, "exit") != 0);

    Dedicated::TerminateThread();

#ifdef __WIN32__
    return ((DWORD) data);
#else
    return data;
#endif
}

int main(int argc, char* argv[])
{
#ifdef VAULTMP_DEBUG
#ifdef __WIN32__
    if (LoadLibrary("exchndl.dll") == NULL)
        return 0;
#else
    system("ulimit -c unlimited");
#endif
#endif

#ifdef __WIN32__
    printf("Vault-Tec dedicated server %s (Windows)\n----------------------------------------------------------\n", DEDICATED_VERSION);
#else
    printf("Vault-Tec dedicated server %s (Unix)\n----------------------------------------------------------\n", DEDICATED_VERSION);
#endif

    int game = FALLOUT3;
    bool query = false;
    int announce = 0;
    int script = 0;
    int port = 0;
    int connections = 0;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "-query") == 0)
            query = true;
        else if (strcmp(argv[i], "-f3") == 0 || strcmp(argv[i], "-fallout3") == 0)
            game = FALLOUT3;
        else if (strcmp(argv[i], "-nv") == 0 || strcmp(argv[i], "-newvegas") == 0)
            game = NEWVEGAS;
        else if (strcmp(argv[i], "-ob") == 0 || strcmp(argv[i], "-oblivion") == 0)
            game = OBLIVION;
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

    AMX* vaultscript = NULL;
    ServerEntry* self = new ServerEntry(game);
    Dedicated::SetServerEntry(self);

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
    printf("Initializing RakNet...\n");

#ifdef __WIN32__
    HANDLE hDedicatedThread = Dedicated::InitializeServer(port, connections, vaultscript, announce ? argv[announce] : 0, query);
    HANDLE hInputThread;
    hInputThread = CreateThread(NULL, 0, InputThread, (LPVOID) 0, 0, NULL);
    HANDLE threads[2];
    threads[0] = hDedicatedThread;
    threads[1] = hInputThread;
#else
    pthread_t threads[2];
    threads[0] = Dedicated::InitializeServer(port, connections, vaultscript, announce ? argv[announce] : 0, query);
    pthread_create(&threads[1], NULL, InputThread, NULL);
#endif

#ifdef __WIN32__
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    CloseHandle(hDedicatedThread);
    CloseHandle(hInputThread);
#else
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
#endif

    if (vaultscript != NULL)
    {
        Script::FreeProgram(vaultscript);
        delete vaultscript;
    }

    delete self;

    return 0;
}
