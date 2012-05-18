#include "Object.h"

Parameter Object::param_Axis = Parameter(vector<string>(), NULL);

#ifdef VAULTMP_DEBUG
Debug* Object::debug;
#endif

#ifdef VAULTMP_DEBUG
void Object::SetDebugHandler(Debug* debug)
{
	Object::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Object class", true);
}
#endif

Object::Object(unsigned int refID, unsigned int baseID) : Reference(refID, baseID)
{
	vector<unsigned char> data = API::RetrieveAllAxis();

	for (unsigned char _data : data)
	{
		object_Game_Pos.insert(pair<unsigned char, Value<double> >(_data, Value<double>()));
		object_Network_Pos.insert(pair<unsigned char, Value<double> >(_data, Value<double>()));
		object_Angle.insert(pair<unsigned char, Value<double> >(_data, Value<double>()));
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

Object::Object(pDefault* packet) : Object(static_cast<const pDefault*>(packet))
{
	PacketFactory::FreePacket(packet);
}

Object::~Object()
{

}

inline
bool Object::IsValidCoordinate(double C)
{
	return (C != 2048.0 && C != 128.0 && C != 0.0);
}

inline
bool Object::IsValidAngle(double A)
{
	return (A >= 0.0 && A <= 360.0);
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
	return object_Game_Pos.at(axis).get();
}

double Object::GetNetworkPos(unsigned char axis) const
{
	return object_Network_Pos.at(axis).get();
}

double Object::GetAngle(unsigned char axis) const
{
	return object_Angle.at(axis).get();
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
	return SetObjectValue(this->object_Name, name);
}

Lockable* Object::SetGamePos(unsigned char axis, double pos)
{
	if (!IsValidCoordinate(pos))
		return NULL;

	return SetObjectValue(this->object_Game_Pos.at(axis), pos);
}

Lockable* Object::SetNetworkPos(unsigned char axis, double pos)
{
	if (!IsValidCoordinate(pos))
		return NULL;

	return SetObjectValue(this->object_Network_Pos.at(axis), pos);
}

Lockable* Object::SetAngle(unsigned char axis, double angle)
{
	if (!IsValidAngle(angle))
		return NULL;

	return SetObjectValue(this->object_Angle.at(axis), angle);
}

Lockable* Object::SetEnabled(bool state)
{
	return SetObjectValue(this->state_Enabled, state);
}

Lockable* Object::SetGameCell(unsigned int cell)
{
	if (!cell)
		return NULL;

	return SetObjectValue(this->cell_Game, cell);
}

Lockable* Object::SetNetworkCell(unsigned int cell)
{
	if (!cell)
		return NULL;

	return SetObjectValue(this->cell_Network, cell);
}

bool Object::IsNearPoint(double X, double Y, double Z, double R) const
{
	return (sqrt((abs(GetGamePos(Axis_X) - X) * abs(GetGamePos(Axis_X) - X)) + (abs(GetGamePos(Axis_Y) - Y) * abs(GetGamePos(Axis_Y) - Y)) + (abs(GetGamePos(Axis_Z) - Z) * abs(GetGamePos(Axis_Z) - Z))) <= R);
}

bool Object::IsCoordinateInRange(unsigned char axis, double pos, double R) const
{
	return (GetGamePos(axis) > (pos - R) && GetGamePos(axis) < (pos + R));
}

bool Object::HasValidCoordinates() const
{
	for (const pair<const unsigned char, Value<double>>& pos : object_Network_Pos)
		if (!IsValidCoordinate(pos.second.get()))
			return false;

	return true;
}

pDefault* Object::toPacket()
{
	pDefault* packet = PacketFactory::CreatePacket(ID_OBJECT_NEW, this->GetNetworkID(), this->GetReference(), this->GetBase(),
												   this->GetName().c_str(), this->GetNetworkPos(Values::Axis_X), this->GetNetworkPos(Values::Axis_Y), this->GetNetworkPos(Values::Axis_Z),
												   this->GetAngle(Values::Axis_X), this->GetAngle(Values::Axis_Y), this->GetAngle(Values::Axis_Z), this->GetNetworkCell(), this->GetEnabled());

	return packet;
}
