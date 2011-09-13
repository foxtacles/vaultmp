#include "Player.h"

using namespace std;

#ifdef VAULTMP_DEBUG
Debug* Player::debug = NULL;
#endif

Player::Player(unsigned int refID, unsigned int baseID) : Actor(refID, baseID)
{
    vector<unsigned char>::iterator it;
    vector<unsigned char> data = API::RetrieveAllControls();

    for (it = data.begin(); it != data.end(); ++it)
        player_Controls.insert(pair<unsigned char, pair<Value<unsigned char>, Value<bool> > >(*it, pair<Value<unsigned char>, Value<bool> >(Value<unsigned char>(), Value<bool>(true))));
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

const Parameter Player::CreateFunctor(unsigned int flags, NetworkID player)
{
    return Parameter(vector<string>(), new PlayerFunctor(flags, player));
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

unsigned char Player::GetPlayerControl(unsigned char control) const
{
    return SAFE_FIND(player_Controls, control)->second.first.Get();
}

bool Player::GetPlayerControlEnabled(unsigned char control) const
{
    return SAFE_FIND(player_Controls, control)->second.second.Get();
}

Lockable* Player::SetPlayerControl(unsigned char control, unsigned char key)
{
    Value<unsigned char>& data = SAFE_FIND(this->player_Controls, control)->second.first;

    if (data.Get() == key)
        return NULL;

    if (!data.Set(key))
        return NULL;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Player control code %02X was associated with key %02X (ref: %08X)", true, control, key, this->GetReference());
#endif

    return &data;
}

Lockable* Player::SetPlayerControlEnabled(unsigned char control, bool state)
{
    Value<bool>& data = SAFE_FIND(this->player_Controls, control)->second.second;

    if (data.Get() == state)
        return NULL;

    if (!data.Set(state))
        return NULL;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Player control code %02X enabled state was set to %d (ref: %08X)", true, control, (int) state, this->GetReference());
#endif

    return &data;
}

vector<string> PlayerFunctor::operator()()
{
    vector<string> result;

    if (this->player)
    {
        Player* player = (Player*) GameFactory::GetObject(ID_PLAYER, this->player);

        if (player)
        {
            if (flags & FLAG_MOVCONTROLS)
            {
                unsigned int forward, backward, left, right;
                if (API::GetGameCode() & FALLOUT_GAMES)
                {
                    forward = player->GetPlayerControl(Values::Fallout::ControlCode_Forward);
                    backward = player->GetPlayerControl(Values::Fallout::ControlCode_Backward);
                    left = player->GetPlayerControl(Values::Fallout::ControlCode_Left);
                    right = player->GetPlayerControl(Values::Fallout::ControlCode_Right);
                }
                else
                {
                    forward = player->GetPlayerControl(Values::Oblivion::ControlCode_Forward);
                    backward = player->GetPlayerControl(Values::Oblivion::ControlCode_Backward);
                    left = player->GetPlayerControl(Values::Oblivion::ControlCode_SlideLeft);
                    right = player->GetPlayerControl(Values::Oblivion::ControlCode_SlideRight);
                }

                unsigned int movcontrols = (right | (left << 8) | (backward << 16) | (forward << 24));

                char value[64];
                snprintf(value, sizeof(value), "%d", movcontrols);
                result.push_back(string(value));
            }

            GameFactory::LeaveReference(player);
        }
    }
    else
    {
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
    }

    _next(result);

    return result;
}

PlayerFunctor::~PlayerFunctor()
{

}
