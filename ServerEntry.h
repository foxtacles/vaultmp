#ifndef SERVERENTRY_H
#define SERVERENTRY_H

#include <climits>
#include <string>
#include <map>

#include "vaultmp.h"

using namespace std;

class ServerEntry
{

	private:
		string name;
		string map;
		std::map<string, string> rules;
		pair<int, int> players;
		int ping;
		unsigned char game;

		ServerEntry& operator=( const ServerEntry& );

	public:
		void SetServerName( string name );
		void SetServerMap( string map );
		void SetServerRule( string rule, string value );
		void SetServerPlayers( pair<int, int> players );
		void SetServerPing( int ping );
		void SetGame( unsigned char game );

		string GetServerName();
		string GetServerMap();
		std::map<string, string> GetServerRules();
		pair<int, int> GetServerPlayers();
		int GetServerPing();
		unsigned char GetGame();

		ServerEntry( unsigned char game );
		ServerEntry( string name, string map, pair<int, int> players, int ping, unsigned char game );
};

#endif
