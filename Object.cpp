#include "Object.h"

const Parameter Object::Param_Axis = Parameter(vector<string>(), &API::RetrieveAllAxis_Reverse);

#ifdef VAULTMP_DEBUG
Debug* Object::debug;
#endif

#ifdef VAULTMP_DEBUG
void Object::SetDebugHandler(Debug* debug)
{
    Object::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Object class", true);
}
#endif

Object::Object(unsigned int refID, unsigned int baseID) : Reference(refID, baseID)
{
    vector<unsigned char>::iterator it;
    vector<unsigned char> data = API::RetrieveAllAxis();

    for (it = data.begin(); it != data.end(); ++it)
    {
        object_Pos.insert(pair<unsigned char, Value<double> >(*it, Value<double>()));
        object_Angle.insert(pair<unsigned char, Value<double> >(*it, Value<double>()));
    }
}

Object::~Object()
{

}

string Object::GetName()
{
    return object_Name.Get();
}

double Object::GetPos(unsigned char axis)
{
    return SAFE_FIND(object_Pos, axis)->second.Get();
}

double Object::GetAngle(unsigned char axis)
{
    return SAFE_FIND(object_Angle, axis)->second.Get();
}

bool Object::GetEnabled()
{
    return state_Enabled.Get();
}

bool Object::SetName(string name)
{
    if (this->object_Name.Get() == name)
        return false;

    if (!this->object_Name.Set(name))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Object name was set to %s (ref: %08X)", true, this->object_Name.Get().c_str(), this->GetReference());
#endif

    return true;
}

bool Object::SetPos(unsigned char axis, double pos)
{
    if (SAFE_FIND(this->object_Pos, axis)->second.Get() == pos)
        return false;

    if ((pos != 2048.0 && pos != 128.0 && pos != 0.0))
    {
        if (!SAFE_FIND(this->object_Pos, axis)->second.Set(pos))
            return false;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Object %s-pos was set to %Lf (ref: %08X)", true, API::RetrieveAxis_Reverse(axis).c_str(), pos, this->GetReference());
#endif
        return true;
    }

    return false;
}

bool Object::SetAngle(unsigned char axis, double angle)
{
    if (SAFE_FIND(this->object_Angle, axis)->second.Get() == angle)
        return false;

    if ((angle != 2048.0 && angle != 128.0 && angle != 0.0))
    {
        if (!SAFE_FIND(this->object_Angle, axis)->second.Set(angle))
            return false;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Object %s-angle was set to %Lf (ref: %08X)", true, API::RetrieveAxis_Reverse(axis).c_str(), angle, this->GetReference());
#endif
        return true;
    }

    return false;
}

bool Object::SetEnabled(bool state)
{
    if (this->state_Enabled.Get() == state)
        return false;

    if (!this->state_Enabled.Set(state))
        return false;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Object enabled state was set to %d (ref: %08X)", true, (int) state, this->GetReference());
#endif

    return true;
}
