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
#include "Data.h"
#include "vaultmp.h"

#define OPENCMD() while (cmdmutex); cmdmutex = true;
#define CLOSECMD() cmdmutex = false;
#define PUSHCMD(cmd) tmplist.push_back(cmd);
#define FLAG(flag) skipcmds[flag]->skipflag = true;
#define UNFLAG(flag) skipcmds[flag]->skipflag = false;
#define FLIPFLAG(flag) skipcmds[flag]->skipflag = !skipcmds[flag]->skipflag;
#define GETFLAG(flag) skipcmds[flag]->skipflag

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
              static fCommand* skipcmds[MAX_SKIP_FLAGS];
              static bool cmdmutex;
              static pPlayerUpdate localPlayerUpdate;

              static PipeClient* pipeServer;
              static PipeServer* pipeClient;

              struct INJECT;
      public:
              static void InitializeVaultMP(RakPeerInterface* peer, SystemAddress addr, string name, string pwd, bool NewVegas);

};
