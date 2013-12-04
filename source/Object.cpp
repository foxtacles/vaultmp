#include "Object.hpp"

#ifndef VAULTSERVER
#include "Game.hpp"
#endif

using namespace std;
using namespace RakNet;
using namespace Values;

#ifdef VAULTMP_DEBUG
DebugInput<Object> Object::debug;
#endif

Object::Object(unsigned int refID, unsigned int baseID) : Reference(refID, baseID)
{
	initialize();
}

Object::Object(const pPacket& packet) : Reference(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	string name;
	tuple<float, float, float> pos, angle;
	unsigned int cell;
	bool enabled;
	unsigned int lock;
	unsigned int owner;

	PacketFactory::Access<pTypes::ID_OBJECT_NEW>(packet, name, pos, angle, cell, enabled, lock, owner);

	this->SetNetworkPos(pos);
	this->SetAngle(angle);
	this->SetName(move(name));
	this->SetNetworkCell(cell);
	this->SetEnabled(enabled);
	this->SetLockLevel(lock);
	this->SetOwner(owner);

	if (this->GetReference())
	{
		this->SetGamePos(pos);
		this->SetGameCell(cell);
	}
}

Object::~Object() noexcept
{

}

void Object::initialize()
{
	this->SetLockLevel(Lock_Unlocked);
}

bool Object::IsValidCoordinate(float C)
{
	return (C != 2048.0 && C != 128.0 && C != 0.0);
}

bool Object::IsValidAngle(unsigned char axis, float A)
{
	return (axis == Axis_Z ? (A >= 0.0 && A <= 360.0) : (A >= -90.0 && A <= 90.0));
}

#ifndef VAULTSERVER
FuncParameter Object::CreateFunctor(unsigned int flags, NetworkID id)
{
	return FuncParameter(unique_ptr<VaultFunctor>(new ObjectFunctor(flags, id)));
}
#endif

const string& Object::GetName() const
{
	return *object_Name;
}

const tuple<float, float, float>& Object::GetGamePos() const
{
	return *object_Game_Pos;
}

const tuple<float, float, float>& Object::GetNetworkPos() const
{
	return *object_Network_Pos;
}

const tuple<float, float, float>& Object::GetAngle() const
{
	return *object_Angle;
}

unsigned int Object::GetGameCell() const
{
	return cell_Game.get();
}

unsigned int Object::GetNetworkCell() const
{
	return cell_Network.get();
}

bool Object::GetEnabled() const
{
	return state_Enabled.get();
}

unsigned int Object::GetLockLevel() const
{
	return state_Lock.get();
}

unsigned int Object::GetOwner() const
{
	return state_Owner.get();
}

Lockable* Object::SetName(const string& name)
{
	return SetObjectValue(this->object_Name, name);
}

Lockable* Object::SetGamePos(const tuple<float, float, float>& pos)
{
	if (!IsValidCoordinate(get<0>(pos)) || !IsValidCoordinate(get<1>(pos)) || !IsValidCoordinate(get<2>(pos)))
		return nullptr;

	return SetObjectValue(this->object_Game_Pos, pos);
}

Lockable* Object::SetNetworkPos(const tuple<float, float, float>& pos)
{
	if (!IsValidCoordinate(get<0>(pos)) || !IsValidCoordinate(get<1>(pos)) || !IsValidCoordinate(get<2>(pos)))
		return nullptr;

	return SetObjectValue(this->object_Network_Pos, pos);
}

Lockable* Object::SetAngle(const tuple<float, float, float>& angle)
{
	if (!IsValidAngle(Axis_X, get<0>(angle)) || !IsValidAngle(Axis_Z, get<2>(angle)))
		return nullptr;

	return SetObjectValue(this->object_Angle, angle);
}

Lockable* Object::SetGameCell(unsigned int cell)
{
	if (!cell)
		return nullptr;

	return SetObjectValue(this->cell_Game, cell);
}

Lockable* Object::SetNetworkCell(unsigned int cell)
{
	if (!cell)
		return nullptr;

	return SetObjectValue(this->cell_Network, cell);
}

Lockable* Object::SetEnabled(bool state)
{
	return SetObjectValue(this->state_Enabled, state);
}

Lockable* Object::SetLockLevel(unsigned int lock)
{
	return SetObjectValue(this->state_Lock, lock);
}

Lockable* Object::SetOwner(unsigned int owner)
{
	return SetObjectValue(this->state_Owner, owner);
}

VaultVector Object::vvec() const
{
	return VaultVector(get<0>(*object_Game_Pos), get<1>(*object_Game_Pos), get<2>(*object_Game_Pos));
}

bool Object::IsNearPoint(float X, float Y, float Z, float R) const
{
	return this->vvec().IsNearPoint(VaultVector(X, Y, Z), R);
}

bool Object::IsCoordinateInRange(unsigned char axis, float pos, float R) const
{
	float pos_ = axis == Axis_X ? get<0>(*object_Game_Pos) : (axis == Axis_Y ? get<1>(*object_Game_Pos) : (axis == Axis_Z ? get<2>(*object_Game_Pos) : throw VaultException("Unknown axis value")));
	return (pos_ > (pos - R) && pos_ < (pos + R));
}

pair<float, float> Object::GetOffset(float N) const
{
	return this->vvec().GetOffset(get<2>(*object_Angle), N);
}

bool Object::HasValidCoordinates() const
{
	if (!IsValidCoordinate(get<0>(*object_Network_Pos)) || !IsValidCoordinate(get<1>(*object_Network_Pos)) || !IsValidCoordinate(get<2>(*object_Network_Pos)))
		return false;

	return true;
}

#ifdef VAULTSERVER
Lockable* Object::SetBase(unsigned int baseID)
{
	const DB::Record* record = *DB::Record::Lookup(baseID, vector<string>{"DOOR", "TERM", "STAT"});

	if (this->GetName().empty())
		this->SetName(record->GetDescription());

	return Reference::SetBase(baseID);
}
#endif

pPacket Object::toPacket() const
{
	pPacket pReferenceNew = Reference::toPacket();

	pPacket packet = PacketFactory::Create<pTypes::ID_OBJECT_NEW>(pReferenceNew, this->GetName(), this->GetNetworkPos(), this->GetAngle(), this->GetNetworkCell(), this->GetEnabled(), this->GetLockLevel(), this->GetOwner());

	return packet;
}

#ifndef VAULTSERVER
vector<string> ObjectFunctor::operator()()
{
	vector<string> result;
	NetworkID id = get();

	if (id)
		GameFactory::Operate<Object, RETURN_VALIDATED>(id, [this, &result](Object* object) {
			result.emplace_back(Utils::toString(object->GetReference()));
		});
	else
	{
		auto references = Game::GetContext(ID_OBJECT);

		for (unsigned int refID : references)
			GameFactory::Operate<Object, RETURN_FACTORY_VALIDATED>(refID, [this, refID, &result](FactoryObject& object) {
				if (!filter(object))
					result.emplace_back(Utils::toString(refID));
			});
	}

	_next(result);

	return result;
}

bool ObjectFunctor::filter(FactoryWrapper<Reference>& reference)
{
	return GameFactory::Operate<Object>(reference->GetNetworkID(), [this](Object* object) {
		unsigned int flags = this->flags();

		if (flags & FLAG_NOTSELF && object->GetReference() == PLAYER_REFERENCE)
			return true;

		else if (flags & FLAG_SELF && object->GetReference() != PLAYER_REFERENCE)
			return true;

		if (flags & FLAG_ENABLED && !object->GetEnabled())
			return true;

		return false;
	});
}
#endif
