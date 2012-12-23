#ifndef SERVERENTRY_H
#define SERVERENTRY_H

#include <climits>
#include <string>
#include <map>
#include <vector>

#include "vaultmp.h"

class ServerEntry
{
	private:
		std::string name;
		std::string map;
		std::map<std::string, std::string> rules;
		std::pair<unsigned int, unsigned int> players;
		unsigned int ping;
		unsigned char game;
		std::vector<std::string> modfiles;

		ServerEntry& operator=(const ServerEntry&) = delete;

	public:
		void SetServerName(const std::string& name);
		void SetServerMap(const std::string& map);
		void SetServerRule(const std::string& rule, const std::string& value);
		void SetServerPlayers(const std::pair<unsigned int, unsigned int>& players);
		void SetServerPing(unsigned int ping);
		void SetGame(unsigned char game);
		void SetModFiles(const std::string& name);

		const std::string& GetServerName();
		const std::string& GetServerMap();
		const std::map<std::string, std::string>& GetServerRules();
		const std::pair<unsigned int, unsigned int>& GetServerPlayers();
		unsigned int GetServerPing();
		unsigned char GetGame();
		const std::vector<std::string>& GetServerModFiles();
		void ClearModFiles();

		ServerEntry(unsigned char game);
		ServerEntry(const std::string& name, const std::string& map, const std::pair<unsigned int, unsigned int>& players, unsigned int ping, unsigned char game);
};

#endif
