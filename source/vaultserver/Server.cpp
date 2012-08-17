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

NetworkResponse Server::Authenticate(RakNetGUID guid, string name, string pwd)
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
			PacketFactory::Create<pTypes::ID_GAME_END>(pTypes::ID_REASON_DENIED),
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
			PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(0, cell.GetWorld(), cell.GetX(), cell.GetY()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}
	catch (...)
	{
		const Record& record = Record::Lookup(Player::GetSpawnCell(), "CELL");

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(0, record.GetName()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	vector<FactoryObject> references = GameFactory::GetObjectTypes(ALL_OBJECTS);
	vector<FactoryObject>::iterator it;

	for (it = references.begin(); it != references.end(); GameFactory::LeaveReference(*it), ++it)
	{
		Object* object = vaultcast<Object>(*it);

		if (vaultcast<Item>(*it))
			continue; // FIXME, this is to not send items in a container

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
	FactoryObject _player = GameFactory::GetObject(id);
	Player* player = vaultcast<Player>(_player);

	Client* client = new Client(guid, player->GetNetworkID());
	Dedicated::self->SetServerPlayers(make_pair(Client::GetClientCount(), Dedicated::connections));

	unsigned int result = Script::OnPlayerRequestGame(_player);

	if (!result)
		throw VaultException("Script did not provide an actor base for a player");

	player->SetReference(0x00000000);
	player->SetBase(result);

	SingleResponse response[] = {Network::CreateResponse(
		player->toPacket(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(client))
	};

	Script::OnSpawn(_player);

	return NetworkResponse(make_move_iterator(begin(response)), make_move_iterator(end(response)));
}

NetworkResponse Server::Disconnect(RakNetGUID guid, pTypes reason)
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

NetworkResponse Server::GetContainerUpdate(RakNetGUID guid, const FactoryObject& reference, const pair<list<NetworkID>, vector<pPacket>>& _diff)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", reference->GetReference());

	SingleResponse response[] = {Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(container->GetNetworkID(), _diff),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid))
	};

	ContainerDiff diff = Container::ToContainerDiff(_diff);
	GameDiff gamediff = container->ApplyDiff(diff);

	for (const auto& _diff : gamediff)
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

NetworkResponse Server::GetActorState(RakNetGUID guid, const FactoryObject& reference, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", reference->GetReference());

	NetworkResponse response;
	bool result, _alerted, _sneaking, _weapon;

	_alerted = static_cast<bool>(actor->SetActorAlerted(alerted));
	_sneaking = static_cast<bool>(actor->SetActorSneaking(sneaking));
	_weapon = static_cast<bool>(actor->SetActorWeaponAnimation(weapon));
	result = (static_cast<bool>(actor->SetActorMovingAnimation(moving)) | static_cast<bool>(actor->SetActorMovingXY(movingxy)) | _weapon | _alerted | _sneaking);

	if (result)
	{
		bool punching = _weapon && actor->IsActorPunching();
		bool power_punching = _weapon && actor->IsActorPowerPunching();

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(actor->GetNetworkID(), moving, movingxy, weapon, alerted, sneaking, !punching && !power_punching),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		if (_weapon)
		{
			if (power_punching)
			{
				// power punch
				// OnActorPunch
			}
			else if (punching)
			{
				// Normal punch
				// OnActorPunch
			}
			else if (actor->IsActorFiring())
			{
				unsigned int baseID = actor->GetEquippedWeapon();
				const Weapon& weapon = Weapon::Lookup(baseID);

				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_FIREWEAPON>(actor->GetNetworkID(), baseID, weapon.IsAutomatic() ? weapon.GetFireRate() : 0.00),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

				// OnActorFireWeapon
			}
		}

		if (_alerted)
			Script::OnActorAlert(reference, alerted);

		if (_sneaking)
			Script::OnActorSneak(reference, sneaking);
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
