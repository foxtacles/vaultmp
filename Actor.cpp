#include "Actor.h"

map<string, Actor*> Actor::actorlist;
bool Actor::initialized = false;

Parameter Actor::Param_EnabledActors = Parameter(vector<string>(), &Actor::GetEnabledRefs);
Parameter Actor::Param_AllActors = Parameter(vector<string>(), &Actor::GetAllRefs);

#ifdef VAULTMP_DEBUG
Debug* Actor::debug;
#endif

Actor::Actor(string base)
{
    Startup("", base);
}

Actor::Actor(string ref, string base)
{
    actorlist.insert(pair<string, Actor*>(ref, this));
    Startup(ref, base);
}

Actor::~Actor()
{
    actorlist.erase(this->GetReference());

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Actor object destroyed (ref: %s)", this->GetReference().c_str());
        debug->Print(text, true);
    }
#endif
}

#ifdef VAULTMP_DEBUG
void Actor::SetDebugHandler(Debug* debug)
{
    Actor::debug = debug;

    if (debug != NULL)
        debug->Print((char*) "Attached debug handler to Actor class", true);
}
#endif

void Actor::Startup(string ref, string base)
{
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
    name = "Actor";
    enabled = true;

    SetReference(ref);
    SetBase(base);

    for (int i = 0; i < MAX_SKIP_FLAGS; i++)
        nowrite[i] = false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "New actor object created (ref: %s)", ref.c_str());
        debug->Print(text, true);
    }
#endif
}

Actor* Actor::GetActorFromRefID(string refID)
{
    map<string, Actor*>::iterator it;

    for (it = actorlist.begin(); it != actorlist.end(); ++it)
    {
        if (it->second->GetReference().compare(refID) == 0)
            return it->second;
    }

    return NULL;
}

vector<string> Actor::GetRefs(string cmd, bool enabled)
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
    map<string, Actor*>::iterator it;

    for (it = actorlist.begin(); it != actorlist.end(); ++it)
    {
        string refID = it->second->GetReference();

        if (!refID.empty() && (!enabled || it->second->GetActorEnabled()) && !it->second->GetActorOverrideFlag(skipflag))
            result.push_back(refID);
    }

    return result;
}

vector<string> Actor::GetAllRefs(string cmd)
{
    return GetRefs(cmd, false);
}

vector<string> Actor::GetEnabledRefs(string cmd)
{
    return GetRefs(cmd, true);
}

void Actor::Initialize()
{
    if (!initialized)
    {
        initialized = true;
    }
}

void Actor::DestroyInstances()
{
    if (initialized)
    {
        int size = actorlist.size();

        if (size != 0)
        {
            map<string, Actor*>::iterator it;
            int i = 0;
            Actor* pActors[size];

            for (it = actorlist.begin(); it != actorlist.end(); ++it, i++)
                pActors[i] = it->second;

            for (i = 0; i < size; i++)
                delete pActors[i];
        }


#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->Print((char*) "All actor instances destroyed", true);
#endif

        actorlist.clear();

        initialized = false;
    }
}

string Actor::GetActorName()
{
    return name;
}

float Actor::GetActorPos(int axis)
{
    float value = (axis >= X_AXIS && axis <= Z_AXIS) ? pos[axis] : 0.00;
    return value;
}

float Actor::GetActorAngle()
{
    return angle;
}

int Actor::GetActorGameCell()
{
    return gcell;
}

int Actor::GetActorNetworkCell()
{
    return ncell;
}

float Actor::GetActorHealth()
{
    return health;
}

float Actor::GetActorBaseHealth()
{
    return baseHealth;
}

float Actor::GetActorCondition(int cond)
{
    float value = (cond >= COND_PERCEPTION && cond <= COND_LEFTMOBILITY) ? this->cond[cond] : 0.00;
    return value;
}

bool Actor::IsActorDead()
{
    return dead;
}

bool Actor::IsActorAlerted()
{
    return alerted;
}

int Actor::GetActorMoving()
{
    return moving;
}

bool Actor::GetActorEnabled()
{
    return enabled;
}

Parameter Actor::GetActorRefParam()
{
    vector<string> self;
    if (!GetReference().empty()) self.push_back(GetReference());
    Parameter Param_Self = Parameter(self, &Data::EmptyVector);
    return Param_Self;
}

Parameter Actor::GetActorNameParam()
{
    vector<string> self;
    self.push_back(name);
    Parameter Param_Self = Parameter(self, &Data::EmptyVector);
    return Param_Self;
}

pActorUpdate Actor::GetActorUpdateStruct()
{
    pActorUpdate data;

    data.type = ID_PLAYER_UPDATE;
    data.X = pos[X_AXIS];
    data.Y = pos[Y_AXIS];
    data.Z = pos[Z_AXIS];
    data.A = angle;
    data.alerted = alerted;
    data.moving = moving;

    return data;
}

pActorStateUpdate Actor::GetActorStateUpdateStruct()
{
    pActorStateUpdate data;

    data.type = ID_PLAYER_STATE_UPDATE;
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

pActorCellUpdate Actor::GetActorCellUpdateStruct()
{
    pActorCellUpdate data;

    data.type = ID_PLAYER_CELL_UPDATE;
    data.cell = gcell;

    return data;
}

pActorItemUpdate Actor::GetActorItemUpdateStruct(Item* item)
{
    pActorItemUpdate data;

    data.type = ID_PLAYER_ITEM_UPDATE;
    data.hidden = false;
    data.item = (*item);

    strcpy(data.baseID, item->item->first);

    return data;
}

bool Actor::UpdateActorUpdateStruct(pActorUpdate* data)
{
    pActorUpdate actor = this->GetActorUpdateStruct();
    actor.guid = data->guid;

    if (actor.type != data->type ||
            actor.X != data->X ||
            actor.Y != data->Y ||
            actor.Z != data->Z ||
            actor.A != data->A ||
            actor.alerted != data->alerted ||
            actor.moving != data->moving)
    {
        (*data) = actor;
        return true;
    }

    return false;
}

bool Actor::UpdateActorStateUpdateStruct(pActorStateUpdate* data)
{
    pActorStateUpdate actor = this->GetActorStateUpdateStruct();
    actor.guid = data->guid;

    if (actor.type != data->type ||
            actor.health != data->health ||
            actor.baseHealth != data->baseHealth ||
            actor.conds[COND_PERCEPTION] != data->conds[COND_PERCEPTION] ||
            actor.conds[COND_ENDURANCE] != data->conds[COND_ENDURANCE] ||
            actor.conds[COND_LEFTATTACK] != data->conds[COND_LEFTATTACK] ||
            actor.conds[COND_RIGHTATTACK] != data->conds[COND_RIGHTATTACK] ||
            actor.conds[COND_LEFTMOBILITY] != data->conds[COND_LEFTMOBILITY] ||
            actor.conds[COND_RIGHTMOBILITY] != data->conds[COND_RIGHTMOBILITY] ||
            actor.dead != data->dead)
    {
        (*data) = actor;
        return true;
    }

    return false;
}

bool Actor::UpdateActorCellUpdateStruct(pActorCellUpdate* data)
{
    pActorCellUpdate actor = this->GetActorCellUpdateStruct();
    actor.guid = data->guid;

    if (actor.type != data->type ||
            actor.cell != data->cell)
    {
        (*data) = actor;
        return true;
    }

    return false;
}

bool Actor::UpdateActorItemUpdateStruct(list<pActorItemUpdate>* items, Inventory* inv)
{
    if (inv == NULL || items == NULL)
        return false;

    this->StartSession();
    Inventory* invcopy = this->Copy();
    this->EndSession();

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
        items->push_back(GetActorItemUpdateStruct(*it));

    invcopy->Copy(inv);
    delete invcopy;

    return true;
}

bool Actor::SetActorName(string name)
{
    if (this->name == name)
        return false;

    this->name = name;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Actor name was set to %s (ref: %s)", this->name.c_str(), this->GetReference().c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

bool Actor::SetActorPos(int axis, float value)
{
    if (axis >= X_AXIS && axis <= Z_AXIS && !nowrite[SKIPFLAG_GETPOS])
    {
        if (this->pos[axis] == value)
            return false;

        if ((value != 2048.0 && value != 128.0 && value != 0.0))
        {
            this->pos[axis] = value;

#ifdef VAULTMP_DEBUG
            if (debug != NULL)
            {
                char text[128];
                snprintf(text, sizeof(text), "Actor %c-coordinate was set to %f (ref: %s)", axis == 0 ? 'X' : axis == 1 ? 'Y' : 'Z', this->pos[axis], this->GetReference().c_str());
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
                snprintf(text, sizeof(text), "Actor %c-coordinate was NOT set to %f (INVALID) (ref: %s)", axis == 0 ? 'X' : axis == 1 ? 'Y' : 'Z', value, this->GetReference().c_str());
                debug->Print(text, true);
            }
#endif

            return false;
        }
    }

    return false;
}

bool Actor::SetActorAngle(float angle)
{
    if (this->angle == angle)
        return false;

    this->angle = angle;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Actor Z-angle was set to %f (ref: %s)", this->angle, this->GetReference().c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

bool Actor::SetActorGameCell(int cell)
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
                snprintf(text, sizeof(text), "Actor game cell was set to %x (ref: %s)", this->gcell, this->GetReference().c_str());
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
                snprintf(text, sizeof(text), "Actor game cell was NOT set to %x (INVALID) (ref: %s)", cell, this->GetReference().c_str());
                debug->Print(text, true);
            }
#endif

            return false;
        }
    }

    return false;
}

bool Actor::SetActorNetworkCell(int cell)
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
            snprintf(text, sizeof(text), "Actor network cell was set to %x (ref: %s)", this->ncell, this->GetReference().c_str());
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
            snprintf(text, sizeof(text), "Actor network cell was NOT set to %x (INVALID) (ref: %s)", cell, this->GetReference().c_str());
            debug->Print(text, true);
        }
#endif

        return false;
    }

    return false;
}

bool Actor::SetActorHealth(float health)
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
            snprintf(text, sizeof(text), "Actor health was set to %f (ref: %s)", this->health, this->GetReference().c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }

    return false;
}

bool Actor::SetActorBaseHealth(float baseHealth)
{
    if (this->baseHealth == baseHealth)
        return false;

    this->baseHealth = baseHealth;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Actor base health was set to %f (ref: %s)", this->baseHealth, this->GetReference().c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

bool Actor::SetActorCondition(int cond, float value)
{
    if (cond >= COND_PERCEPTION && cond <= COND_LEFTMOBILITY)
    {
        if (this->cond[cond] == value)
            return false;

        this->cond[cond] = value;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
        {
            char text[128];
            snprintf(text, sizeof(text), "Actor %s was set to %f (ref: %s)", cond == 0 ? (char*) "PerceptionCondition" : cond == 1 ? (char*) "EnduranceCondition" : cond == 2 ? (char*) "LeftAttackCondition" : cond == 3 ? (char*) "RightAttackCondition" : cond == 4 ? (char*) "LeftMobilityCondition" : (char*) "RightMobilityCondition", this->cond[cond], this->GetReference().c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }

    return false;
}

bool Actor::SetActorDead(bool dead)
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
            snprintf(text, sizeof(text), "Actor dead state was set to %d (ref: %s)", (int) this->dead, this->GetReference().c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }

    return false;
}

bool Actor::SetActorAlerted(bool alerted)
{
    if (this->alerted == alerted)
        return false;

    this->alerted = alerted;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Actor alerted state was set to %d (ref: %s)", (int) this->alerted, this->GetReference().c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

bool Actor::SetActorMoving(int moving)
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
            snprintf(text, sizeof(text), "Actor running animation was set to %s (ref: %s)", moving == 0 ? (char*) "Idle" : moving == 1 ? (char*) "FastForward" : moving == 2 ? (char*) "FastBackward" : moving == 3 ? (char*) "FastLeft" : moving == 4 ? (char*) "FastRight" : moving == 5 ? (char*) "Forward" : moving == 6 ? (char*) "Backward" : moving == 7 ? (char*) "Left" : (char*) "Right", this->GetReference().c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }

    return false;
}

bool Actor::SetActorEnabled(bool enabled)
{
    if (this->enabled == enabled)
        return false;

    this->enabled = enabled;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
    {
        char text[128];
        snprintf(text, sizeof(text), "Actor enabled state was set to %d (ref: %s)", (int) this->enabled, this->GetReference().c_str());
        debug->Print(text, true);
    }
#endif

    return true;
}

bool Actor::ToggleNoOverride(int skipflag, bool toggle)
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
            snprintf(text, sizeof(text), "Actor no-override flag %d was set to %d (ref: %s)", skipflag, (int) toggle, this->GetReference().c_str());
            debug->Print(text, true);
        }
#endif

        return true;
    }

    return false;
}

bool Actor::GetActorOverrideFlag(int skipflag)
{
    if (skipflag >= 0 && skipflag < MAX_SKIP_FLAGS)
    {
        return nowrite[skipflag];
    }

    return false;
}

bool Actor::IsActorNearPoint(float X, float Y, float Z, float R)
{
    return (sqrt((abs(pos[0] - X) * abs(pos[0] - X)) + (abs(pos[1] - Y) * abs(pos[1] - Y)) + (abs(pos[2] - Z) * abs(pos[2] - Z))) <= R);
}

bool Actor::IsCoordinateInRange(int axis, float value, float R)
{
    if (axis >= X_AXIS && axis <= Z_AXIS)
        return (pos[axis] > (value - R) && pos[axis] < (value + R));
    return false;
}
