#include <windows.h>
#include <stdio.h>
#include <time.h>

#include "vaultmaster.h"
#include "MasterServer.h"

void timestamp()
{
    time_t ltime;
    ltime = time(NULL);
    char t[32];
    sprintf(t, "[%s", asctime(localtime(&ltime)));
    char* newline = strchr(t, '\n');
    *newline = ']';
    strcat(t, " ");
    printf(t);
}

DWORD WINAPI InputThread(LPVOID data)
{
      char input[64];

      do
      {
          fgets(input, 64, stdin);
          if (strlen(input) > 0 && input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0';

      } while (strcmp(input, "exit") != 0);

      MasterServer::TerminateThread();

      return ((DWORD) data);
}

int main()
{
    printf("Vault-Tec MasterServer %s \n----------------------------------------------------------\n", MASTER_VERSION);

    timestamp();
    printf("Initializing RakNet...\n");

    HANDLE hMasterThread = MasterServer::InitalizeRakNet();
    HANDLE hInputThread;
    DWORD InputID;

    hInputThread = CreateThread(NULL, 0, InputThread, (LPVOID) 0, 0, &InputID);

    HANDLE threads[2];
    threads[0] = hMasterThread;
    threads[1] = hInputThread;

    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    CloseHandle(hMasterThread);
    CloseHandle(hInputThread);

    return 0;
}
