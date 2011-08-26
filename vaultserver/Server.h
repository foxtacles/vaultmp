#ifndef SERVER_H
#define SERVER_H

#include "Dedicated.h"
#include "Client.h"
#include "../PacketTypes.h"
#include "../Player.h"
#include "../Network.h"
#include "../VaultException.h"

class Server
{
friend class NetworkServer;
friend class Dedicated;

private:

    Server();

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    static NetworkResponse Authenticate(RakNetGUID guid, string name, string pwd);
    static NetworkResponse NewPlayer(RakNetGUID guid, NetworkID id, string name);
    static NetworkResponse Disconnect(RakNetGUID guid, unsigned char reason);

public:

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif
};

#endif
