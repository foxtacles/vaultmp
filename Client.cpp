#include "Client.h"

using namespace RakNet;
using namespace std;

map<RakNetGUID, Client*> Client::clients;
map<int, RakNetGUID> Client::clientGUIDs;
stack<int> Client::clientIDs;

Client::Client(RakNetGUID guid, string authname, string authpwd)
{
    clients.insert(pair<RakNetGUID, Client*>(guid, this));
    this->guid = guid;
    this->authname = authname;
    this->authpwd = authpwd;
    this->ID = clientIDs.top();
    clientGUIDs.insert(pair<int, RakNetGUID>(this->ID, guid));
    clientIDs.pop();
}

Client::~Client()
{
    clients.erase(this->guid);
    clientGUIDs.erase(this->ID);
    clientIDs.push(this->ID);
}

void Client::SetMaximumClients(int clients)
{
    for (int i = clients - 1; i >= 0; i--)
        clientIDs.push(i);
}

int Client::GetClientCount()
{
    return clients.size();
}

Client* Client::GetClientFromGUID(RakNetGUID guid)
{
    map<RakNetGUID, Client*>::iterator it;
    it = clients.find(guid);

    if (it != clients.end())
        return it->second;

    return NULL;
}

RakNetGUID Client::GetGUIDFromID(int ID)
{
    map<int, RakNetGUID>::iterator it;
    it = clientGUIDs.find(ID);

    if (it != clientGUIDs.end())
        return it->second;

    RakNetGUID err;
    return err;
}

RakNetGUID Client::GetRakNetGUID()
{
    return guid;
}

int Client::GetClientID()
{
    return ID;
}

string Client::GetAuthName()
{
    return authname;
}

string Client::GetAuthPwd()
{
    return authpwd;
}
