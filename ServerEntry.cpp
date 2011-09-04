#include "ServerEntry.h"

using namespace std;

ServerEntry::ServerEntry(unsigned char game)
{
    this->game = game;
    this->name = "Vault-Tec Multiplayer Mod server";
    this->map = "default";
    this->players = pair<int, int>(0, 0);
    this->ping = USHRT_MAX;
    this->SetServerRule("game", game == FALLOUT3 ? "Fallout 3" : game == NEWVEGAS ? "Fallout NV" : game == OBLIVION ? "TES: Oblivion" : "undefined");
}

ServerEntry::ServerEntry(string name, string map, pair<int, int> players, int ping, unsigned char game)
{
    this->game = game;
    SetServerName(name);
    SetServerMap(map);
    SetServerPlayers(players);
    SetServerPing(ping);
    SetGame(game);
}

void ServerEntry::SetServerName(string name)
{
    this->name = name;
}

void ServerEntry::SetServerMap(string map)
{
    this->map = map;
}

void ServerEntry::SetServerRule(string rule, string value)
{
    this->rules.erase(rule);
    this->rules.insert(pair<string, string>(rule, value));
}

void ServerEntry::SetServerPlayers(pair<int, int> players)
{
    this->players = players;
}

void ServerEntry::SetServerPing(int ping)
{
    this->ping = ping;
}

void ServerEntry::SetGame(unsigned char game)
{
    this->game = game;
    this->SetServerRule("game", game == FALLOUT3 ? "Fallout 3" : game == NEWVEGAS ? "Fallout NV" : game == OBLIVION ? "TES: Oblivion" : "undefined");
}

string ServerEntry::GetServerName()
{
    return name;
}

string ServerEntry::GetServerMap()
{
    return map;
}

map<string, string> ServerEntry::GetServerRules()
{
    return rules;
}

pair<int, int> ServerEntry::GetServerPlayers()
{
    return players;
}

int ServerEntry::GetServerPing()
{
    return ping;
}

unsigned char ServerEntry::GetGame()
{
    return game;
}
