#include "Player.h"

using namespace RakNet;
using namespace std;

map<RakNetGUID, string> Player::players;
map<RakNetGUID, Player*> Player::playersguids;

Player::Player(RakNetGUID guid)
{
    players.insert(pair<RakNetGUID, string>(guid, refID));
    playersguids.insert(pair<RakNetGUID, Player*>(guid, this));
    this->guid = guid;
    refID = "none";
    pos[0] = 0.00;
    pos[1] = 0.00;
    pos[2] = 0.00;
    name = "Player";
}

Player::~Player()
{
    players.erase(this->guid);
    playersguids.erase(this->guid);
}

Player* Player::GetPlayerFromGUID(RakNetGUID guid)
{
    map<RakNetGUID, Player*>::iterator it;
    it = playersguids.find(guid);

    if (it != playersguids.end())
        return it->second;

    return NULL;
}

map<RakNetGUID, string> Player::GetPlayerList()
{
    return players;
}

string Player::GetPlayerName()
{
    return name;
}

float Player::GetPlayerPos(int cell)
{
    float ret = (cell == 0) ? pos[0] : (cell == 1) ? pos[1] : (cell == 2) ? pos[2] : 0.00;
    return ret;
}

string Player::GetPlayerRefID()
{
    return refID;
}

void Player::SetPlayerName(string name)
{
    this->name = name;
}

void Player::SetPlayerPos(int cell, float pos)
{
    if (cell >= 0 && cell <= 3)
        this->pos[cell] = pos;
}

void Player::SetPlayerRefID(string refID)
{
    this->refID = refID;
}
