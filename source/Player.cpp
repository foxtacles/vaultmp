#include "Player.h"

using namespace std;

unsigned int Player::default_respawn = DEFAULT_PLAYER_RESPAWN;

#ifdef VAULTMP_DEBUG
Debug* Player::debug = nullptr;
#endif

Player::Player(unsigned int refID, unsigned int baseID) : Actor(refID, baseID)
{
	initialize();
}

Player::Player(const pDefault* packet) : Actor(PacketFactory::ExtractPartial(packet))
{
	initialize();

	map<unsigned char, pair<unsigned char, bool> > controls;

	PacketFactory::Access(packet, &controls);

	for (const pair<unsigned char, pair<unsigned char, bool>>& control : controls)
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
		player_Controls.insert(pair<unsigned char, pair<Value<unsigned char>, Value<bool> > >(_data, pair<Value<unsigned char>, Value<bool> >(Value<unsigned char>(), Value<bool>(true))));

	player_Respawn.set(default_respawn);
}

#ifdef VAULTMP_DEBUG
void Player::SetDebugHandler(Debug* debug)
{
	Player::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Player class", true);
}
#endif

void Player::SetRespawn(unsigned int respawn)
{
	default_respawn = respawn;
}

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

unsigned int Player::GetPlayerRespawn() const
{
	return player_Respawn.get();
}

Lockable* Player::SetPlayerControl(unsigned char control, unsigned char key)
{
	return SetObjectValue(this->player_Controls.at(control).first, key);
}

Lockable* Player::SetPlayerControlEnabled(unsigned char control, bool state)
{
	return SetObjectValue(this->player_Controls.at(control).second, state);
}

Lockable* Player::SetPlayerRespawn(unsigned int respawn)
{
	return SetObjectValue(this->player_Respawn, respawn);
}

pPacket Player::toPacket()
{
	vector<unsigned char> data = API::RetrieveAllControls();
	map<unsigned char, pair<unsigned char, bool>> controls;

	for (unsigned char _data : data)
		controls.insert(pair<unsigned char, pair<unsigned char, bool> >(_data, pair<unsigned char, bool>(this->GetPlayerControl(_data), this->GetPlayerControlEnabled(_data))));

	pPacket pActorNew = Actor::toPacket();
	pPacket packet = PacketFactory::CreatePacket(ID_PLAYER_NEW, pActorNew.get(), &controls);

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

				result.push_back(Utils::toString(movcontrols));
			}
		}
	}
	else
	{
		vector<FactoryObject>::iterator it;
		vector<FactoryObject> references = GameFactory::GetObjectTypes(ID_PLAYER);
		unsigned int refID;

		for (it = references.begin(); it != references.end(); GameFactory::LeaveReference(*it), ++it)
			if ((refID = (**it)->GetReference()) && !filter(*it))
				result.push_back(Utils::toString(refID));
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
