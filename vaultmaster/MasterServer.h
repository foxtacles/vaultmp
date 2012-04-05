#ifdef __WIN32__
#include <winsock2.h>
#endif

#include "../vaultmp.h"

#include <cstdio>
#include <ctime>
#include <map>
#include <thread>
#include <chrono>

#include "../RakNet/RakPeerInterface.h"
#include "../RakNet/MessageIdentifiers.h"
#include "../RakNet/BitStream.h"
#include "../RakNet/RakString.h"
#include "../RakNet/RakSleep.h"

#include "../ServerEntry.h"
#include "../Data.h"
#include "../Utils.h"
#include "vaultmaster.h"

#define RAKNET_PORT        1660
#define RAKNET_CONNECTIONS 128

using namespace RakNet;
using namespace Data;
using namespace std;

typedef map<SystemAddress, ServerEntry> ServerMap;

class MasterServer
{

	private:
		static RakPeerInterface* peer;
		static SocketDescriptor* sockdescr;

		static ServerMap serverList;
		static void RemoveServer( SystemAddress addr );

		static void MasterThread();
		static bool thread;

	public:
		static std::thread InitalizeRakNet();
		static void TerminateThread();

};
