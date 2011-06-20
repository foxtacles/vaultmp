#include "Player.h"

using namespace RakNet;
using namespace std;

map<RakNetGUID, string> Player::players;
map<RakNetGUID, Player*> Player::playersguids;
map<string, Player*> Player::playersrefs;

Player::Player(RakNetGUID guid)
{
    this->guid = guid;
    refID = "none";
    pos[0] = 0.00;
    pos[1] = 0.00;
    pos[2] = 0.00;
    angle = 0.00;
    health = 0.00;
    baseHealth = 0.00;
    cond[0] = 0.00;
    cond[1] = 0.00;
    cond[2] = 0.00;
    cond[3] = 0.00;
    cond[4] = 0.00;
    cond[5] = 0.00;
    dead = false;
    alerted = false;
    moving = 0;
    name = "Player";
    players.insert(pair<RakNetGUID, string>(guid, refID));
    playersguids.insert(pair<RakNetGUID, Player*>(guid, this));

    for (int i = 0; i < MAX_SKIP_FLAGS; i++)
        nowrite[i] = false;
}

Player::~Player()
{
    players.erase(this->guid);
    playersguids.erase(this->guid);
    if (this->refID.compare("none") != 0) playersrefs.erase(this->refID);
}

Player* Player::GetPlayerFromGUID(RakNetGUID guid)
{
    map<RakNetGUID, Player*>::iterator it;
    it = playersguids.find(guid);

    if (it != playersguids.end())
        return it->second;

    return NULL;
}

Player* Player::GetPlayerFromRefID(string refID)
{
    map<string, Player*>::iterator it;
    it = playersrefs.find(refID);

    if (it != playersrefs.end())
        return it->second;

    return NULL;
}

void Player::DestroyInstances()
{
    map<RakNetGUID, Player*>::iterator it;
    int size = playersguids.size();
    int i = 0;
    Player* pPlayers[size];

    for (it = playersguids.begin(); it != playersguids.end(); it++, i++)
        pPlayers[i] = it->second;

    for (int j = 0; j < size; j++)
        delete pPlayers[j];
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
    float ret = (cell >= 0 && cell <= 2) ? pos[cell] : 0.00;
    return ret;
}

float Player::GetPlayerAngle()
{
    return angle;
}

float Player::GetPlayerHealth()
{
    return health;
}

float Player::GetPlayerBaseHealth()
{
    return baseHealth;
}

float Player::GetPlayerCondition(int cell)
{
    float ret = (cell >= 0 && cell <= 5) ? cond[cell] : 0.00;
    return ret;
}

bool Player::IsPlayerDead()
{
    return dead;
}

bool Player::IsPlayerAlerted()
{
    return alerted;
}

int Player::GetPlayerMoving()
{
    return moving;
}

string Player::GetPlayerRefID()
{
    return refID;
}

pPlayerUpdate Player::GetPlayerUpdateStruct()
{
    pPlayerUpdate data;

    data.type = ID_PLAYER_UPDATE;
    data.guid = guid;
    data.X = pos[0];
    data.Y = pos[1];
    data.Z = pos[2];
    data.A = angle;
    data.health = health;
    data.baseHealth = baseHealth;
    data.conds[0] = cond[0];
    data.conds[1] = cond[1];
    data.conds[2] = cond[2];
    data.conds[3] = cond[3];
    data.conds[4] = cond[4];
    data.conds[5] = cond[5];
    data.dead = dead;
    data.alerted = alerted;
    data.moving = moving;

    return data;
}

bool Player::UpdatePlayerUpdateStruct(pPlayerUpdate* data)
{
    pPlayerUpdate player = this->GetPlayerUpdateStruct();

    if (player.type != data->type ||
        player.guid != data->guid ||
        player.X != data->X ||
        player.Y != data->Y ||
        player.Z != data->Z ||
        player.A != data->A ||
        player.health != data->health ||
        player.baseHealth != data->baseHealth ||
        player.conds[0] != data->conds[0] ||
        player.conds[1] != data->conds[1] ||
        player.conds[2] != data->conds[2] ||
        player.conds[3] != data->conds[3] ||
        player.conds[4] != data->conds[4] ||
        player.conds[5] != data->conds[5] ||
        player.dead != data->dead ||
        player.alerted != data->alerted ||
        player.moving != data->moving)
        {
            (*data) = player;
            return true;
        }

    return false;
}

void Player::SetPlayerName(string name)
{
    this->name = name;
}

void Player::SetPlayerPos(int cell, float pos)
{
    if (cell >= 0 && cell <= 2 && !nowrite[(cell == 0) ? SKIPFLAG_GETPOS_X : (cell == 1) ? SKIPFLAG_GETPOS_Y : (cell == 2) ? SKIPFLAG_GETPOS_Z : 0])
        this->pos[cell] = pos;
}

void Player::SetPlayerAngle(float angle)
{
    this->angle = angle;
}

void Player::SetPlayerHealth(float health)
{
    this->health = health;
}

void Player::SetPlayerBaseHealth(float baseHealth)
{
    this->baseHealth = baseHealth;
}

void Player::SetPlayerCondition(int cell, float cond)
{
    if (cell >= 0 && cell <= 5)
        this->cond[cell] = cond;
}

void Player::SetPlayerDead(bool dead)
{
    this->dead = dead;
}

void Player::SetPlayerAlerted(bool alerted)
{
    this->alerted = alerted;
}

void Player::SetPlayerMoving(int moving)
{
    if (moving >= 0 && moving <= 8)
        this->moving = moving;
}

void Player::SetPlayerRefID(string refID)
{
    this->refID = refID;
    map<RakNetGUID, string>::iterator it;
    it = players.find(this->guid);
    it->second = refID;
    playersrefs.insert(pair<string, Player*>(refID, this));
}

void Player::ToggleNoOverride(int skipflag, bool toggle)
{
    if (skipflag >= 0 && skipflag < MAX_SKIP_FLAGS)
        nowrite[skipflag] = toggle;
}

bool Player::IsPlayerNearPoint(float X, float Y, float Z, float R)
{
    return (sqrt((abs(pos[0] - X) * abs(pos[0] - X)) + (abs(pos[1] - Y) * abs(pos[1] - Y)) + (abs(pos[2] - Z) * abs(pos[2] - Z))) <= R);
}

bool Player::IsCoordinateInRange(int cell, float XYZ, float R)
{
    if (cell >=0 && cell <= 2)
        return (pos[cell] > (XYZ - R) && pos[cell] < (XYZ + R));
    return false;
}
