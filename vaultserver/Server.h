#ifndef SERVER_H
#define SERVER_H

#include "Dedicated.h"
#include "Client.h"
#include "../PacketTypes.h"
#include "../Player.h"
#include "../Network.h"
#include "../VaultException.h"

/**
 * \brief Server game code, communicating with loaded scripts
 *
 * Creates and returns packets
 */

class Server
{
friend class NetworkServer;
friend class Dedicated;

private:

    Server();

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

public:

    /**
     * \brief Authenticates a client
     */
    static NetworkResponse Authenticate(RakNetGUID guid, string name, string pwd);
    /**
     * \brief Creates a new Player
     */
    static NetworkResponse NewPlayer(RakNetGUID guid, NetworkID id, string name);
    /**
     * \brief Disconnects a client
     */
    static NetworkResponse Disconnect(RakNetGUID guid, unsigned char reason);

    /**
     * \brief Handles GetPos network packet
     */
    static NetworkResponse GetPos(RakNetGUID guid, NetworkID id, unsigned char axis, double value);
    /**
     * \brief Handles GetAngle network packet
     */
    static NetworkResponse GetAngle(RakNetGUID guid, NetworkID id, unsigned char axis, double value);
    /**
     * \brief Handles cell network packet
     */
    static NetworkResponse GetGameCell(RakNetGUID guid, NetworkID id, unsigned int cell);
    /**
     * \brief Handles actor value network packet
     */
    static NetworkResponse GetActorValue(RakNetGUID guid, NetworkID id, bool base, unsigned char index, double value);
    /**
     * \brief Handles actor state network packet
     */
    static NetworkResponse GetActorState(RakNetGUID guid, NetworkID id, unsigned char index, bool alerted);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif
};

#endif
