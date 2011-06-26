#include "Player.h"

using namespace RakNet;
using namespace std;

map<RakNetGUID, string> Player::players;
map<RakNetGUID, Player*> Player::playersguids;
map<string, Player*> Player::playersrefs;
Debug* Player::debug;

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

    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "New player object created (guid: %s)", this->guid.ToString());
        debug->Print(text, true);
    }
}

Player::~Player()
{
    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player object destroyed (ref: %s, guid: %s)", this->refID.c_str(), this->guid.ToString());
        debug->Print(text, true);
    }

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

    if (debug != NULL)
        debug->Print((char*) "All player instances destroyed", true);
}

void Player::SetDebugHandler(Debug* debug)
{
    Player::debug = debug;

    if (debug != NULL)
        debug->Print((char*) "Attached debug handler to Player class", true);
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

    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player name was set to %s (ref: %s, guid: %s)", this->name.c_str(), this->refID.c_str(), this->guid.ToString());
        debug->Print(text, true);
    }
}

bool Player::SetPlayerPos(int cell, float pos)
{
    if (cell >= 0 && cell <= 2 && !nowrite[(cell == 0) ? SKIPFLAG_GETPOS_X : (cell == 1) ? SKIPFLAG_GETPOS_Y : (cell == 2) ? SKIPFLAG_GETPOS_Z : 0])
    {
        if ((pos != 2048.0 && pos != 128.0 && pos != 0.0))
        {
            this->pos[cell] = pos;

            if (debug != NULL)
            {
                char text[128];
                ZeroMemory(text, sizeof(text));

                sprintf(text, "Player %c-coordinate was set to %f (ref: %s, guid: %s)", cell == 0 ? 'X' : cell == 1 ? 'Y' : 'Z', this->pos[cell], this->refID.c_str(), this->guid.ToString());
                debug->Print(text, true);
            }

            return true;
        }
        else
        {
            if (debug != NULL)
            {
                char text[128];
                ZeroMemory(text, sizeof(text));

                sprintf(text, "Player %c-coordinate was NOT set to %f (INVALID) (ref: %s, guid: %s)", cell == 0 ? 'X' : cell == 1 ? 'Y' : 'Z', pos, this->refID.c_str(), this->guid.ToString());
                debug->Print(text, true);
            }

            return false;
        }
    }

    return false;
}

void Player::SetPlayerAngle(float angle)
{
    this->angle = angle;

    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player Z-angle was set to %f (ref: %s, guid: %s)", this->angle, this->refID.c_str(), this->guid.ToString());
        debug->Print(text, true);
    }
}

void Player::SetPlayerHealth(float health)
{
    this->health = health;

    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player health was set to %f (ref: %s, guid: %s)", this->health, this->refID.c_str(), this->guid.ToString());
        debug->Print(text, true);
    }
}

void Player::SetPlayerBaseHealth(float baseHealth)
{
    this->baseHealth = baseHealth;

    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player base health was set to %f (ref: %s, guid: %s)", this->baseHealth, this->refID.c_str(), this->guid.ToString());
        debug->Print(text, true);
    }
}

void Player::SetPlayerCondition(int cell, float cond)
{
    if (cell >= 0 && cell <= 5)
    {
        this->cond[cell] = cond;

        if (debug != NULL)
        {
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Player %s was set to %f (ref: %s, guid: %s)", cell == 0 ? (char*) "PerceptionCondition" : cell == 1 ? (char*) "EnduranceCondition" : cell == 2 ? (char*) "LeftAttackCondition" : cell == 3 ? (char*) "RightAttackCondition" : cell == 4 ? (char*) "LeftMobilityCondition" : (char*) "RightMobilityCondition", this->cond[cell], this->refID.c_str(), this->guid.ToString());
            debug->Print(text, true);
        }
    }
}

void Player::SetPlayerDead(bool dead)
{
    this->dead = dead;

    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player dead state was set to %d (ref: %s, guid: %s)", (int) this->dead, this->refID.c_str(), this->guid.ToString());
        debug->Print(text, true);
    }
}

void Player::SetPlayerAlerted(bool alerted)
{
    this->alerted = alerted;

    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player alerted state was set to %d (ref: %s, guid: %s)", (int) this->alerted, this->refID.c_str(), this->guid.ToString());
        debug->Print(text, true);
    }
}

void Player::SetPlayerMoving(int moving)
{
    if (moving >= 0 && moving <= 8)
    {
        this->moving = moving;

        if (debug != NULL)
        {
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Player running animation was set to %s (ref: %s, guid: %s)", moving == 0 ? (char*) "Idle" : moving == 1 ? (char*) "FastForward" : moving == 2 ? (char*) "FastBackward" : moving == 3 ? (char*) "FastLeft" : moving == 4 ? (char*) "FastRight" : moving == 5 ? (char*) "Forward" : moving == 6 ? (char*) "Backward" : moving == 7 ? (char*) "Left" : (char*) "Right", this->refID.c_str(), this->guid.ToString());
            debug->Print(text, true);
        }
    }

}

void Player::SetPlayerRefID(string refID)
{
    this->refID = refID;
    map<RakNetGUID, string>::iterator it;
    it = players.find(this->guid);
    it->second = refID;
    playersrefs.insert(pair<string, Player*>(refID, this));

    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player refID was set to %s (guid: %s)", this->refID.c_str(), this->guid.ToString());
        debug->Print(text, true);
    }
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
