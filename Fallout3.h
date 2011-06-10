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
#include "Pipe.h"

#define FALLOUT3_TICKS 25
#define OPENCMD() while (cmdmutex); cmdmutex = true;
#define CLOSECMD() cmdmutex = false;
#define PUSHCMD(cmd) cmdlist.push_back(cmd);

#define FALLOUT3_PLAYER_BASEID  00000007
#define FALLOUTNV_PLAYER_BASEID 00000007

using namespace RakNet;
using namespace pipe;
using namespace std;

class Fallout3 {

      private:
              static bool NewVegas;
              static bool endThread;
              static bool wakeup;
              static HANDLE Fallout3pipethread;
              static HANDLE Fallout3gamethread;
              static HANDLE InitalizeFallout3();

              static DWORD lookupProgramID(const char process[]);
              static DWORD WINAPI InjectedCode(LPVOID addr);
              static void InjectedEnd();
              static DWORD WINAPI Fallout3pipe(LPVOID data);
              static DWORD WINAPI Fallout3game(LPVOID data);

              struct pPlayerUpdate;
              struct fCommand;

              static Player* self;
              static queue<Player*> refqueue;
              static list<fCommand*> cmdlist;
              static bool cmdmutex;
              static pPlayerUpdate localPlayerUpdate;

              static PipeClient* pipeServer;
              static PipeServer* pipeClient;

              struct INJECT;
      public:
              static void InitalizeVaultMP(RakPeerInterface* peer, SystemAddress addr, string name, string pwd);

};
