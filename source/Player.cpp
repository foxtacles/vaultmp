#include "Player.h"

using namespace std;

unsigned int Player::default_respawn = DEFAULT_PLAYER_RESPAWN;

#ifdef VAULTMP_DEBUG
Debug* Player::debug = NULL;
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

	for (const pair<unsigned char, pair<unsigned char, bool> >& control : controls)
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

const Parameter Player::CreateFunctor(unsigned int flags, NetworkID player)
{
	return Parameter(vector<string>(), new PlayerFunctor(flags, player));
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

pDefault* Player::toPacket()
{
	vector<unsigned char> data = API::RetrieveAllControls();
	map<unsigned char, pair<unsigned char, bool> > controls;

	for (unsigned char _data : data)
		controls.insert(pair<unsigned char, pair<unsigned char, bool> >(_data, pair<unsigned char, bool>(this->GetPlayerControl(_data), this->GetPlayerControlEnabled(_data))));

	pDefault* pActorNew = Actor::toPacket();

	pDefault* packet = PacketFactory::CreatePacket(ID_PLAYER_NEW, pActorNew, &controls);

	PacketFactory::FreePacket(pActorNew);

	return packet;
}

vector<string> PlayerFunctor::operator()()
{
	vector<string> result;

	if (this->player)
	{
		FactoryObject reference = GameFactory::GetObject(this->player);
		Player* player = vaultcast<Player>(reference);

		if (player)
		{
			if (flags & FLAG_MOVCONTROLS)
			{
				unsigned int forward, backward, left, right;

				forward = player->GetPlayerControl(Values::Fallout::ControlCode_Forward);
				backward = player->GetPlayerControl(Values::Fallout::ControlCode_Backward);
				left = player->GetPlayerControl(Values::Fallout::ControlCode_Left);
				right = player->GetPlayerControl(Values::Fallout::ControlCode_Right);

				unsigned int movcontrols = (right | (left << 8) | (backward << 16) | (forward << 24));

				char value[64];
				snprintf(value, sizeof(value), "%d", movcontrols);
				result.push_back(string(value));
			}
		}
	}

	else
	{
		vector<FactoryObject>::iterator it;
		vector<FactoryObject> playerlist = GameFactory::GetObjectTypes(ID_PLAYER);

		for (it = playerlist.begin(); it != playerlist.end(); GameFactory::LeaveReference(*it), ++it)
		{
			Player* player = vaultcast<Player>(*it);
			unsigned int refID = player->GetReference();

			if (refID != 0x00000000)
			{
				if (flags& FLAG_NOTSELF && refID == PLAYER_REFERENCE)
					continue;

				if (flags & FLAG_ENABLED && !player->GetEnabled())
					continue;

				else if (flags & FLAG_DISABLED && player->GetEnabled())
					continue;

				if (flags & FLAG_ALIVE && player->GetActorDead())
					continue;

				else if (flags & FLAG_DEAD && !player->GetActorDead())
					continue;

				if (flags & FLAG_ISALERTED && !player->GetActorAlerted())
					continue;

				else if (flags & FLAG_NOTALERTED && player->GetActorAlerted())
					continue;

				result.push_back(Utils::LongToHex(refID));
			}
		}
	}

	_next(result);

	return result;
}

PlayerFunctor::~PlayerFunctor()
{

}
