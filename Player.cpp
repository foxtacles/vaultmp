#include "Player.h"

using namespace RakNet;
using namespace std;

map<RakNetGUID, string> Player::players;
map<RakNetGUID, Player*> Player::playersguids;
map<string, Player*> Player::playersrefs;
CRITICAL_SECTION Player::cs_player;
bool Player::initialized = false;

Parameter Player::Param_EnabledPlayers = Parameter(vector<string>(), &Player::GetEnabledRefs);
Parameter Player::Param_EnabledPlayers_NotSelf = Parameter(vector<string>(), &Player::GetEnabledRefs_NotSelf);
Parameter Player::Param_AllPlayers = Parameter(vector<string>(), &Player::GetAllRefs);
Parameter Player::Param_AllPlayers_NotSelf = Parameter(vector<string>(), &Player::GetAllRefs_NotSelf);

#ifdef VAULTMP_DEBUG
Debug* Player::debug;
#endif

Player::Player(RakNetGUID guid)
{
    this->guid = guid;
    inventory = NULL;
    refID = "none";
    pos[X_AXIS] = 0.00;
    pos[Y_AXIS] = 0.00;
    pos[Z_AXIS] = 0.00;
    angle = 0.00;
    gcell = 0x00;
    ncell = 0x00;
    health = 0.00;
    baseHealth = 0.00;
    cond[COND_PERCEPTION] = 0.00;
    cond[COND_ENDURANCE] = 0.00;
    cond[COND_LEFTATTACK] = 0.00;
    cond[COND_RIGHTATTACK] = 0.00;
    cond[COND_LEFTMOBILITY] = 0.00;
    cond[COND_RIGHTMOBILITY] = 0.00;
    dead = false;
    alerted = false;
    moving = MOV_IDLE;
    name = "Player";
    enabled = true;
    players.insert(pair<RakNetGUID, string>(guid, refID));
    playersguids.insert(pair<RakNetGUID, Player*>(guid, this));

    for (int i = 0; i < MAX_SKIP_FLAGS; i++)
        nowrite[i] = false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "New player object created (guid: %s)", this->guid.ToString());
        debug->Print(text, true);
    }
#endif
}

Player::~Player()
{
#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player object destroyed (ref: %s)", this->refID.c_str());
        debug->Print(text, true);
    }
#endif

    if (inventory != NULL)
        delete inventory;

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

vector<string> Player::GetRefs(string cmd, bool enabled, bool notself)
{
    int skipflag = MAX_SKIP_FLAGS;

    if (cmd.compare("GetPos") == 0)
        skipflag = SKIPFLAG_GETPOS;
    else if (cmd.compare("GetParentCell") == 0)
        skipflag = SKIPFLAG_GETPARENTCELL;
    else if (cmd.compare("GetActorValue") == 0)
        skipflag = SKIPFLAG_GETACTORVALUE;
    else if (cmd.compare("GetDead") == 0)
        skipflag = SKIPFLAG_GETDEAD;

    vector<string> result;
    map<RakNetGUID, Player*>::iterator it;

    for (it = playersguids.begin(); it != playersguids.end(); ++it)
    {
        string refID = it->second->GetPlayerRefID();

        if (refID.compare("none") != 0 && (!notself || refID.compare("player") != 0) && (!enabled || it->second->GetPlayerEnabled()) && !it->second->GetPlayerOverrideFlag(skipflag))
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
        InitializeCriticalSection(&cs_player);
        initialized = true;
    }
}

void Player::DestroyInstances()
{
    if (initialized)
    {
        EnterCriticalSection(&cs_player);
        LeaveCriticalSection(&cs_player);
        DeleteCriticalSection(&cs_player);

        map<RakNetGUID, Player*>::iterator it;
        int size = playersguids.size();

        if (size != 0)
        {
            int i = 0;
            Player* pPlayers[size];

            for (it = playersguids.begin(); it != playersguids.end(); ++it, i++)
                pPlayers[i] = it->second;

            for (int j = 0; j < size; j++)
                delete pPlayers[j];

#ifdef VAULTMP_DEBUG
            if (debug != NULL)
                debug->Print((char*) "All player instances destroyed", true);
#endif
        }

        players.clear();
        playersguids.clear();
        playersrefs.clear();

        initialized = false;
    }
}

void Player::StartSession()
{
    EnterCriticalSection(&cs_player);
}

void Player::EndSession()
{
    LeaveCriticalSection(&cs_player);
}

#ifdef VAULTMP_DEBUG
void Player::SetDebugHandler(Debug* debug)
{
    Player::debug = debug;

    if (debug != NULL)
        debug->Print((char*) "Attached debug handler to Player class", true);
}
#endif

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
    float ret = (cell >= X_AXIS && cell <= Z_AXIS) ? pos[cell] : 0.00;
    return ret;
}

float Player::GetPlayerAngle()
{
    return angle;
}

DWORD Player::GetPlayerGameCell()
{
    return gcell;
}

DWORD Player::GetPlayerNetworkCell()
{
    return ncell;
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
    float ret = (cell >= COND_PERCEPTION && cell <= COND_LEFTMOBILITY) ? cond[cell] : 0.00;
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

bool Player::GetPlayerEnabled()
{
    return enabled;
}

Inventory* Player::GetPlayerInventory()
{
    return inventory;
}

Parameter Player::GetPlayerRefParam()
{
    vector<string> self;
    if (this->GetPlayerRefID().compare("none") != 0) self.push_back(this->GetPlayerRefID());
    Parameter Param_Self = Parameter(self, &Data::EmptyVector);
    return Param_Self;
}

Parameter Player::GetPlayerNameParam()
{
    vector<string> self;
    self.push_back(this->GetPlayerName());
    Parameter Param_Self = Parameter(self, &Data::EmptyVector);
    return Param_Self;
}

pPlayerUpdate Player::GetPlayerUpdateStruct()
{
    pPlayerUpdate data;

    data.type = ID_PLAYER_UPDATE;
    data.guid = guid;
    data.X = pos[X_AXIS];
    data.Y = pos[Y_AXIS];
    data.Z = pos[Z_AXIS];
    data.A = angle;
    data.alerted = alerted;
    data.moving = moving;

    return data;
}

pPlayerStateUpdate Player::GetPlayerStateUpdateStruct()
{
    pPlayerStateUpdate data;

    data.type = ID_PLAYER_STATE_UPDATE;
    data.guid = guid;
    data.health = health;
    data.baseHealth = baseHealth;
    data.conds[COND_PERCEPTION] = cond[COND_PERCEPTION];
    data.conds[COND_ENDURANCE] = cond[COND_ENDURANCE];
    data.conds[COND_LEFTATTACK] = cond[COND_LEFTATTACK];
    data.conds[COND_RIGHTATTACK] = cond[COND_RIGHTATTACK];
    data.conds[COND_LEFTMOBILITY] = cond[COND_LEFTMOBILITY];
    data.conds[COND_RIGHTMOBILITY] = cond[COND_RIGHTMOBILITY];
    data.dead = dead;

    return data;
}

pPlayerCellUpdate Player::GetPlayerCellUpdateStruct()
{
    pPlayerCellUpdate data;

    data.type = ID_PLAYER_CELL_UPDATE;
    data.guid = guid;
    data.cell = gcell;

    return data;
}

pPlayerItemUpdate Player::GetPlayerItemUpdateStruct(Item* item)
{
    pPlayerItemUpdate data;

    data.type = ID_PLAYER_ITEM_UPDATE;
    data.hidden = false;
    data.guid = guid;
    data.item = (*item);

    strcpy(data.baseID, item->item->first);

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
            player.alerted != data->alerted ||
            player.moving != data->moving)
    {
        (*data) = player;
        return true;
    }

    return false;
}

bool Player::UpdatePlayerStateUpdateStruct(pPlayerStateUpdate* data)
{
    pPlayerStateUpdate player = this->GetPlayerStateUpdateStruct();

    if (player.type != data->type ||
            player.guid != data->guid ||
            player.health != data->health ||
            player.baseHealth != data->baseHealth ||
            player.conds[COND_PERCEPTION] != data->conds[COND_PERCEPTION] ||
            player.conds[COND_ENDURANCE] != data->conds[COND_ENDURANCE] ||
            player.conds[COND_LEFTATTACK] != data->conds[COND_LEFTATTACK] ||
            player.conds[COND_RIGHTATTACK] != data->conds[COND_RIGHTATTACK] ||
            player.conds[COND_LEFTMOBILITY] != data->conds[COND_LEFTMOBILITY] ||
            player.conds[COND_RIGHTMOBILITY] != data->conds[COND_RIGHTMOBILITY] ||
            player.dead != data->dead)
    {
        (*data) = player;
        return true;
    }

    return false;
}

bool Player::UpdatePlayerCellUpdateStruct(pPlayerCellUpdate* data)
{
    pPlayerCellUpdate player = this->GetPlayerCellUpdateStruct();

    if (player.type != data->type ||
            player.guid != data->guid ||
            player.cell != data->cell)
    {
        (*data) = player;
        return true;
    }

    return false;
}

bool Player::UpdatePlayerItemUpdateStruct(list<pPlayerItemUpdate>* items, Inventory* inv)
{
    if (inv == NULL || items == NULL || this->inventory == NULL)
        return false;

    Inventory* invcopy = inventory->Copy();
    Inventory diff;

    Inventory::CreateDiff(inv, invcopy, &diff);

    if (diff.IsEmpty())
    {
        delete invcopy;
        return false;
    }

    items->clear();

    list<Item*> items_diff = diff.GetItemList();
    list<Item*>::iterator it;
    int i = 0;

    for (it = items_diff.begin(), i = 0; it != items_diff.end(); ++it, i++)
        items->push_back(GetPlayerItemUpdateStruct(*it));

    invcopy->Copy(inv);
    delete invcopy;

    return true;
}

bool Player::SetPlayerName(string name)
{
    if (this->name == name)
        return false;

    this->name = name;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player name was set to %s (ref: %s)", this->name.c_str(), this->refID.c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

bool Player::SetPlayerPos(int cell, float pos)
{
    if (cell >= X_AXIS && cell <= Z_AXIS && !nowrite[SKIPFLAG_GETPOS])
    {
        if (this->pos[cell] == pos)
            return false;

        if ((pos != 2048.0 && pos != 128.0 && pos != 0.0))
        {
            this->pos[cell] = pos;

#ifdef VAULTMP_DEBUG
            if (debug != NULL)
            {
                char text[128];
                ZeroMemory(text, sizeof(text));

                sprintf(text, "Player %c-coordinate was set to %f (ref: %s)", cell == 0 ? 'X' : cell == 1 ? 'Y' : 'Z', this->pos[cell], this->refID.c_str());
                debug->Print(text, true);
            }
#endif

            return true;
        }
        else
        {
#ifdef VAULTMP_DEBUG
            if (debug != NULL)
            {
                char text[128];
                ZeroMemory(text, sizeof(text));

                sprintf(text, "Player %c-coordinate was NOT set to %f (INVALID) (ref: %s)", cell == 0 ? 'X' : cell == 1 ? 'Y' : 'Z', pos, this->refID.c_str());
                debug->Print(text, true);
            }
#endif

            return false;
        }
    }

    return false;
}

bool Player::SetPlayerAngle(float angle)
{
    if (this->angle == angle)
        return false;

    this->angle = angle;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player Z-angle was set to %f (ref: %s)", this->angle, this->refID.c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

bool Player::SetPlayerGameCell(DWORD cell)
{
    if (!nowrite[SKIPFLAG_GETPARENTCELL])
    {
        if (cell != 0x00)
        {
            if (this->gcell == cell)
                return false;

            this->gcell = cell;

#ifdef VAULTMP_DEBUG
            if (debug != NULL)
            {
                char text[128];
                ZeroMemory(text, sizeof(text));

                sprintf(text, "Player game cell was set to %x (ref: %s)", this->gcell, this->refID.c_str());
                debug->Print(text, true);
            }
#endif

            return true;
        }
        else
        {
#ifdef VAULTMP_DEBUG
            if (debug != NULL)
            {
                char text[128];
                ZeroMemory(text, sizeof(text));

                sprintf(text, "Player game cell was NOT set to %x (INVALID) (ref: %s)", cell, this->refID.c_str());
                debug->Print(text, true);
            }
#endif

            return false;
        }
    }

    return false;
}

bool Player::SetPlayerNetworkCell(DWORD cell)
{
    if (cell != 0x00)
    {
        if (this->ncell == cell)
            return false;

        this->ncell = cell;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Player network cell was set to %x (ref: %s)", this->ncell, this->refID.c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }
    else
    {
#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Player network cell was NOT set to %x (INVALID) (ref: %s)", cell, this->refID.c_str());
            debug->Print(text, true);
        }
#endif

        return false;
    }

    return false;
}

bool Player::SetPlayerHealth(float health)
{
    if (!nowrite[SKIPFLAG_GETACTORVALUE])
    {
        if (this->health == health)
            return false;

        this->health = health;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Player health was set to %f (ref: %s)", this->health, this->refID.c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }

    return false;
}

bool Player::SetPlayerBaseHealth(float baseHealth)
{
    if (this->baseHealth == baseHealth)
        return false;

    this->baseHealth = baseHealth;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player base health was set to %f (ref: %s)", this->baseHealth, this->refID.c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

bool Player::SetPlayerCondition(int cell, float cond)
{
    if (cell >= COND_PERCEPTION && cell <= COND_LEFTMOBILITY)
    {
        if (this->cond[cell] == cond)
            return false;

        this->cond[cell] = cond;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Player %s was set to %f (ref: %s)", cell == 0 ? (char*) "PerceptionCondition" : cell == 1 ? (char*) "EnduranceCondition" : cell == 2 ? (char*) "LeftAttackCondition" : cell == 3 ? (char*) "RightAttackCondition" : cell == 4 ? (char*) "LeftMobilityCondition" : (char*) "RightMobilityCondition", this->cond[cell], this->refID.c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }

    return false;
}

bool Player::SetPlayerDead(bool dead)
{
    if (!nowrite[SKIPFLAG_GETDEAD])
    {
        if (this->dead == dead)
            return false;

        this->dead = dead;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Player dead state was set to %d (ref: %s)", (int) this->dead, this->refID.c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }

    return false;
}

bool Player::SetPlayerAlerted(bool alerted)
{
    if (this->alerted == alerted)
        return false;

    this->alerted = alerted;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player alerted state was set to %d (ref: %s)", (int) this->alerted, this->refID.c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

bool Player::SetPlayerMoving(int moving)
{
    if (moving >= MOV_IDLE && moving <= MOV_RIGHT)
    {
        if (this->moving == moving)
            return false;

        this->moving = moving;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Player running animation was set to %s (ref: %s)", moving == 0 ? (char*) "Idle" : moving == 1 ? (char*) "FastForward" : moving == 2 ? (char*) "FastBackward" : moving == 3 ? (char*) "FastLeft" : moving == 4 ? (char*) "FastRight" : moving == 5 ? (char*) "Forward" : moving == 6 ? (char*) "Backward" : moving == 7 ? (char*) "Left" : (char*) "Right", this->refID.c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }

    return false;
}

bool Player::SetPlayerRefID(string refID)
{
    if (this->refID == refID)
        return false;

    this->refID = refID;
    map<RakNetGUID, string>::iterator it;
    it = players.find(this->guid);
    it->second = refID;
    playersrefs.insert(pair<string, Player*>(refID, this));

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player refID was set to %s (name: %s)", this->refID.c_str(), this->GetPlayerName().c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

bool Player::SetPlayerEnabled(bool enabled)
{
    if (this->enabled == enabled)
        return false;

    this->enabled = enabled;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        ZeroMemory(text, sizeof(text));

        sprintf(text, "Player enabled state was set to %d (ref: %s)", (int) this->enabled, this->refID.c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

void Player::SetPlayerInventory(Inventory* inv)
{
    if (inventory != NULL)
        delete inventory;
    inventory = inv;
}

bool Player::ToggleNoOverride(int skipflag, bool toggle)
{
    if (skipflag >= 0 && skipflag < MAX_SKIP_FLAGS)
    {
        if (nowrite[skipflag] == toggle)
            return false;

        nowrite[skipflag] = toggle;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Player no-override flag %d was set to %d (ref: %s)", skipflag, (int) toggle, this->refID.c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }

    return false;
}

bool Player::GetPlayerOverrideFlag(int skipflag)
{
    if (skipflag >= 0 && skipflag < MAX_SKIP_FLAGS)
    {
        return nowrite[skipflag];
    }

    return false;
}

bool Player::IsPlayerNearPoint(float X, float Y, float Z, float R)
{
    return (sqrt((abs(pos[0] - X) * abs(pos[0] - X)) + (abs(pos[1] - Y) * abs(pos[1] - Y)) + (abs(pos[2] - Z) * abs(pos[2] - Z))) <= R);
}

bool Player::IsCoordinateInRange(int cell, float XYZ, float R)
{
    if (cell >= X_AXIS && cell <= Z_AXIS)
        return (pos[cell] > (XYZ - R) && pos[cell] < (XYZ + R));
    return false;
}
