#include <windows.h>
#include <stdio.h>

#include "../RakNet/RakPeerInterface.h"
#include "../RakNet/MessageIdentifiers.h"
#include "../RakNet/BitStream.h"
#include "../RakNet/RakString.h"
#include "../RakNet/RakSleep.h"
#include "../RakNet/GetTime.h"

#include "../ServerEntry.h"
#include "../Client.h"
#include "../Player.h"
#include "Script.h"
#include "Utils.h"

#define RAKNET_STANDARD_PORT        1770
#define RAKNET_STANDARD_CONNECTIONS 32
#define RAKNET_MASTER_RATE          6000
#define RAKNET_MASTER_STANDARD_PORT 1660

using namespace RakNet;
using namespace std;

class Dedicated {

      private:
              static RakPeerInterface* peer;
              static SocketDescriptor* sockdescr;

              static int port;
              static int connections;
              static AMX* amx;
              static char* announce;
              static bool query;

              static void Announce(bool announce);
              static TimeMS announcetime;
              static SystemAddress master;

              static ServerEntry self;

              static DWORD WINAPI DedicatedThread(LPVOID data);
              static bool thread;

      public:
              static HANDLE InitalizeServer(int port, int connections, AMX* amx, char* announce, bool query);
              static void TerminateThread();

              // static void SetServerConnections(int connections);

};
