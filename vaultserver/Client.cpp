#include "Client.h"

using namespace RakNet;
using namespace std;

map<RakNetGUID, Client*> Client::clients;
stack<unsigned int> Client::clientID;

Client::Client( RakNetGUID guid, NetworkID player )
{
	clients.insert( pair<RakNetGUID, Client*>( guid, this ) );
	this->guid = guid;
	this->player = player;
	this->ID = clientID.top();
	clientID.pop();
}

Client::~Client()
{
	clients.erase( this->guid );
	clientID.push( this->ID );
}

void Client::SetMaximumClients( unsigned int clients )
{
	for ( int i = clients - 1; i >= 0; i-- )
		clientID.push( i );
}

int Client::GetClientCount()
{
	return clients.size();
}

Client* Client::GetClientFromGUID( RakNetGUID guid )
{
	map<RakNetGUID, Client*>::iterator it;
	it = clients.find( guid );

	if ( it != clients.end() )
		return it->second;

	return NULL;
}

Client* Client::GetClientFromID( unsigned int ID )
{
	map<RakNetGUID, Client*>::iterator it;

	for ( it = clients.begin(); it != clients.end(); ++it )
		if ( it->second->GetID() == ID )
			return it->second;

	return NULL;
}

vector<RakNetGUID> Client::GetNetworkList( Client* except )
{
	vector<RakNetGUID> network;
	map<RakNetGUID, Client*>::iterator it;

	for ( it = clients.begin(); it != clients.end(); ++it )
		if ( it->second != except )
			network.push_back( it->first );

	return network;
}

vector<RakNetGUID> Client::GetNetworkList( RakNetGUID except )
{
	vector<RakNetGUID> network;
	map<RakNetGUID, Client*>::iterator it;

	for ( it = clients.begin(); it != clients.end(); ++it )
		if ( it->first != except )
			network.push_back( it->first );

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
