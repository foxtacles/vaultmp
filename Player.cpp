#include "Player.h"

using namespace RakNet;
using namespace std;

map<RakNetGUID, Player*> Player::playerlist;
bool Player::initialized = false;

Parameter Player::Param_EnabledPlayers = Parameter(vector<string>(), &Player::GetEnabledRefs);
Parameter Player::Param_EnabledPlayers_NotSelf = Parameter(vector<string>(), &Player::GetEnabledRefs_NotSelf);
Parameter Player::Param_AllPlayers = Parameter(vector<string>(), &Player::GetAllRefs);
Parameter Player::Param_AllPlayers_NotSelf = Parameter(vector<string>(), &Player::GetAllRefs_NotSelf);

#ifdef VAULTMP_DEBUG
Debug* Player::debug;
#endif

Player::Player(RakNetGUID guid, unsigned int baseID) : Actor(baseID)
{
    Startup(guid);
}

Player::Player(RakNetGUID guid, unsigned int refID, unsigned int baseID) : Actor(refID, baseID)
{
    Startup(guid);
}

Player::~Player()
{
    playerlist.erase(this->guid);

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Player object destroyed (ref: %08X)", true, GetReference());
#endif
}

RakNetGUID Player::GetPlayerGUID()
{
    return guid;
}

#ifdef VAULTMP_DEBUG
void Player::SetDebugHandler(Debug* debug)
{
    Player::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Player class", true);
}
#endif

void Player::Startup(RakNetGUID guid)
{
    this->guid = guid;
    playerlist.insert(pair<RakNetGUID, Player*>(this->guid, this));

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("New player object created (ref: %08X, guid: %s)", true, this->GetReference(), this->guid.ToString());
#endif
}

Player* Player::GetPlayerFromGUID(RakNetGUID guid)
{
    map<RakNetGUID, Player*>::iterator it;
    it = playerlist.find(guid);

    if (it != playerlist.end())
        return it->second;

    return NULL;
}

Player* Player::GetPlayerFromRefID(unsigned int refID)
{
    map<RakNetGUID, Player*>::iterator it;

    for (it = playerlist.begin(); it != playerlist.end(); ++it)
    {
        if (it->second->GetReference() == refID)
            return it->second;
    }

    return NULL;
}

vector<string> Player::GetRefs(bool enabled, bool notself, bool enabled_disabled, bool self_notself)
{
    vector<string> result;
    map<RakNetGUID, Player*>::iterator it;

    for (it = playerlist.begin(); it != playerlist.end(); ++it)
    {
        unsigned int refID = it->second->GetReference();

        if (refID != 0x00 && (!enabled_disabled || it->second->GetActorEnabled() == enabled) && (!self_notself || !notself || refID != PLAYER_REFERENCE))
            result.push_back(Utils::LongToHex(refID));
    }

    return result;
}

vector<string> Player::GetAllRefs()
{
    return GetRefs(false, false);
}

vector<string> Player::GetAllRefs_NotSelf()
{
    return GetRefs(false, true, false, true);
}

vector<string> Player::GetEnabledRefs()
{
    return GetRefs(true, false, true, false);
}

vector<string> Player::GetEnabledRefs_NotSelf()
{
    return GetRefs(true, true, true, true);
}

void Player::Initialize()
{
    if (!initialized)
    {
        initialized = true;
    }
}

void Player::DestroyInstances()
{
    if (initialized)
    {
        int size = playerlist.size();

        if (size != 0)
        {
            map<RakNetGUID, Player*>::iterator it;
            int i = 0;
            Player* pPlayers[size];

            for (it = playerlist.begin(); it != playerlist.end(); ++it, i++)
                pPlayers[i] = it->second;

            for (i = 0; i < size; i++)
                delete pPlayers[i];
        }

#ifdef VAULTMP_DEBUG
            if (debug != NULL)
                debug->Print("All player instances destroyed", true);
#endif

        playerlist.clear();

        initialized = false;
    }
}

map<RakNetGUID, Player*> Player::GetPlayerList()
{
    return playerlist;
}

