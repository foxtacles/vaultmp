#include "Player.h"
#include "PacketFactory.h"

using namespace std;

#ifdef VAULTSERVER
unsigned int Player::default_respawn = DEFAULT_PLAYER_RESPAWN;
unsigned int Player::default_cell;
#endif

#ifdef VAULTMP_DEBUG
Debug* Player::debug;
#endif

Player::Player(unsigned int refID, unsigned int baseID) : Actor(refID, baseID)
{
	initialize();
}

Player::Player(const pDefault* packet) : Actor(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	map<unsigned char, pair<unsigned char, bool>> controls;

	PacketFactory::Access<pTypes::ID_PLAYER_NEW>(packet, controls);

	for (const auto& control : controls)
	{
		this->SetPlayerControl(control.first, control.second.first);
		this->SetPlayerControlEnabled(control.first, control.second.second);
	}
}

Player::~Player()
{
#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("Player object destroyed (ref: %08X)", true, GetReference());

#endif
}

void Player::initialize()
{
	vector<unsigned char> data = API::RetrieveAllControls();

	for (unsigned char _data : data)
		player_Controls.insert(make_pair(_data, make_pair(Value<unsigned char>(), Value<bool>(true))));

#ifdef VAULTSERVER
	player_Respawn.set(default_respawn);
	player_Cell.set(default_cell);
#endif
}

#ifdef VAULTMP_DEBUG
void Player::SetDebugHandler(Debug* debug)
{
	Player::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Player class", true);
}
#endif

#ifdef VAULTSERVER
unsigned int Player::GetRespawn()
{
	return default_respawn;
}

unsigned int Player::GetSpawnCell()
{
	return default_cell;
}

void Player::SetRespawn(unsigned int respawn)
{
	default_respawn = respawn;
}

void Player::SetSpawnCell(unsigned int cell)
{
	default_cell = cell;
}
#endif

FuncParameter Player::CreateFunctor(unsigned int flags, NetworkID id)
{
	return FuncParameter(unique_ptr<VaultFunctor>(new PlayerFunctor(flags, id)));
}

unsigned char Player::GetPlayerControl(unsigned char control) const
{
	return player_Controls.at(control).first.get();
}

bool Player::GetPlayerControlEnabled(unsigned char control) const
{
	return player_Controls.at(control).second.get();
}

#ifdef VAULTSERVER
unsigned int Player::GetPlayerRespawn() const
{
	return player_Respawn.get();
}

unsigned int Player::GetPlayerSpawnCell() const
{
	return player_Cell.get();
}
#endif

Lockable* Player::SetPlayerControl(unsigned char control, unsigned char key)
{
	return SetObjectValue(this->player_Controls.at(control).first, key);
}

Lockable* Player::SetPlayerControlEnabled(unsigned char control, bool state)
{
	return SetObjectValue(this->player_Controls.at(control).second, state);
}

#ifdef VAULTSERVER
Lockable* Player::SetPlayerRespawn(unsigned int respawn)
{
	return SetObjectValue(this->player_Respawn, respawn);
}

Lockable* Player::SetPlayerSpawnCell(unsigned int cell)
{
	return SetObjectValue(this->player_Cell, cell);
}
#endif

pPacket Player::toPacket() const
{
	vector<unsigned char> data = API::RetrieveAllControls();
	map<unsigned char, pair<unsigned char, bool>> controls;

	for (unsigned char _data : data)
		controls.insert(make_pair(_data, make_pair(this->GetPlayerControl(_data), this->GetPlayerControlEnabled(_data))));

	pPacket pActorNew = Actor::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_PLAYER_NEW>(pActorNew, controls);

	return packet;
}

vector<string> PlayerFunctor::operator()()
{
	vector<string> result;
	NetworkID id = get();

	if (id)
	{
		FactoryObject reference = GameFactory::GetObject(id);
		Player* player = vaultcast<Player>(reference);

		if (player)
		{
			unsigned int flags = this->flags();

			if (flags & FLAG_MOVCONTROLS)
			{
				unsigned int forward, backward, left, right;

				forward = player->GetPlayerControl(Values::ControlCode_Forward);
				backward = player->GetPlayerControl(Values::ControlCode_Backward);
				left = player->GetPlayerControl(Values::ControlCode_Left);
				right = player->GetPlayerControl(Values::ControlCode_Right);

				unsigned int movcontrols = (right | (left << 8) | (backward << 16) | (forward << 24));

				result.emplace_back(Utils::toString(movcontrols));
			}
		}
	}
	else
	{
		vector<FactoryObject>::iterator it;
		vector<FactoryObject> references = GameFactory::GetObjectTypes(ID_PLAYER);
		unsigned int refID;

		for (it = references.begin(); it != references.end(); GameFactory::LeaveReference(*it), ++it)
			if ((refID = (*it)->GetReference()) && !filter(*it))
				result.emplace_back(Utils::toString(refID));
	}

	_next(result);

	return result;
}

bool PlayerFunctor::filter(FactoryObject& reference)
{
	if (ActorFunctor::filter(reference))
		return true;

	Player* player = vaultcast<Player>(reference);

	return false;
}
