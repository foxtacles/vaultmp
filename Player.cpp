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

Player::Player(RakNetGUID guid, string base) : Actor(base)
{
    Startup(guid);
}

Player::Player(RakNetGUID guid, string ref, string base) : Actor(ref, base)
{
    Startup(guid);
}

Player::~Player()
{
    playerlist.erase(this->guid);

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Player object destroyed (ref: %s)", GetReference().c_str());
        debug->Print(text, true);
    }
#endif
}

#ifdef VAULTMP_DEBUG
void Player::SetDebugHandler(Debug* debug)
{
    Player::debug = debug;

    if (debug != NULL)
        debug->Print((char*) "Attached debug handler to Player class", true);
}
#endif

void Player::Startup(RakNetGUID guid)
{
    this->guid = guid;
    playerlist.insert(pair<RakNetGUID, Player*>(this->guid, this));

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "New player object created (ref: %s, guid: %s)", this->GetReference().c_str(), this->guid.ToString());
        debug->Print(text, true);
    }
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

Player* Player::GetPlayerFromRefID(string refID)
{
    map<RakNetGUID, Player*>::iterator it;

    for (it = playerlist.begin(); it != playerlist.end(); ++it)
    {
        if (it->second->GetReference().compare(refID) == 0)
            return it->second;
    }

    return NULL;
}

vector<string> Player::GetRefs(string cmd, bool enabled, bool notself)
{
    int skipflag = MAX_SKIP_FLAGS;

    if (cmd.compare("GetPos") == 0)
        skipflag = SKIPFLAG_GETPOS;
    else if (cmd.compare("GetParentCell") == 0)
        skipflag = SKIPFLAG_GETPARENTCELL;
    else if (cmd.compare("GetPlayerValue") == 0)
        skipflag = SKIPFLAG_GETACTORVALUE;
    else if (cmd.compare("GetDead") == 0)
        skipflag = SKIPFLAG_GETDEAD;

    vector<string> result;
    map<RakNetGUID, Player*>::iterator it;

    for (it = playerlist.begin(); it != playerlist.end(); ++it)
    {
        string refID = it->second->GetReference();

        if (!refID.empty() && (!notself || refID.compare("player") != 0) && (!enabled || it->second->GetActorEnabled()) && !it->second->GetActorOverrideFlag(skipflag))
            result.push_back(refID);
    }

    return result;
}

vector<string> Player::GetAllRefs(string cmd)
{
    return GetRefs(cmd, false, false);
}

vector<string> Player::GetAllRefs_NotSelf(string cmd)
{
    return GetRefs(cmd, false, true);
}

vector<string> Player::GetEnabledRefs(string cmd)
{
    return GetRefs(cmd, true, false);
}

vector<string> Player::GetEnabledRefs_NotSelf(string cmd)
{
    return GetRefs(cmd, true, true);
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
                debug->Print((char*) "All player instances destroyed", true);
#endif

        playerlist.clear();

        initialized = false;
    }
}

map<RakNetGUID, Player*> Player::GetPlayerList()
{
    return playerlist;
}

