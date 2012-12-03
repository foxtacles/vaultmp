#ifndef SERVERENTRY_H
#define SERVERENTRY_H

#include <climits>
#include <string>
#include <map>
#include <vector>

#include "vaultmp.h"

using namespace std;

class ServerEntry
{
	private:
		string name;
		string map;
		std::map<string, string> rules;
		pair<unsigned int, unsigned int> players;
		unsigned int ping;
		unsigned char game;
		std::vector<string> modfiles;

		ServerEntry& operator=(const ServerEntry&) = delete;

	public:
		void SetServerName(const string& name);
		void SetServerMap(const string& map);
		void SetServerRule(const string& rule, const string& value);
		void SetServerPlayers(const pair<unsigned int, unsigned int>& players);
		void SetServerPing(unsigned int ping);
		void SetGame(unsigned char game);
		void SetModFiles(const string& name);

		const string& GetServerName();
		const string& GetServerMap();
		const std::map<string, string>& GetServerRules();
		const pair<unsigned int, unsigned int>& GetServerPlayers();
		unsigned int GetServerPing();
		unsigned char GetGame();
		const std::vector<string> GetServerModFiles();
		void ClearModFiles();

		ServerEntry(unsigned char game);
		ServerEntry(const string& name, const string& map, const pair<unsigned int, unsigned int>& players, unsigned int ping, unsigned char game);
};

#endif
