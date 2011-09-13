#include "Object.h"

Parameter Object::param_Axis = Parameter(vector<string>(), NULL);

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
        object_Game_Pos.insert(pair<unsigned char, Value<double> >(*it, Value<double>()));
        object_Network_Pos.insert(pair<unsigned char, Value<double> >(*it, Value<double>()));
        object_Angle.insert(pair<unsigned char, Value<double> >(*it, Value<double>()));
    }
}

Object::~Object()
{

}

const Parameter& Object::Param_Axis()
{
    if (param_Axis.first.empty())
        param_Axis.first = API::RetrieveAllAxis_Reverse();

    return param_Axis;
}

string Object::GetName() const
{
    return object_Name.Get();
}

double Object::GetGamePos(unsigned char axis) const
{
    return SAFE_FIND(object_Game_Pos, axis)->second.Get();
}

double Object::GetNetworkPos(unsigned char axis) const
{
    return SAFE_FIND(object_Network_Pos, axis)->second.Get();
}

double Object::GetAngle(unsigned char axis) const
{
    return SAFE_FIND(object_Angle, axis)->second.Get();
}

bool Object::GetEnabled() const
{
    return state_Enabled.Get();
}

unsigned int Object::GetGameCell() const
{
    return cell_Game.Get();
}

unsigned int Object::GetNetworkCell() const
{
    return cell_Network.Get();
}

Lockable* Object::SetName(string name)
{
    if (this->object_Name.Get() == name)
        return NULL;

    if (!this->object_Name.Set(name))
        return NULL;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Object name was set to %s (ref: %08X)", true, this->object_Name.Get().c_str(), this->GetReference());
#endif

    return &this->object_Name;
}

Lockable* Object::SetGamePos(unsigned char axis, double pos)
{
    Value<double>& data = SAFE_FIND(this->object_Game_Pos, axis)->second;

    if (Utils::DoubleCompare(data.Get(), pos, 0.01))
        return NULL;

    if ((pos != 2048.0 && pos != 128.0 && pos != 0.0))
    {
        if (!data.Set(pos))
            return NULL;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Object game %s-pos was set to %f (ref: %08X)", true, API::RetrieveAxis_Reverse(axis).c_str(), (float) pos, this->GetReference());
#endif
        return &data;
    }

    return NULL;
}

Lockable* Object::SetNetworkPos(unsigned char axis, double pos)
{
    Value<double>& data = SAFE_FIND(this->object_Network_Pos, axis)->second;

    if (Utils::DoubleCompare(data.Get(), pos, 0.01))
        return NULL;

    if ((pos != 2048.0 && pos != 128.0 && pos != 0.0))
    {
        if (!data.Set(pos))
            return NULL;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Object network %s-pos was set to %f (ref: %08X)", true, API::RetrieveAxis_Reverse(axis).c_str(), (float) pos, this->GetReference());
#endif
        return &data;
    }

    return NULL;
}

Lockable* Object::SetAngle(unsigned char axis, double angle)
{
    Value<double>& data = SAFE_FIND(this->object_Angle, axis)->second;

    if (Utils::DoubleCompare(data.Get(), angle, 0.01))
        return NULL;

    if ((angle != 2048.0 && angle != 128.0 && angle != 0.0))
    {
        if (!data.Set(angle))
            return NULL;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Object %s-angle was set to %f (ref: %08X)", true, API::RetrieveAxis_Reverse(axis).c_str(), (float) angle, this->GetReference());
#endif
        return &data;
    }

    return NULL;
}

Lockable* Object::SetEnabled(bool state)
{
    if (this->state_Enabled.Get() == state)
        return NULL;

    if (!this->state_Enabled.Set(state))
        return NULL;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("Object enabled state was set to %d (ref: %08X)", true, (int) state, this->GetReference());
#endif

    return &this->state_Enabled;
}

Lockable* Object::SetGameCell(unsigned int cell)
{
    if (this->cell_Game.Get() == cell)
        return NULL;

    if (cell != 0x00000000)
    {
        if (!this->cell_Game.Set(cell))
            return NULL;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Object game cell was set to %08X (ref: %08X)", true, cell, this->GetReference());
#endif
        return &this->cell_Game;
    }

    return NULL;
}

Lockable* Object::SetNetworkCell(unsigned int cell)
{
    if (this->cell_Network.Get() == cell)
        return NULL;

    if (cell != 0x00000000)
    {
        if (!this->cell_Network.Set(cell))
            return NULL;

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("Object network cell was set to %08X (ref: %08X)", true, cell, this->GetReference());
#endif
        return &this->cell_Network;
    }

    return NULL;
}

bool Object::IsNearPoint(double X, double Y, double Z, double R) const
{
    return (sqrt((abs(GetGamePos(Axis_X) - X) * abs(GetGamePos(Axis_X) - X)) + ((GetGamePos(Axis_Y) - Y) * abs(GetGamePos(Axis_Y) - Y)) + ((GetGamePos(Axis_Z) - Z) * abs(GetGamePos(Axis_Z) - Z))) <= R);
}

bool Object::IsCoordinateInRange(unsigned char axis, double pos, double R) const
{
    return (GetGamePos(axis) > (pos - R) && GetGamePos(axis) < (pos + R));
}
