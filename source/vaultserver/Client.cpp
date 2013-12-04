#include "Client.hpp"

#include <algorithm>

using namespace std;
using namespace RakNet;

Guarded<> Client::cs;
map<RakNetGUID, Client*> Client::clients;
stack<unsigned int> Client::clientID;

Client::Client(RakNetGUID guid, NetworkID player) : guid(guid), player(player)
{
	cs.Operate([guid, this]() {
		ID = clientID.top();
		clients.emplace(guid, this);
		clientID.pop();
	});
}

Client::~Client()
{
	cs.Operate([this]() {
		clients.erase(this->guid);
		clientID.push(this->ID);
	});
}

void Client::SetMaximumClients(unsigned int clients)
{
	cs.Operate([clients]() {
		while (!clientID.empty())
			clientID.pop();

		for (signed int i = clients - 1; i >= 0; --i)
			clientID.push(i);
	});
}

unsigned int Client::GetClientCount()
{
	return cs.Operate([]() {
		return clients.size();
	});
}

Client* Client::GetClientFromGUID(RakNetGUID guid)
{
	return cs.Operate([guid]() -> Client* {
		auto it = clients.find(guid);

		if (it != clients.end())
			return it->second;

		return nullptr;
	});
}

Client* Client::GetClientFromID(unsigned int ID)
{
	return cs.Operate([ID]() -> Client* {
		for (auto it = clients.begin(); it != clients.end(); ++it)
			if (it->second->GetID() == ID)
				return it->second;

		return nullptr;
	});
}

Client* Client::GetClientFromPlayer(NetworkID id)
{
	return cs.Operate([id]() -> Client* {
		for (auto it = clients.begin(); it != clients.end(); ++it)
			if (it->second->GetPlayer() == id)
				return it->second;

		return nullptr;
	});
}

vector<RakNetGUID> Client::GetNetworkList(Client* except)
{
	return cs.Operate([except]() {
		vector<RakNetGUID> network;

		for (auto it = clients.begin(); it != clients.end(); ++it)
			if (it->second != except)
				network.emplace_back(it->first);

		return network;
	});
}

vector<RakNetGUID> Client::GetNetworkList(RakNetGUID except)
{
	return cs.Operate([except]() {
		vector<RakNetGUID> network;

		for (auto it = clients.begin(); it != clients.end(); ++it)
			if (it->first != except)
				network.emplace_back(it->first);

		return network;
	});
}

vector<RakNetGUID> Client::GetNetworkList(const vector<NetworkID>& players, RakNetGUID except)
{
	return cs.Operate([&players, except]() {
		vector<RakNetGUID> network;

		for (auto it = clients.begin(); it != clients.end(); ++it)
			if (it->first != except && find(players.begin(), players.end(), it->second->player) != players.end())
				network.emplace_back(it->first);

		return network;
	});
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
