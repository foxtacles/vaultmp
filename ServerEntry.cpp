#include "ServerEntry.h"

using namespace std;

ServerEntry::ServerEntry(bool NewVegas)
{
    this->NewVegas = NewVegas;
    this->name = "Vault-Tec Multiplayer Mod server";
    this->map = "default";
    this->players = pair<int, int>(0, 0);
    this->ping = 999;
    this->SetServerRule("game", NewVegas ? "Fallout NV" : "Fallout 3");
}

ServerEntry::ServerEntry(string name, string map, pair<int, int> players, int ping, bool NewVegas)
{
    this->NewVegas = NewVegas;
    SetServerName(name);
    SetServerMap(map);
    SetServerPlayers(players);
    SetServerPing(ping);
    SetServerRule("game", NewVegas ? "Fallout NV" : "Fallout 3");
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

bool ServerEntry::IsNewVegas()
{
    return NewVegas;
}
