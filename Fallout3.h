#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <shlwapi.h>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakSleep.h"

#include "Pipe.h"

using namespace RakNet;
using namespace pipe;
using namespace std;

class Fallout3 {

      private:
              static bool endThread;
              static bool wakeup;
              static HANDLE Fallout3thread;
              static HANDLE InitalizeFallout3();

              static DWORD lookupProgramID(const char process[]);
              static DWORD WINAPI InjectedCode(LPVOID addr);
              static void InjectedEnd();
              static DWORD WINAPI Fallout3pipe(LPVOID data);

              static PipeClient pipeClient;

              struct INJECT;
      public:
              static void InitalizeVaultMP(RakPeerInterface* peer, SystemAddress addr, string name, string pwd);

};
