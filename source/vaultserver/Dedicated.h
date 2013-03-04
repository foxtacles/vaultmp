#ifndef DEDICATED_H
#define DEDICATED_H

#ifdef __WIN32__
#include <winsock2.h>
#include <io.h>
#endif
#include <thread>
#include <chrono>
#include <cstdio>

#include "RakNet.h"

#include "vaultmp.h"
#include "ServerEntry.h"
#include "Actor.h"
#include "Player.h"
#include "Data.h"
#include "Utils.h"
#include "VaultException.h"
#include "API.h"
#include "Client.h"
#include "NetworkServer.h"
#include "time/time64.h"
#include "vaultserver.h"

#define RAKNET_STANDARD_PORT            1770
#define RAKNET_STANDARD_CONNECTIONS     4
#define RAKNET_MASTER_RATE              2000
#define RAKNET_MASTER_STANDARD_PORT     1660

typedef std::vector<std::pair<std::string, unsigned int>> ModList;

/**
 * \brief The main class of the dedicated server
 */

class Dedicated
{
		friend class NetworkServer;
		friend class Server;
		friend class FileProgress;

	private:
		static RakNet::RakPeerInterface* peer;

		static unsigned int port;
		static const char* host;
		static unsigned int fileslots;
		static unsigned int connections;
		static const char* announce;
		static bool query;
		static bool fileserve;

		static void Announce(bool announce);
		static void Query(RakNet::Packet* packet);
		static RakNet::TimeMS announcetime;
		static RakNet::SystemAddress master;
		static ServerEntry* self;
		static unsigned int cell;
		static ModList modfiles;

		static void DedicatedThread();
		static void FileThread();

		static bool thread;

#ifdef VAULTMP_DEBUG
		static DebugInput<Dedicated> debug;
#endif

	public:
		/**
		 * \brief Initializes the dedicated server
		 *
		 * port - the port to run the server on
		 * connections - the maximum amount of player connections
		 * host - the IP address to listen on
		 * announce - whether the server should announce himself to a MasterServer, can be nullptr
		 * query - enable / disable direct query
		 * fileserver - enable / disable file downloading from this server
		 * fileslots - the maximum amount of file downloading connections
		 */
		static std::thread InitializeServer(unsigned int port, const char* host, unsigned int connections, const char* announce, bool query, bool fileserve, unsigned int fileslots);
		/**
		 * \brief Sets the ServerEntry of the dedicated server
		 *
		 * A ServerEntry contains information about the game, the players and a set of rules / values
		 */
		static void SetServerEntry(ServerEntry* self);
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
		static void SetServerName(const char* name);
		/**
		 * \brief Sets the server map
		 */
		static void SetServerMap(const char* map);
		/**
		 * \brief Defines a server rule
		 */
		static void SetServerRule(const char* rule, const char* value);
		/**
		 * \brief Sets the default spawn cell for players of the dedicated server
		 */
		static void SetSpawnCell(unsigned int cell);
		/**
		 * \brief Returns the game code of the dedicated server
		 */
		static unsigned char GetGameCode();
		/**
		 * \brief Returns the current number of player connections
		 */
		static unsigned int GetCurrentPlayers();
		/**
		 * \brief Returns the maximum number of player connections
		 */
		static unsigned int GetMaximumPlayers();

		//static void SetServerConnections(int connections);

};

#endif
