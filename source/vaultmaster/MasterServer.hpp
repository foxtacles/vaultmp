#ifdef __WIN32__
#include <winsock2.h>
#endif

#include <cstdio>
#include <ctime>
#include <map>
#include <thread>
#include <chrono>

#include "../RakNet.hpp"

#include "../vaultmp.hpp"
#include "../ServerEntry.hpp"
#include "../Data.hpp"
#include "../Utils.hpp"
#include "vaultmaster.hpp"

#define RAKNET_PORT        1660
#define RAKNET_CONNECTIONS 128

using namespace RakNet;
using namespace std;

typedef map<SystemAddress, ServerEntry> ServerMap;

class MasterServer
{

	private:
		static RakPeerInterface* peer;
		static SocketDescriptor* sockdescr;

		static ServerMap serverList;
		static void RemoveServer(SystemAddress addr);

		static void MasterThread();
		static bool thread;

	public:
		static std::thread InitalizeRakNet();
		static void TerminateThread();

};
