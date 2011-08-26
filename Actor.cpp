#include "Actor.h"

using namespace Values;

const Parameter Actor::Param_EnabledActors = Parameter(vector<string>(), &Actor::GetEnabledRefs);
const Parameter Actor::Param_DisabledActors = Parameter(vector<string>(), &Actor::GetDisabledRefs);
const Parameter Actor::Param_AllActors = Parameter(vector<string>(), &Actor::GetAllRefs);
const Parameter Actor::Param_ActorValues = Parameter(vector<string>(), &API::RetrieveAllValues_Reverse);

#ifdef VAULTMP_DEBUG
Debug* Actor::debug;
#endif

Actor::Actor(unsigned int refID, unsigned int baseID) : Container(refID, baseID)
{
    vector<unsigned char>::iterator it;
    vector<unsigned char> data = API::RetrieveAllValues();

    for (it = data.begin(); it != data.end(); ++it)
    {
        actor_Values.insert(pair<unsigned char, Value<double> >(*it, Value<double>()));
        actor_BaseValues.insert(pair<unsigned char, Value<double> >(*it, Value<double>()));
    }

    data = API::RetrieveAllAnims();

    for (it = data.begin(); it != data.end(); ++it)
        actor_Animations.insert(pair<unsigned char, Value<bool> >(*it, Value<bool>()));

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("New actor object created (ref: %08X)", true, this->GetReference());
#endif
}

Actor::~Actor()
{

}

#ifdef VAULTMP_DEBUG
void Actor::SetDebugHandler(Debug* debug)
{
    Actor::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Actor class", true);
}
#endif

list<Actor*> Actor::GetActorList()
{
    list<Actor*> actorlist;
    list<Reference*>::iterator it;
    list<Reference*> instances = GameFactory::GetObjectTypes(ID_ACTOR);

    for (it = instances.begin(); it != instances.end(); ++it)
        actorlist.push_back((Actor*) *it);

    return actorlist;
}

vector<string> Actor::GetRefs(bool enabled, bool enabled_disabled)
{
    vector<string> result;
    list<Actor*>::iterator it;
    list<Actor*> actorlist = GetActorList();

    for (it = actorlist.begin(); it != actorlist.end(); ++it)
    {
        unsigned int refID = (*it)->GetReference();

        if (refID != 0x00 && (!enabled_disabled || ((*it)->GetEnabled() == enabled)))
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
    return SAFE_FIND(actor_Values, index)->second.Get();
}

double Actor::GetActorBaseValue(unsigned char index)
{
    return SAFE_FIND(actor_BaseValues, index)->second.Get();
}

bool Actor::IsActorAnimationActive(unsigned char anim)
{
    return SAFE_FIND(actor_Animations, anim)->second.Get();
}

bool Actor::GetActorDead()
{
    return state_Dead.Get();
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
    if (SAFE_FIND(actor_Values, index)->second.Get() == value)
        return false;

    if (!SAFE_FIND(actor_Values, index)->second.Set(value))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor value %s was set to %Lf (ref: %08X)", true, API::RetrieveValue_Reverse(index).c_str(), value, this->GetReference());
#endif

    return true;
}

bool Actor::SetActorBaseValue(unsigned char index, double value)
{
    if (SAFE_FIND(actor_BaseValues, index)->second.Get() == value)
        return false;

    if (!SAFE_FIND(actor_BaseValues, index)->second.Set(value))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor base value %s was set to %Lf (ref: %08X)", true, API::RetrieveValue_Reverse(index).c_str(), value, this->GetReference());
#endif

    return true;
}

bool Actor::SetActorAnimation(unsigned char anim, bool state)
{
    if (SAFE_FIND(actor_Animations, anim)->second.Get() == state)
        return false;

    if (!SAFE_FIND(actor_Animations, anim)->second.Set(state))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor animation %s was set to %d (ref: %08X)", true,  API::RetrieveAnim_Reverse(anim).c_str(), (int) state, this->GetReference());
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
    return (sqrt((abs(GetPos(Axis_X) - X) * abs(GetPos(Axis_X) - X)) + ((GetPos(Axis_Y) - Y) * abs(GetPos(Axis_Y) - Y)) + ((GetPos(Axis_Z) - Z) * abs(GetPos(Axis_Z) - Z))) <= R);
}

bool Actor::IsCoordinateInRange(unsigned char axis, double pos, double R)
{
    return (GetPos(axis) > (pos - R) && GetPos(axis) < (pos + R));
}
