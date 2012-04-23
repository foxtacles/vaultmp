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

Object::Object(const pDefault* packet) : Object(PacketFactory::ExtractReference(packet), PacketFactory::ExtractBase(packet))
{
	NetworkID id;
	unsigned int refID, baseID;
	char name[MAX_PLAYER_NAME + 1];
	ZeroMemory(name, sizeof(name));
	double X, Y, Z, aX, aY, aZ;
	unsigned int cell;
	bool enabled;

	PacketFactory::Access(packet, &id, &refID, &baseID, name, &X, &Y, &Z, &aX, &aY, &aZ, &cell, &enabled);

	// NetworkID set by GameFactory, refID and baseID in constructor
	this->SetName(string(name));
	this->SetNetworkPos(Axis_X, X);
	this->SetNetworkPos(Axis_Y, Y);
	this->SetNetworkPos(Axis_Z, Z);
	this->SetAngle(Axis_X, aX);
	this->SetAngle(Axis_Y, aY);
	this->SetAngle(Axis_Z, aZ);
	this->SetNetworkCell(cell);
	this->SetEnabled(enabled);
}

Object::Object(pDefault* packet) : Object(reinterpret_cast<const pDefault*>(packet))
{
	PacketFactory::FreePacket(packet);
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
	return object_Name.get();
}

double Object::GetGamePos(unsigned char axis) const
{
	return SAFE_FIND(object_Game_Pos, axis)->second.get();
}

double Object::GetNetworkPos(unsigned char axis) const
{
	return SAFE_FIND(object_Network_Pos, axis)->second.get();
}

double Object::GetAngle(unsigned char axis) const
{
	return SAFE_FIND(object_Angle, axis)->second.get();
}

bool Object::GetEnabled() const
{
	return state_Enabled.get();
}

unsigned int Object::GetGameCell() const
{
	return cell_Game.get();
}

unsigned int Object::GetNetworkCell() const
{
	return cell_Network.get();
}

Lockable* Object::SetName(string name)
{
	if (this->object_Name.get() == name)
		return NULL;

	if (!this->object_Name.set(name))
		return NULL;

#ifdef VAULTMP_DEBUG

	if (debug != NULL)
		debug->PrintFormat("Object name was set to %s (ref: %08X)", true, this->object_Name.get().c_str(), this->GetReference());

#endif

	return &this->object_Name;
}

Lockable* Object::SetGamePos(unsigned char axis, double pos)
{
	Value<double>& data = SAFE_FIND(this->object_Game_Pos, axis)->second;

	if (Utils::DoubleCompare(data.get(), pos, 0.01))
		return NULL;

	if ((pos != 2048.0 && pos != 128.0 && pos != 0.0))
	{
		if (!data.set(pos))
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

	if (Utils::DoubleCompare(data.get(), pos, 0.01))
		return NULL;

	if ((pos != 2048.0 && pos != 128.0 && pos != 0.0))
	{
		if (!data.set(pos))
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

	if (Utils::DoubleCompare(data.get(), angle, 0.01))
		return NULL;

	if ((angle != 2048.0 && angle != 128.0 && angle != 0.0))
	{
		if (!data.set(angle))
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
	if (this->state_Enabled.get() == state)
		return NULL;

	if (!this->state_Enabled.set(state))
		return NULL;

#ifdef VAULTMP_DEBUG

	if (debug != NULL)
		debug->PrintFormat("Object enabled state was set to %d (ref: %08X)", true, (int) state, this->GetReference());

#endif

	return &this->state_Enabled;
}

Lockable* Object::SetGameCell(unsigned int cell)
{
	if (this->cell_Game.get() == cell)
		return NULL;

	if (cell != 0x00000000)
	{
		if (!this->cell_Game.set(cell))
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
	if (this->cell_Network.get() == cell)
		return NULL;

	if (cell != 0x00000000)
	{
		if (!this->cell_Network.set(cell))
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

pDefault* Object::toPacket()
{
	pDefault* packet = PacketFactory::CreatePacket(ID_OBJECT_NEW, this->GetNetworkID(), this->GetReference(), this->GetBase(),
												   this->GetName().c_str(), this->GetNetworkPos(Values::Axis_X), this->GetNetworkPos(Values::Axis_Y), this->GetNetworkPos(Values::Axis_Z),
												   this->GetAngle(Values::Axis_X), this->GetAngle(Values::Axis_Y), this->GetAngle(Values::Axis_Z), this->GetNetworkCell(), this->GetEnabled());

	return packet;
}
