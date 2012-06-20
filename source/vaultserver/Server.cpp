#include "Server.h"

#ifdef VAULTMP_DEBUG
Debug* Server::debug = NULL;
#endif

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
		for (const pair<string, unsigned int>& mod : Dedicated::modfiles)
		{
			response.push_back(Network::CreateResponse(
				PacketFactory::CreatePacket(ID_GAME_MOD, mod.first.c_str(), mod.second),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
		}

		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_GAME_START, Dedicated::savegame.first.c_str(), Dedicated::savegame.second),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}
	else
	{
		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_GAME_END, ID_REASON_DENIED),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	return response;
}

NetworkResponse Server::LoadGame(RakNetGUID guid)
{
	NetworkResponse response;

	vector<FactoryObject> references = GameFactory::GetObjectTypes(ALL_OBJECTS);
	vector<FactoryObject>::iterator it;

	for (it = references.begin(); it != references.end(); GameFactory::LeaveReference(*it), ++it)
	{
		Object* object = vaultcast<Object>(*it);

		if (vaultcast<Item>(*it))
			continue; // FIXME, this is to not send items in a container

		response.push_back(Network::CreateResponse(
			object->toPacket(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	response.push_back(Network::CreateResponse(
		PacketFactory::CreatePacket(ID_GAME_LOAD),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	return response;
}

NetworkResponse Server::NewPlayer(RakNetGUID guid, NetworkID id)
{
	FactoryObject _player = GameFactory::GetObject(id);
	Player* player = vaultcast<Player>(_player);

	Client* client = new Client(guid, player->GetNetworkID());
	Dedicated::self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), Dedicated::connections));

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

NetworkResponse Server::Disconnect(RakNetGUID guid, unsigned char reason)
{
	NetworkResponse response;
	Client* client = Client::GetClientFromGUID(guid);

	if (client != NULL)
	{
		FactoryObject reference = GameFactory::GetObject(client->GetPlayer());
		Script::OnPlayerDisconnect(reference, reason);
		delete client;

		NetworkID id = GameFactory::DestroyInstance(reference);

		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_OBJECT_REMOVE, id),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(NULL)));

		Dedicated::self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), Dedicated::connections));
	}

	return response;
}

NetworkResponse Server::GetPos(RakNetGUID guid, FactoryObject reference, double X, double Y, double Z)
{
	NetworkResponse response;
	Object* object = vaultcast<Object>(reference);
	bool result = ((bool) object->SetNetworkPos(Axis_X, X) | (bool) object->SetNetworkPos(Axis_Y, Y) | (bool) object->SetNetworkPos(Axis_Z, Z));

	if (result)
	{
		object->SetGamePos(Axis_X, X);
		object->SetGamePos(Axis_Y, Y);
		object->SetGamePos(Axis_Z, Z);

		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_POS, object->GetNetworkID(), X, Y, Z),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, Client::GetNetworkList(guid)));
	}

	return response;
}

NetworkResponse Server::GetAngle(RakNetGUID guid, FactoryObject reference, unsigned char axis, double value)
{
	NetworkResponse response;
	Object* object = vaultcast<Object>(reference);
	bool result = (bool) object->SetAngle(axis, value);

	if (result)
	{
		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_ANGLE, object->GetNetworkID(), axis, value),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, Client::GetNetworkList(guid)));
	}

	return response;
}

NetworkResponse Server::GetCell(RakNetGUID guid, FactoryObject reference, unsigned int cell)
{
	NetworkResponse response;
	Object* object = vaultcast<Object>(reference);
	bool result = (bool) object->SetNetworkCell(cell);

	if (result)
	{
		object->SetGameCell(cell);

		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_CELL, object->GetNetworkID(), cell),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		Script::OnCellChange(reference, cell);
	}

	return response;
}

NetworkResponse Server::GetContainerUpdate(RakNetGUID guid, FactoryObject reference, ContainerDiff diff)
{
	Container* container = vaultcast<Container>(reference);

	if (!container)
		throw VaultException("Object with reference %08X is not a Container", (*reference)->GetReference());

	SingleResponse response[] = {Network::CreateResponse(
		PacketFactory::CreatePacket(ID_UPDATE_CONTAINER, container->GetNetworkID(), &diff),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid))
	};

	GameDiff gamediff = container->ApplyDiff(diff);

	for (const pair<unsigned int, Diff>& _diff : gamediff)
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

NetworkResponse Server::GetActorValue(RakNetGUID guid, FactoryObject reference, bool base, unsigned char index, double value)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	NetworkResponse response;
	bool result;

	if (base)
		result = (bool) actor->SetActorBaseValue(index, value);

	else
		result = (bool) actor->SetActorValue(index, value);

	if (result)
	{
		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_VALUE, actor->GetNetworkID(), base, index, value),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		Script::OnActorValueChange(reference, index, base, value);
	}

	return response;
}

NetworkResponse Server::GetActorState(RakNetGUID guid, FactoryObject reference, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	NetworkResponse response;
	bool result, _alerted, _sneaking;

	_alerted = (bool) actor->SetActorAlerted(alerted);
	_sneaking = (bool) actor->SetActorSneaking(sneaking);
	result = ((bool) actor->SetActorMovingAnimation(moving) | (bool) actor->SetActorMovingXY(movingxy) | (bool) actor->SetActorWeaponAnimation(weapon) | _alerted | _sneaking);

	if (result)
	{
		printf("%s %02X\n", API::RetrieveAnim_Reverse(weapon).c_str(), weapon);

		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_STATE, actor->GetNetworkID(), moving, movingxy, weapon, alerted, sneaking),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		if (_alerted)
			Script::OnActorAlert(reference, alerted);

		if (_sneaking)
			Script::OnActorSneak(reference, sneaking);
	}

	return response;
}

NetworkResponse Server::GetActorDead(RakNetGUID guid, FactoryObject reference, bool dead)
{
	Actor* actor = vaultcast<Actor>(reference);

	if (!actor)
		throw VaultException("Object with reference %08X is not an Actor", (*reference)->GetReference());

	NetworkResponse response;
	bool result;

	result = (bool) actor->SetActorDead(dead);

	if (result)
	{
		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_UPDATE_DEAD, actor->GetNetworkID(), dead),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		if (dead)
		{
			Script::OnActorDeath(reference);

			Player* player = vaultcast<Player>(actor);

			if (player)
				Script::CreateTimerEx(reinterpret_cast<ScriptFunc>(&Script::Timer_Respawn), player->GetPlayerRespawn(), "l", player->GetNetworkID());
		}
		else
			Script::OnSpawn(reference);
	}

	return response;
}

NetworkResponse Server::GetPlayerControl(RakNetGUID guid, FactoryObject reference, unsigned char control, unsigned char key)
{
	Player* player = vaultcast<Player>(reference);

	if (!player)
		throw VaultException("Object with reference %08X is not a Player", (*reference)->GetReference());

	NetworkResponse response;
	bool result;

	result = (bool) player->SetPlayerControl(control, key);

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
		throw VaultException("Object with reference %08X is not a Player", (*reference)->GetReference());

	NetworkResponse response;

	bool result = Script::OnPlayerChat(reference, message);

	if (result && !message.empty())
	{
		response.push_back(Network::CreateResponse(
			PacketFactory::CreatePacket(ID_GAME_CHAT, message.c_str()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(NULL)));
	}

	return response;
}
