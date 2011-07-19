#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <queue>
#include <list>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakSleep.h"

#include "Player.h"
#include "Inventory.h"
#include "Pipe.h"
#include "Data.h"
#include "vaultmp.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#define OPENCMD() while (cmdmutex); cmdmutex = true;
#define CLOSECMD() cmdmutex = false;
#define PUSHCMD(cmd) tmplist.push_back(cmd);

using namespace RakNet;
using namespace Data;
using namespace pipe;
using namespace std;

class Fallout3 {

      private:
              static bool NewVegas;
              static bool endThread;
              static bool wakeup;
              static HANDLE Fallout3pipethread;
              static HANDLE Fallout3gamethread;
              static HANDLE InitializeFallout3(bool NewVegas);

              static DWORD lookupProgramID(const char process[]);
              static DWORD WINAPI InjectedCode(LPVOID addr);
              static void InjectedEnd();
              static DWORD WINAPI Fallout3pipe(LPVOID data);
              static DWORD WINAPI Fallout3game(LPVOID data);

              static Player* self;
              static queue<Player*> refqueue;
              static list<fCommand*> cmdlist;
              static list<fCommand*> tmplist;
              static bool cmdmutex;

              static PipeClient* pipeServer;
              static PipeServer* pipeClient;

              #ifdef VAULTMP_DEBUG
              static Debug* debug;
              #endif

              struct INJECT;
      public:
              static void InitializeVaultMP(RakPeerInterface* peer, SystemAddress addr, string name, string pwd, bool NewVegas);

};
