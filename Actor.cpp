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

vector<Actor*> Actor::GetActorList()
{
    vector<Actor*> actorlist;
    vector<Reference*>::iterator it;
    vector<Reference*> instances = GameFactory::GetObjectTypes(ID_ACTOR);

    for (it = instances.begin(); it != instances.end(); ++it)
        actorlist.push_back((Actor*) *it);

    return actorlist;
}

vector<string> Actor::GetRefs(bool enabled, bool enabled_disabled)
{
    vector<string> result;
    vector<Actor*>::iterator it;
    vector<Actor*> actorlist = GetActorList();

    for (it = actorlist.begin(); it != actorlist.end(); ++it)
    {
        unsigned int refID = (*it)->GetReference();

        if (refID != 0x00 && (!enabled_disabled || ((*it)->GetEnabled() == enabled)))
            result.push_back(Utils::LongToHex(refID));

        GameFactory::LeaveReference(*it);
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

double Actor::GetActorValue(unsigned char index) const
{
    return SAFE_FIND(actor_Values, index)->second.Get();
}

double Actor::GetActorBaseValue(unsigned char index) const
{
    return SAFE_FIND(actor_BaseValues, index)->second.Get();
}

unsigned char Actor::GetActorRunningAnimation() const
{
    return anim_Running.Get();
}

bool Actor::GetActorAlerted() const
{
    return state_Alerted.Get();
}

bool Actor::GetActorDead() const
{
    return state_Dead.Get();
}

Lockable* Actor::SetActorValue(unsigned char index, double value)
{
    Value<double>& data = SAFE_FIND(this->actor_Values, index)->second;

    if (Utils::DoubleCompare(data.Get(), value, 0.01))
        return NULL;

    if (!data.Set(value))
        return NULL;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor value %s was set to %f (ref: %08X)", true, API::RetrieveValue_Reverse(index).c_str(), (float) value, this->GetReference());
#endif

    return &data;
}

Lockable* Actor::SetActorBaseValue(unsigned char index, double value)
{
    Value<double>& data = SAFE_FIND(this->actor_BaseValues, index)->second;

    if (Utils::DoubleCompare(data.Get(), value, 0.01))
        return NULL;

    if (!data.Set(value))
        return NULL;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor base value %s was set to %f (ref: %08X)", true, API::RetrieveValue_Reverse(index).c_str(), (float) value, this->GetReference());
#endif

    return &data;
}

Lockable* Actor::SetActorRunningAnimation(unsigned char index)
{
    string anim = API::RetrieveAnim_Reverse(index);
    if (anim.empty())
        throw VaultException("Value %02X not defined in database", index);

    if (anim_Running.Get() == index)
        return NULL;

    if (!anim_Running.Set(index))
        return NULL;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor running animation was set to %s (ref: %08X)", true,  anim.c_str(), this->GetReference());
#endif

    return &anim_Running;
}

Lockable* Actor::SetActorAlerted(bool state)
{
    if (this->state_Alerted.Get() == state)
        return NULL;

    if (!this->state_Alerted.Set(state))
        return NULL;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor alerted state was set to %d (ref: %08X)", true, (int) state, this->GetReference());
#endif

    return &this->state_Alerted;
}

Lockable* Actor::SetActorDead(bool state)
{
    if (this->state_Dead.Get() == state)
        return NULL;

    if (!this->state_Dead.Set(state))
        return NULL;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Actor dead state was set to %d (ref: %08X)", true, (int) state, this->GetReference());
#endif

    return &this->state_Dead;
}
