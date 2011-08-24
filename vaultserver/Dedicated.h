#ifndef DEDICATED_H
#define DEDICATED_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <stdio.h>
#include <io.h>

#include "../RakNet/RakPeerInterface.h"
#include "../RakNet/MessageIdentifiers.h"
#include "../RakNet/IncrementalReadInterface.h"
#include "../RakNet/FileListTransfer.h"
#include "../RakNet/PacketizedTCP.h"
#include "../RakNet/BitStream.h"
#include "../RakNet/RakString.h"
#include "../RakNet/RakSleep.h"
#include "../RakNet/GetTime.h"

#include "../ServerEntry.h"
#include "../Actor.h"
#include "../Player.h"
#include "../Data.h"
#include "../Utils.h"
#include "../VaultException.h"
#include "Client.h"
#include "NetworkServer.h"
#include "Script.h"
#include "vaultserver.h"

#define RAKNET_STANDARD_PORT            1770
#define RAKNET_STANDARD_CONNECTIONS     4
#define RAKNET_MASTER_RATE              2000
#define RAKNET_MASTER_STANDARD_PORT     1660

using namespace RakNet;
using namespace Data;
using namespace std;

typedef pair<string, unsigned int> Savegame;
typedef vector<pair<string, unsigned int> > ModList;

class Dedicated
{
friend class NetworkServer;
friend class Server;
friend class FileProgress;

private:
    static RakPeerInterface* peer;
    static SocketDescriptor* sockdescr;

    static int port;
    static int fileslots;
    static int connections;
    static char* announce;
    static bool query;
    static bool fileserve;

    static void Announce(bool announce);
    static TimeMS announcetime;
    static SystemAddress master;
    static ServerEntry* self;
    static Savegame savegame;
    static ModList modfiles;

    #ifdef __WIN32__
    static DWORD WINAPI DedicatedThread(LPVOID data);
    static DWORD WINAPI FileThread(LPVOID data);
    #else
    static void* DedicatedThread(void* data);
    static void* FileThread(void* data);
    #endif
    static bool thread;

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

public:
    #ifdef __WIN32__
    static HANDLE InitializeServer(int port, int connections, char* announce, bool query, bool fileserve, int fileslots);
    #else
    static pthread_t InitializeServer(int port, int connections, char* announce, bool query, bool fileserve, int fileslots);
    #endif
    static void SetServerEntry(ServerEntry* self);
    static void SetSavegame(Savegame savegame);
    static void SetModfiles(ModList modfiles);
    static void TerminateThread();

    static void SetServerName(string name);
    static void SetServerMap(string map);
    static void SetServerRule(string rule, string value);
    static int GetGameCode();

    //static void SetServerConnections(int connections);

};

#endif
