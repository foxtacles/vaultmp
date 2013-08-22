#ifndef SERVERENTRY_H
#define SERVERENTRY_H

#include "vaultmp.hpp"

#include <string>
#include <map>
#include <vector>
#include <climits>

class ServerEntry
{
	private:
		std::string name;
		std::string map;
		std::map<std::string, std::string> rules;
		std::pair<unsigned int, unsigned int> players;
		unsigned int ping;
		std::vector<std::string> modfiles;

		ServerEntry& operator=(const ServerEntry&) = delete;

	public:
		void SetServerName(const std::string& name);
		void SetServerMap(const std::string& map);
		void SetServerRule(const std::string& rule, const std::string& value);
		void SetServerPlayers(const std::pair<unsigned int, unsigned int>& players);
		void SetServerPing(unsigned int ping);
		void SetModFiles(const std::string& name);

		const std::string& GetServerName();
		const std::string& GetServerMap();
		const std::map<std::string, std::string>& GetServerRules();
		const std::pair<unsigned int, unsigned int>& GetServerPlayers();
		unsigned int GetServerPing();
		const std::vector<std::string>& GetServerModFiles();
		void ClearModFiles();

		ServerEntry() : name("Vault-Tec Multiplayer Mod server"), map("default"), ping(USHRT_MAX) {}
		ServerEntry(const std::string& name, const std::string& map, const std::pair<unsigned int, unsigned int>& players, unsigned int ping) : name(name), map(map), players(players), ping(ping) {}
};

#endif
