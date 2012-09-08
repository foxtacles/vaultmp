#include "Server.h"

#ifdef VAULTMP_DEBUG
Debug* Server::debug = nullptr;
#endif

using namespace Values;

#ifdef VAULTMP_DEBUG
void Server::SetDebugHandler(Debug* debug)
{
	Server::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Server class", true);
}
#endif

NetworkResponse Server::Authenticate(RakNetGUID guid, const string& name, const string& pwd)
{
	NetworkResponse response;
	bool result = Script::OnClientAuthenticate(name, pwd);

	if (result)
	{
		for (const auto& mod : Dedicated::modfiles)
		{
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_GAME_MOD>(mod.first, mod.second),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
		}

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_START>(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}
	else
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_END>(Reason::ID_REASON_DENIED),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	return response;
}

NetworkResponse Server::LoadGame(RakNetGUID guid)
{
	NetworkResponse response;

	try
	{
		const Exterior& cell = Exterior::Lookup(Player::GetSpawnCell());

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(0, cell.GetWorld(), cell.GetX(), cell.GetY(), true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}
	catch (...)
	{
		const Record& record = Record::Lookup(Player::GetSpawnCell(), "CELL");

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(0, record.GetName(), true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	vector<FactoryObject> references = GameFactory::GetObjectTypes(ALL_OBJECTS);
	vector<FactoryObject>::iterator it;

	for (it = references.begin(); it != references.end(); GameFactory::LeaveReference(*it), ++it)
	{
		Object* object = vaultcast<Object>(*it);
		Item* item;

		if ((item = vaultcast<Item>(*it)) && item->GetItemContainer())
			continue;

		response.emplace_back(Network::CreateResponse(
			object->toPacket(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	response.emplace_back(Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_LOAD>(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	return response;
}

NetworkResponse Server::NewPlayer(RakNetGUID guid, NetworkID id)
{
	NetworkResponse response;
	FactoryObject _player = GameFactory::GetObject(id);
	Player* player = vaultcast<Player>(_player);

	Client* client = new Client(guid, player->GetNetworkID());
	Dedicated::self->SetServerPlayers(make_pair(Client::GetClientCount(), Dedicated::connections));

	unsigned int result = Script::OnPlayerRequestGame(_player);

	// TODO hardcoded hack to not get DLC bases, no proper mod handling yet
	if (!result)
		result = NPC::GetNPCNotIn(Player::GetBaseIDs(), [](const NPC& data)
		{
			return (!(data.GetBase() & 0xFF000000) && !data.IsEssential() && !Race::Lookup(data.GetRace()).IsChild());
		}).GetBase();

	const NPC& npc = NPC::Lookup(result);

	player->SetReference(0x00000000);
	player->SetBase(result);

	unsigned int race = npc.GetRace();

	if (player->SetActorRace(race))
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_RACE>(id, race),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	bool female = npc.IsFemale();

	if (player->SetActorFemale(female))
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_SEX>(id, female),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	response.emplace_back(Network::CreateResponse(
		player->toPacket(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(client)));

	Script::OnSpawn(_player);

	return response;
}

NetworkResponse Server::Disconnect(RakNetGUID guid, Reason reason)
{
	NetworkResponse response;
	Client* client = Client::GetClientFromGUID(guid);

	if (client != nullptr)
	{
		FactoryObject reference = GameFactory::GetObject(client->GetPlayer());
		Script::OnPlayerDisconnect(reference, reason);
		delete client;

		NetworkID id = GameFactory::DestroyInstance(reference);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_OBJECT_REMOVE>(id),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)));

		Dedicated::self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), Dedicated::connections));
	}

	return response;
}

NetworkResponse Server::GetPos(RakNetGUID guid, const FactoryObject& reference, double X, double Y, double Z)
{
	NetworkResponse response;
	Object* object = vaultcast<Object>(reference);
	bool result = (static_cast<bool>(object->SetNetworkPos(Axis_X, X)) | static_cast<bool>(object->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(object->SetNetworkPos(Axis_Z, Z)));

	if (result)
	{
		object->SetGamePos(Axis_X, X);
		object->SetGamePos(Axis_Y, Y);
		object->SetGamePos(Axis_Z, Z);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_POS>(object->GetNetworkID(), X, Y, Z),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, Client::GetNetworkList(guid)));
	}

	return response;
}

NetworkResponse Server::GetAngle(RakNetGUID guid, const FactoryObject& reference, unsigned char axis, double value)
{
	NetworkResponse response;
	Object* object = vaultcast<Object>(reference);
	bool result = static_cast<bool>(object->SetAngle(axis, value));

	if (result)
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(object->GetNetworkID(), axis, value),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, Client::GetNetworkList(guid)));
	}

	return response;
}

NetworkResponse Server::GetCell(RakNetGUID guid, const FactoryObject& reference, unsigned int cell)
{
	NetworkResponse response;
	Object* object = vaultcast<Object>(reference);
	bool result = static_cast<bool>(object->SetNetworkCell(cell));

	if (result)
	{
		object->SetGameCell(cell);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CELL>(object->GetNetworkID(), cell),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		Script::OnCellChange(reference, cell);
	}

	return response;
}

NetworkResponse Server::GetContainerUpdate(RakNetGUID guid, const FactoryObject& reference, const pair<list<NetworkID>, vector<pPacket>>& ndiff, const pair<list<NetworkID>, vector<pPacket>>& gdiff)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", reference->GetReference());

	SingleResponse response[] = {Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(container->GetNetworkID(), ndiff, gdiff),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid))
	};

	ContainerDiff diff = Container::ToContainerDiff(ndiff);
	GameDiff _gdiff = container->ApplyDiff(diff);

	for (const auto& packet : gdiff.second)
	{
		NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, packet.get());
		FactoryObject _reference = GameFactory::GetObject(id);

		Item* item = vaultcast<Item>(_reference);

		item->SetReference(0x00000000);

		unsigned int baseID = item->GetBase();
		_gdiff.remove_if([=](const pair<unsigned int, Diff>& diff) { return diff.first == baseID; });

		Script::OnActorDropItem(reference, baseID, item->GetItemCount(), item->GetItemCondition());
	}

	for (const auto& id : gdiff.first)
	{
		FactoryObject _reference = GameFactory::GetObject(id);
		Item* item = vaultcast<Item>(_reference);

		unsigned int baseID = item->GetBase();
		_gdiff.remove_if([=](const pair<unsigned int, Diff>& diff) { return diff.first == baseID; });

		unsigned int count = item->GetItemCount();
		double condition = item->GetItemCondition();

		GameFactory::DestroyInstance(_reference);

		Script::OnActorPickupItem(reference, baseID, count, condition);
	}

	for (const auto& _diff : _gdiff)
	{
		if (_diff.second.equipped)
		{
			if (_diff.second.equipped > 0)
				Script::OnActorEquipItem(reference, _diff.first, _diff.second.condition);
			else if (_diff.second.equipped < 0)
				Script::OnActorUnequipItem(reference, _diff.first, _diff.second.condition);
		}
		else
			Script::OnContainerItemChange(reference, _diff.first, _diff.second.count, _diff.second.condition);
	}

	return NetworkResponse(make_move_iterator(begin(response)), make_move_iterator(end(response)));
}

NetworkResponse Server::GetActorValue(RakNetGUID guid, const FactoryObject& reference, bool base, unsigned char index, double value)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	NetworkResponse response;
	bool result;

	if (base)
		result = static_cast<bool>(actor->SetActorBaseValue(index, value));
	else
		result = static_cast<bool>(actor->SetActorValue(index, value));

	if (result)
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(actor->GetNetworkID(), base, index, value),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		Script::OnActorValueChange(reference, index, base, value);
	}

	return response;
}

NetworkResponse Server::GetActorState(RakNetGUID guid, const FactoryObject& reference, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	NetworkResponse response;
	bool result, _alerted, _sneaking, _weapon, _idle;

	_alerted = static_cast<bool>(actor->SetActorAlerted(alerted));
	_sneaking = static_cast<bool>(actor->SetActorSneaking(sneaking));
	_weapon = static_cast<bool>(actor->SetActorWeaponAnimation(weapon));
	_idle = static_cast<bool>(actor->SetActorIdleAnimation(idle));
	result = (static_cast<bool>(actor->SetActorMovingAnimation(moving)) | static_cast<bool>(actor->SetActorMovingXY(movingxy)) | _idle | _weapon | _alerted | _sneaking);

	if (result)
	{
		bool punching = _weapon && actor->IsActorPunching();
		bool power_punching = _weapon && actor->IsActorPowerPunching();
		bool firing = _weapon && actor->IsActorFiring();

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(actor->GetNetworkID(), idle, moving, movingxy, weapon, alerted, sneaking, !punching && !power_punching && firing),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		if (_weapon)
		{
			if (power_punching)
				Script::OnActorPunch(reference, true);
			else if (punching)
				Script::OnActorPunch(reference, false);
			else if (firing)
			{
				unsigned int baseID = actor->GetEquippedWeapon();
				const Weapon& weapon = Weapon::Lookup(baseID);

				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_FIREWEAPON>(actor->GetNetworkID(), baseID, weapon.IsAutomatic() ? weapon.GetFireRate() : 0.00),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

				Script::OnActorFireWeapon(reference, baseID);
			}
		}

		if (_alerted)
			Script::OnActorAlert(reference, alerted);

		if (_sneaking)
			Script::OnActorSneak(reference, sneaking);

		if (_idle)
		{
			const Record* record = nullptr;

			if (idle)
				record = &Record::Lookup(idle, "IDLE");

			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_IDLE>(actor->GetNetworkID(), idle, record ? record->GetName() : ""),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));
		}
	}

	return response;
}

NetworkResponse Server::GetActorDead(RakNetGUID guid, const FactoryObject& reference, bool dead, unsigned short limbs, signed char cause)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	NetworkResponse response;
	bool result;

	result = static_cast<bool>(actor->SetActorDead(dead));

	if (result)
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(actor->GetNetworkID(), dead, limbs, cause),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		if (dead)
		{
			Script::OnActorDeath(reference, limbs, cause);

			Player* player = vaultcast<Player>(reference);

			if (player)
				Script::CreateTimerEx(reinterpret_cast<ScriptFunc>(&Script::Timer_Respawn), player->GetPlayerRespawn(), "l", player->GetNetworkID());
		}
		else
			Script::OnSpawn(reference);
	}

	return response;
}

NetworkResponse Server::GetPlayerControl(RakNetGUID guid, const FactoryObject& reference, unsigned char control, unsigned char key)
{
	Player* player = vaultcast<Player>(reference);

	if (!player)
		throw VaultException("Object with reference %08X is not a Player", reference->GetReference());

	NetworkResponse response;
	bool result;

	result = static_cast<bool>(player->SetPlayerControl(control, key));

	if (result)
	{
		// maybe script call
	}

	return response;
}

NetworkResponse Server::ChatMessage(RakNetGUID guid, string message)
{
	Client* client = Client::GetClientFromGUID(guid);

	FactoryObject reference = GameFactory::GetObject(client->GetPlayer());

	Player* player = vaultcast<Player>(reference);

	if (!player)
		throw VaultException("Object with reference %08X is not a Player", reference->GetReference());

	NetworkResponse response;

	bool result = Script::OnPlayerChat(reference, message);

	if (result && !message.empty())
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_CHAT>(message),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)));
	}

	return response;
}
