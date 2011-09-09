#ifndef DEDICATED_H
#define DEDICATED_H

#ifdef __WIN32__
#include <windows.h>
#include <io.h>
#endif
#include <cstdio>

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
#include "Timer.h"
#include "vaultserver.h"

#define RAKNET_STANDARD_PORT            1770
#define RAKNET_STANDARD_CONNECTIONS     4
#define RAKNET_MASTER_RATE              2000
#define RAKNET_MASTER_STANDARD_PORT     1660

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

using namespace RakNet;
using namespace Data;
using namespace std;

typedef pair<string, unsigned int> Savegame;
typedef vector<pair<string, unsigned int> > ModList;

/**
 * \brief The main class of the dedicated server
 */

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
    static void Query(Packet* packet);
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

    /**
     * \brief Initializes the dedicated server
     *
     * port - the port to run the server on
     * connections - the maximum amount of player connections
     * announce - whether the server should announce himself to a MasterServer, can be NULL
     * query - enable / disable direct query
     * fileserver - enable / disable file downloading from this server
     * fileslots - the maximum amount of file downloading connections
     */
    #ifdef __WIN32__
    static HANDLE InitializeServer(int port, int connections, char* announce, bool query, bool fileserve, int fileslots);
    #else
    /**
     * \brief Initializes the dedicated server
     *
     * port - the port to run the server on
     * connections - the maximum amount of player connections
     * announce - whether the server should announce himself to a MasterServer, can be NULL
     * query - enable / disable direct query
     * fileserver - enable / disable file downloading from this server
     * fileslots - the maximum amount of file downloading connections
     */
    static pthread_t InitializeServer(int port, int connections, char* announce, bool query, bool fileserve, int fileslots);
    #endif
    /**
     * \brief Sets the ServerEntry of the dedicated server
     *
     * A ServerEntry contains information about the game, the players and a set of rules / values
     */
    static void SetServerEntry(ServerEntry* self);
    /**
     * \brief Sets the Savegame of the dedicated server
     *
     * A Savegame is of the form pair<string, unsigned int>
     * string is the relative path to the savegame
     * unsigned int is the CRC32 of the savegame
     */
    static void SetSavegame(Savegame savegame);
    /**
     * \brief Sets the ModList of the dedicated server
     *
     * A ModList is of the form vector<pair<string, unsigned int> >
     * string is the relative path to the modfile
     * unsigned int is the CRC32 of the modfile
     */
    static void SetModfiles(ModList modfiles);
    /**
     * \brief Terminates the dedicated server thread
     */
    static void TerminateThread();

    /**
     * \brief Sets the server name
     */
    static void SetServerName(string name);
    /**
     * \brief Sets the server map
     */
    static void SetServerMap(string map);
    /**
     * \brief Defines a server rule
     */
    static void SetServerRule(string rule, string value);
    /**
     * \brief Returns the game code of the dedicated server
     */
    static unsigned char GetGameCode();

    //static void SetServerConnections(int connections);

};

#endif
