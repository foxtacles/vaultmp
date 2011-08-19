#include "Actor.h"

using namespace Values;

map<unsigned int, Actor*> Actor::actorlist;
map<unsigned char, unsigned int> Actor::axis;
map<unsigned char, unsigned int> Actor::values;
map<unsigned char, unsigned int> Actor::anims;
bool Actor::initialized = false;

Parameter Actor::Param_EnabledActors = Parameter(vector<string>(), &Actor::GetEnabledRefs);
Parameter Actor::Param_DisabledActors = Parameter(vector<string>(), &Actor::GetDisabledRefs);
Parameter Actor::Param_AllActors = Parameter(vector<string>(), &Actor::GetAllRefs);

#ifdef VAULTMP_DEBUG
Debug* Actor::debug;
#endif

Actor::Actor(unsigned int baseID)
{
    Startup(0x00, baseID);
}

Actor::Actor(unsigned int refID, unsigned int baseID)
{
    actorlist.insert(pair<unsigned int, Actor*>(refID, this));
    Startup(refID, baseID);
}

Actor::~Actor()
{
    actorlist.erase(this->GetReference());

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor object destroyed (ref: %08X)", true, this->GetReference());
#endif
}

#ifdef VAULTMP_DEBUG
void Actor::SetDebugHandler(Debug* debug)
{
    Actor::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Actor class", true);
}
#endif

void Actor::Startup(unsigned int refID, unsigned int baseID)
{
    map<unsigned char, unsigned int>::iterator it;

    for (it = axis.begin(); it != axis.end(); ++it)
    {
        actor_Pos.push_back(Value<double>(it->first));
        actor_Angle.push_back(Value<double>(it->first));
    }

    for (it = values.begin(); it != values.end(); ++it)
    {
        actor_Values.push_back(Value<double>(it->first));
        actor_BaseValues.push_back(Value<double>(it->first));
    }

    for (it = anims.begin(); it != anims.end(); ++it)
        actor_Animations.push_back(Value<bool>(it->first));

    SetReference(refID);
    SetBase(baseID);

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("New actor object created (ref: %08X)", true, refID);
#endif
}

Actor* Actor::GetActorFromRefID(unsigned int refID)
{
    map<unsigned int, Actor*>::iterator it;

    for (it = actorlist.begin(); it != actorlist.end(); ++it)
    {
        if (it->second->GetReference() == refID)
            return it->second;
    }

    return NULL;
}

vector<string> Actor::GetRefs(bool enabled, bool enabled_disabled)
{
    vector<string> result;
    map<unsigned int, Actor*>::iterator it;

    for (it = actorlist.begin(); it != actorlist.end(); ++it)
    {
        unsigned int refID = it->second->GetReference();

        if (refID != 0x00 && (!enabled_disabled || (it->second->GetActorEnabled() == enabled)))
            result.push_back(Utils::LongToHex(refID));
    }

    return result;
}

vector<string> Actor::GetEnabledRefs()
{
    return GetRefs(true, true);
}

vector<string> Actor::GetDisabledRefs()
{
    return GetRefs(false, true);
}

vector<string> Actor::GetAllRefs()
{
    return GetRefs();
}

void Actor::Initialize()
{
    if (!initialized)
    {
        vector<unsigned char>::iterator it;
        vector<unsigned char> data = API::RetrieveAllAxis();

        for (it = data.begin(); it != data.end(); ++it)
            axis.insert(pair<unsigned char, unsigned int>(*it, axis.size()));

        data = API::RetrieveAllValues();

        for (it = data.begin(); it != data.end(); ++it)
            values.insert(pair<unsigned char, unsigned int>(*it, values.size()));

        data = API::RetrieveAllAnims();

        for (it = data.begin(); it != data.end(); ++it)
            anims.insert(pair<unsigned char, unsigned int>(*it, anims.size()));

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
            map<unsigned int, Actor*>::iterator it;
            int i = 0;
            Actor* pActors[size];

            for (it = actorlist.begin(); it != actorlist.end(); ++it, i++)
                pActors[i] = it->second;

            for (i = 0; i < size; i++)
                delete pActors[i];
        }


#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->Print("All actor instances destroyed", true);
#endif

        actorlist.clear();
        axis.clear();
        values.clear();
        anims.clear();

        initialized = false;
    }
}

unsigned int Actor::LookupIndex(map<unsigned char, unsigned int>& indices, unsigned char index)
{
    map<unsigned char, unsigned int>::iterator it;
    it = indices.find(index);

    if (it == indices.end())
        throw VaultException("Error in Actor class; identifier %02X not found", (unsigned int) index);

    return it->second;
}

string Actor::GetActorName()
{
    return name.Get();
}

double Actor::GetActorPos(unsigned char axis)
{
    return actor_Pos.at(LookupIndex(Actor::axis, axis)).Get();
}

double Actor::GetActorAngle(unsigned char axis)
{
    return actor_Angle.at(LookupIndex(Actor::axis, axis)).Get();
}

unsigned int Actor::GetActorGameCell()
{
    return cell_Game.Get();
}

unsigned int Actor::GetActorNetworkCell()
{
    return cell_Network.Get();
}

double Actor::GetActorValue(unsigned char index)
{
    return actor_Values.at(LookupIndex(Actor::values, index)).Get();
}

double Actor::GetActorBaseValue(unsigned char index)
{
    return actor_BaseValues.at(LookupIndex(Actor::values, index)).Get();
}

bool Actor::IsActorAnimationActive(unsigned char anim)
{
    return actor_Animations.at(LookupIndex(Actor::anims, anim)).Get();
}

bool Actor::GetActorEnabled()
{
    return state_Enabled.Get();
}

bool Actor::GetActorDead()
{
    return state_Dead.Get();
}

Parameter Actor::GetActorRefParam()
{
    vector<string> self;
    if (GetReference() != 0x00) self.push_back(Utils::LongToHex(GetReference()));
    Parameter Param_Self = Parameter(self, &Data::EmptyVector);
    return Param_Self;
}

Parameter Actor::GetActorNameParam()
{
    vector<string> self;
    self.push_back(GetActorName());
    Parameter Param_Self = Parameter(self, &Data::EmptyVector);
    return Param_Self;
}

bool Actor::SetActorName(string name)
{
    if (this->name.Get() == name)
        return false;

    if (!this->name.Set(name))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor name was set to %s (ref: %08X)", true, this->name.Get().c_str(), this->GetReference());
#endif

    return true;
}

bool Actor::SetActorPos(unsigned char axis, double pos)
{
    unsigned int idx = LookupIndex(Actor::axis, axis);

    if (this->actor_Pos.at(idx).Get() == pos)
        return false;

    if ((pos != 2048.0 && pos != 128.0 && pos != 0.0))
    {
        if (!this->actor_Pos.at(idx).Set(pos))
            return false;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Actor %s-pos was set to %Lf (ref: %08X)", true, API::RetrieveAxis_Reverse(axis).c_str(), pos, this->GetReference());
#endif
        return true;
    }

    return false;
}

bool Actor::SetActorAngle(unsigned char axis, double angle)
{
    unsigned int idx = LookupIndex(Actor::axis, axis);

    if (this->actor_Angle.at(idx).Get() == angle)
        return false;

    if ((angle != 2048.0 && angle != 128.0 && angle != 0.0))
    {
        if (!this->actor_Angle.at(idx).Set(angle))
            return false;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Actor %s-angle was set to %Lf (ref: %08X)", true, API::RetrieveAxis_Reverse(axis).c_str(), angle, this->GetReference());
#endif
        return true;
    }

    return false;
}

bool Actor::SetActorGameCell(unsigned int cell)
{
    if (this->cell_Game.Get() == cell)
        return false;

    if (cell != 0x00)
    {
        if (!this->cell_Game.Set(cell))
            return false;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Actor game cell was set to %08X (ref: %08X)", true, cell, this->GetReference());
#endif
        return true;
    }

    return false;
}

bool Actor::SetActorNetworkCell(unsigned int cell)
{
    if (this->cell_Network.Get() == cell)
        return false;

    if (cell != 0x00)
    {
        if (!this->cell_Network.Set(cell))
            return false;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Actor network cell was set to %08X (ref: %08X)", cell, this->GetReference());
#endif
        return true;
    }

    return false;
}

bool Actor::SetActorValue(unsigned char index, double value)
{
    unsigned int idx = LookupIndex(Actor::values, index);

    if (this->actor_Values.at(idx).Get() == value)
        return false;

    if (!this->actor_Values.at(idx).Set(value))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor value %s was set to %Lf (ref: %08X)", true, API::RetrieveValue_Reverse(index).c_str(), value, this->GetReference());
#endif

    return true;
}

bool Actor::SetActorBaseValue(unsigned char index, double value)
{
    unsigned int idx = LookupIndex(Actor::values, index);

    if (this->actor_BaseValues.at(idx).Get() == value)
        return false;

    if (!this->actor_BaseValues.at(idx).Set(value))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor base value %s was set to %Lf (ref: %08X)", true, API::RetrieveValue_Reverse(index).c_str(), value, this->GetReference());
#endif

    return true;
}

bool Actor::SetActorAnimation(unsigned char anim, bool state)
{
    unsigned int idx = LookupIndex(Actor::anims, anim);

    if (this->actor_Animations.at(idx).Get() == state)
        return false;

    if (!this->actor_Animations.at(idx).Set(state))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor animation %s was set to %d (ref: %08X)", true,  API::RetrieveAnim_Reverse(anim).c_str(), (int) state, this->GetReference());
#endif

    return true;
}

bool Actor::SetActorEnabled(bool state)
{
    if (this->state_Enabled.Get() == state)
        return false;

    if (!this->state_Enabled.Set(state))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor enabled state was set to %d (ref: %08X)", true, (int) state, this->GetReference());
#endif

    return true;
}

bool Actor::SetActorDead(bool state)
{
    if (this->state_Dead.Get() == state)
        return false;

    if (!this->state_Dead.Set(state))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor dead state was set to %d (ref: %08X)", true, (int) state, this->GetReference());
#endif

    return true;
}

bool Actor::IsActorNearPoint(double X, double Y, double Z, double R)
{
    unsigned int idx_X = LookupIndex(Actor::axis, Axis_X);
    unsigned int idx_Y = LookupIndex(Actor::axis, Axis_Y);
    unsigned int idx_Z = LookupIndex(Actor::axis, Axis_Z);

    return (sqrt((abs(actor_Pos.at(idx_X).Get() - X) * abs(actor_Pos.at(idx_X).Get() - X)) + ((actor_Pos.at(idx_Y).Get() - Y) * abs(actor_Pos.at(idx_Y).Get() - Y)) + ((actor_Pos.at(idx_Z).Get() - Z) * abs(actor_Pos.at(idx_Z).Get() - Z))) <= R);
}

bool Actor::IsCoordinateInRange(unsigned char axis, double pos, double R)
{
    unsigned int idx = LookupIndex(Actor::axis, axis);

    return (actor_Pos.at(idx).Get() > (pos - R) && actor_Pos.at(idx).Get() < (pos + R));
}
