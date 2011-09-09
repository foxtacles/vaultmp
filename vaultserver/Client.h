#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <map>
#include <stack>
#include <vector>

#include "../RakNet/RakPeerInterface.h"

class Player;

using namespace RakNet;
using namespace std;

/**
 * \brief The Client class contains information about a connected client
 */

class Client
{
private:
    static map<RakNetGUID, Client*> clients;
    static stack<unsigned int> clientID;

    RakNetGUID guid;
    unsigned int ID;
    NetworkID player;

    Client(const Client&);
    Client& operator=(const Client&);

public:
    Client(RakNetGUID guid, NetworkID player);
    ~Client();

    /**
     * \brief Sets the maximum amount of connected clients. This should be called in the initial startup procedure of the server
     */
    static void SetMaximumClients(unsigned int clients);
    /**
     * \brief Returns the amount of connected clients
     */
    static int GetClientCount();
    /**
     * \brief Given the RakNetGUID, returns the Client
     */
    static Client* GetClientFromGUID(RakNetGUID guid);
    /**
     * \brief Given the ID, returns the Client
     */
    static Client* GetClientFromID(unsigned int id);
    /**
     * \brief Returns a STL vector containing every RakNetGUID
     *
     * except (optional, Client*) - excludes a RakNetGUID from the result
     */
    static vector<RakNetGUID> GetNetworkList(Client* except = NULL);
    /**
     * \brief Returns a STL vector containing every RakNetGUID
     *
     * except (optional, RakNetGUID) - excludes a RakNetGUID from the result
     */
    static vector<RakNetGUID> GetNetworkList(RakNetGUID except = UNASSIGNED_RAKNET_GUID);

    /**
     * \brief Returns the RakNetGUID
     */
    RakNetGUID GetGUID();
    /**
     * \brief Returns the ID
     */
    unsigned int GetID();
    /**
     * \brief Returns the NetworkID of the coressponding Player instance
     */
    NetworkID GetPlayer();

};

#endif
