#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "../Network.h"
#include "Server.h"

class NetworkServer : public Network
{
friend class Dedicated;

private:

    NetworkServer();

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

public:

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    static NetworkResponse ProcessEvent(unsigned char id);
    static NetworkResponse ProcessPacket(Packet* data);

};

#endif
