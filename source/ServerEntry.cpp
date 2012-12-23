#include "ServerEntry.h"

using namespace std;

ServerEntry::ServerEntry(unsigned char game) : name("Vault-Tec Multiplayer Mod server"), map("default"), ping(USHRT_MAX)
{
	SetGame(game);
}

ServerEntry::ServerEntry(const string& name, const string& map, const pair<unsigned int, unsigned int>& players, unsigned int ping, unsigned char game) : name(name), map(map), players(players), ping(ping)
{
	SetGame(game);
}

void ServerEntry::SetServerName(const string& name)
{
	this->name = name;
}

void ServerEntry::SetServerMap(const string& map)
{
	this->map = map;
}

void ServerEntry::SetServerRule(const string& rule, const string& value)
{
	this->rules.erase(rule);
	// emplace
	this->rules.insert(make_pair(rule, value));
}

void ServerEntry::SetServerPlayers(const pair<unsigned int, unsigned int>& players)
{
	this->players = players;
}

void ServerEntry::SetServerPing(unsigned int ping)
{
	this->ping = ping;
}

void ServerEntry::SetGame(unsigned char game)
{
	this->game = game;
	this->SetServerRule("game", game == FALLOUT3 ? "Fallout 3" : game == NEWVEGAS ? "Fallout NV" : "undefined");
}

void ServerEntry::SetModFiles(const string& name)
{
    this->modfiles.push_back(name);
}

const string& ServerEntry::GetServerName()
{
	return name;
}

const string& ServerEntry::GetServerMap()
{
	return map;
}

const map<string, string>& ServerEntry::GetServerRules()
{
	return rules;
}

const pair<unsigned int, unsigned int>& ServerEntry::GetServerPlayers()
{
	return players;
}

unsigned int ServerEntry::GetServerPing()
{
	return ping;
}

unsigned char ServerEntry::GetGame()
{
	return game;
}

const std::vector<string>& ServerEntry::GetServerModFiles()
{
    return modfiles;
}

void ServerEntry::ClearModFiles()
{
    modfiles.clear();
}
