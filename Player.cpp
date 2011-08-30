#include "Player.h"

using namespace RakNet;
using namespace std;

const Parameter Player::Param_EnabledPlayers = Parameter(vector<string>(), &Player::GetEnabledRefs);
const Parameter Player::Param_EnabledPlayers_NotSelf = Parameter(vector<string>(), &Player::GetEnabledRefs_NotSelf);
const Parameter Player::Param_AllPlayers = Parameter(vector<string>(), &Player::GetAllRefs);
const Parameter Player::Param_AllPlayers_NotSelf = Parameter(vector<string>(), &Player::GetAllRefs_NotSelf);

#ifdef VAULTMP_DEBUG
Debug* Player::debug;
#endif

Player::Player(unsigned int refID, unsigned int baseID) : Actor(refID, baseID)
{

}

Player::~Player()
{
#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Player object destroyed (ref: %08X)", true, GetReference());
#endif
}

#ifdef VAULTMP_DEBUG
void Player::SetDebugHandler(Debug* debug)
{
    Player::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Player class", true);
}
#endif

list<Player*> Player::GetPlayerList()
{
    list<Player*> playerlist;
    list<Reference*>::iterator it;
    list<Reference*> instances = GameFactory::GetObjectTypes(ID_PLAYER);

    for (it = instances.begin(); it != instances.end(); ++it)
        playerlist.push_back((Player*) *it);

    return playerlist;
}

vector<string> Player::GetRefs(bool enabled, bool notself, bool enabled_disabled, bool self_notself)
{
    vector<string> result;
    list<Player*>::iterator it;
    list<Player*> playerlist = GetPlayerList();

    for (it = playerlist.begin(); it != playerlist.end(); ++it)
    {
        unsigned int refID = (*it)->GetReference();

        if (refID != 0x00 && (!enabled_disabled || (*it)->GetEnabled() == enabled) && (!self_notself || !notself || refID != PLAYER_REFERENCE))
            result.push_back(Utils::LongToHex(refID));

        GameFactory::LeaveReference(*it);
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
