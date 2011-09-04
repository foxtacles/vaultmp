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

    static void SetMaximumClients(unsigned int clients);
    static int GetClientCount();
    static Client* GetClientFromGUID(RakNetGUID guid);
    static Client* GetClientFromID(unsigned int id);
    static vector<RakNetGUID> GetNetworkList(Client* except = NULL);
    static vector<RakNetGUID> GetNetworkList(RakNetGUID except = UNASSIGNED_RAKNET_GUID);

    RakNetGUID GetGUID();
    unsigned int GetID();
    NetworkID GetPlayer();

};

#endif
