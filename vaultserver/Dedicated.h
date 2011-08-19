#ifndef DEDICATED_H
#define DEDICATED_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <stdio.h>

#include "../RakNet/RakPeerInterface.h"
#include "../RakNet/MessageIdentifiers.h"
#include "../RakNet/BitStream.h"
#include "../RakNet/RakString.h"
#include "../RakNet/RakSleep.h"
#include "../RakNet/GetTime.h"

#include "../ServerEntry.h"
#include "../Client.h"
#include "../Actor.h"
#include "../Player.h"
#include "../Data.h"
#include "../Utils.h"
#include "../VaultException.h"
#include "../Network.h"
#include "../PacketTypes.h"
#include "Script.h"
#include "vaultserver.h"

#define RAKNET_STANDARD_PORT            1770
#define RAKNET_STANDARD_CONNECTIONS     32
#define RAKNET_MASTER_RATE              2000
#define RAKNET_MASTER_STANDARD_PORT     1660

using namespace RakNet;
using namespace Data;
using namespace std;

class Dedicated
{
friend class Network;

private:
    static RakPeerInterface* peer;
    static SocketDescriptor* sockdescr;

    static AMX* amx;
    static int port;
    static int connections;
    static char* announce;
    static bool query;

    static void Announce(bool announce);
    static TimeMS announcetime;
    static SystemAddress master;

    static ServerEntry* self;

    #ifdef __WIN32__
    static DWORD WINAPI DedicatedThread(LPVOID data);
    #else
    static void* DedicatedThread(void* data);
    #endif
    static bool thread;

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

public:
    #ifdef __WIN32__
    static HANDLE InitializeServer(int port, int connections, AMX* amx, char* announce, bool query);
    #else
    static pthread_t InitializeServer(int port, int connections, AMX* amx, char* announce, bool query);
    #endif
    static void SetServerEntry(ServerEntry* self);
    static void TerminateThread();

    static void SetServerName(string name);
    static void SetServerMap(string map);
    static void SetServerRule(string rule, string value);
    static int GetGame();

    //static void SetServerConnections(int connections);

};

#endif
