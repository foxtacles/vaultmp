#ifndef DEDICATED_H
#define DEDICATED_H

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

              static ServerEntry* self;

              struct pPlayerUpdate;

              static DWORD WINAPI DedicatedThread(LPVOID data);
              static bool thread;

      public:
              static HANDLE InitializeServer(int port, int connections, AMX* amx, char* announce, bool query);
              static void SetServerEntry(ServerEntry* self);
              static void TerminateThread();

              static void SetServerName(string name);
              static void SetServerMap(string map);
              static void SetServerRule(string rule, string value);

              // static void SetServerConnections(int connections);

};

#endif
