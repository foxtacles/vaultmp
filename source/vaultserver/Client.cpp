#include "Client.h"

using namespace std;
using namespace RakNet;

map<RakNetGUID, Client*> Client::clients;
stack<unsigned int> Client::clientID;

Client::Client(RakNetGUID guid, NetworkID player) : guid(guid), ID(clientID.top()), player(player)
{
	// emplace
	clients.insert(make_pair(guid, this));
	clientID.pop();
}

Client::~Client()
{
	clients.erase(this->guid);
	clientID.push(this->ID);
}

void Client::SetMaximumClients(unsigned int clients)
{
	for (signed int i = clients - 1; i >= 0; --i)
		clientID.push(i);
}

unsigned int Client::GetClientCount()
{
	return clients.size();
}

Client* Client::GetClientFromGUID(RakNetGUID guid)
{
	map<RakNetGUID, Client*>::iterator it;
	it = clients.find(guid);

	if (it != clients.end())
		return it->second;

	return nullptr;
}

Client* Client::GetClientFromID(unsigned int ID)
{
	map<RakNetGUID, Client*>::iterator it;

	for (it = clients.begin(); it != clients.end(); ++it)
		if (it->second->GetID() == ID)
			return it->second;

	return nullptr;
}

Client* Client::GetClientFromPlayer(NetworkID id)
{
	map<RakNetGUID, Client*>::iterator it;

	for (it = clients.begin(); it != clients.end(); ++it)
		if (it->second->GetPlayer() == id)
			return it->second;

	return nullptr;
}

vector<RakNetGUID> Client::GetNetworkList(Client* except)
{
	vector<RakNetGUID> network;
	map<RakNetGUID, Client*>::iterator it;

	for (it = clients.begin(); it != clients.end(); ++it)
		if (it->second != except)
			network.emplace_back(it->first);

	return network;
}

vector<RakNetGUID> Client::GetNetworkList(RakNetGUID except)
{
	vector<RakNetGUID> network;
	map<RakNetGUID, Client*>::iterator it;

	for (it = clients.begin(); it != clients.end(); ++it)
		if (it->first != except)
			network.emplace_back(it->first);

	return network;
}

RakNetGUID Client::GetGUID()
{
	return guid;
}

unsigned int Client::GetID()
{
	return ID;
}

NetworkID Client::GetPlayer()
{
	return player;
}
