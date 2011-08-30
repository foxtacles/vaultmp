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

    static NetworkResponse GetPos(RakNetGUID guid, NetworkID id, unsigned char axis, double value);
    static NetworkResponse GetAngle(RakNetGUID guid, NetworkID id, unsigned char axis, double value);
    static NetworkResponse GetGameCell(RakNetGUID guid, NetworkID id, unsigned int cell);
    static NetworkResponse GetActorValue(RakNetGUID guid, NetworkID id, bool base, unsigned char index, double value);

public:

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif
};

#endif
