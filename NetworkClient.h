#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include "Network.h"

class NetworkClient : public Network
{
friend class Game;

private:

    NetworkClient();

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
