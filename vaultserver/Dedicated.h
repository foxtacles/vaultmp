#include <windows.h>
#include <stdio.h>
#include <time.h>

#include "../RakNet/RakPeerInterface.h"
#include "../RakNet/MessageIdentifiers.h"
#include "../RakNet/BitStream.h"
#include "../RakNet/RakString.h"
#include "../RakNet/RakSleep.h"
#include "../RakNet/GetTime.h"

#include "ServerEntry.h"
#include "Script.h"
#include "Utils.h"

#define RAKNET_PORT           1770
#define RAKNET_CONNECTIONS    32
#define RAKNET_MASTER_ADDRESS "127.0.0.1"
#define RAKNET_MASTER_PORT    1660
#define RAKNET_MASTER_RATE    8000

#define ANNOUNCE // if defined, one connection slot will be reserved for announce
#define QUERY

using namespace RakNet;
using namespace std;

class Dedicated {

      private:
              static RakPeerInterface* peer;
              static SocketDescriptor* sockdescr;
              static AMX* script;

              #ifdef ANNOUNCE
              static void Announce(bool announce);
              static TimeMS announcetime;
              static SystemAddress master;
              #endif

              static ServerEntry self;

              static DWORD WINAPI DedicatedThread(LPVOID data);
              static bool thread;

      public:
              static HANDLE InitalizeServer(AMX* amx);
              static void TerminateThread();

};
