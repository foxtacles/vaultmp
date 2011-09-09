#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "../Network.h"
#include "Server.h"

/**
 * \brief Server network interface
 */

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

    /**
     * \brief Processes an event of a given type
     *
     * Returns a NetworkResponse to send to the client(s)
     */
    static NetworkResponse ProcessEvent(unsigned char id);
    /**
     * \brief Processes a packet from a client
     *
     * Returns a NetworkResponse to send to the client(s)
     */
    static NetworkResponse ProcessPacket(Packet* data);

};

#endif
