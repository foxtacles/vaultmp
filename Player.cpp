#include "Player.h"

using namespace std;

#ifdef VAULTMP_DEBUG
Debug* Player::debug = NULL;
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

const Parameter Player::CreateFunctor(unsigned int flags)
{
    return Parameter(vector<string>(), new PlayerFunctor(flags));
}

vector<Player*> Player::GetPlayerList()
{
    vector<Player*> playerlist;
    vector<Reference*>::iterator it;
    vector<Reference*> instances = GameFactory::GetObjectTypes(ID_PLAYER);

    for (it = instances.begin(); it != instances.end(); ++it)
        playerlist.push_back((Player*) *it);

    return playerlist;
}

vector<string> PlayerFunctor::operator()()
{
    vector<string> result;
    vector<Player*>::iterator it;
    vector<Player*> playerlist = Player::GetPlayerList();

    for (it = playerlist.begin(); it != playerlist.end(); GameFactory::LeaveReference(*it), ++it)
    {
        Player* player = *it;
        unsigned int refID = player->GetReference();

        if (refID != 0x00000000)
        {
            if (flags & FLAG_NOTSELF && refID == PLAYER_REFERENCE)
                continue;

            if (flags & FLAG_ENABLED && !player->GetEnabled())
                continue;
            else if (flags & FLAG_DISABLED && player->GetEnabled())
                continue;

            if (flags & FLAG_ALIVE && player->GetActorDead())
                continue;
            else if (flags & FLAG_DEAD && !player->GetActorDead())
                continue;

            if (flags & FLAG_ISALERTED && !player->GetActorAlerted())
                continue;
            else if (flags & FLAG_NOTALERTED && player->GetActorAlerted())
                continue;
        }

        result.push_back(Utils::LongToHex(refID));
    }

    return result;
}

PlayerFunctor::~PlayerFunctor()
{

}
