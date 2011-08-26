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
    Player* player;

public:
    Client(RakNetGUID guid, Player* player);
    ~Client();

    static void SetMaximumClients(unsigned int clients);
    static int GetClientCount();
    static Client* GetClientFromGUID(RakNetGUID guid);
    static Client* GetClientFromID(unsigned int id);
    static vector<RakNetGUID> GetNetworkList();

    RakNetGUID GetGUID();
    unsigned int GetID();
    Player* GetPlayer();

};

#endif
