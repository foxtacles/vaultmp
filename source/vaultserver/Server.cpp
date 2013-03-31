#include "Server.h"
#include "Script.h"

#ifdef VAULTMP_DEBUG
DebugInput<Server> Server::debug;
#endif

using namespace std;
using namespace RakNet;
using namespace Values;

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

	auto cell = DB::Exterior::Lookup(Player::GetSpawnCell());

	if (cell)
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(0, cell->GetWorld(), cell->GetX(), cell->GetY(), true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(0, cell->GetAdjacents()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}
	else
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(0, DB::Record::Lookup(Player::GetSpawnCell(), "CELL")->GetName(), true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(0, array<unsigned int, 9>{{Player::GetSpawnCell(), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}}),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	vector<FactoryObject<Object>> references = GameFactory::GetObjectTypes<Object>(ALL_OBJECTS);
	vector<FactoryObject<Object>>::iterator it;

	for (it = references.begin(); it != references.end(); GameFactory::LeaveReference(*it), ++it)
	{
		auto item = vaultcast<Item>(*it);

		if (item && item->GetItemContainer())
			continue;

		response.emplace_back(Network::CreateResponse(
			(*it)->toPacket(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	response.emplace_back(Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameYear, Script::GetGameYear()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	response.emplace_back(Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameMonth, Script::GetGameMonth()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	response.emplace_back(Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameDay, Script::GetGameDay()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	response.emplace_back(Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameHour, Script::GetGameHour()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	response.emplace_back(Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_WEATHER>(Script::GetGameWeather()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	response.emplace_back(Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_LOAD>(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	return response;
}

NetworkResponse Server::NewPlayer(RakNetGUID guid, NetworkID id)
{
	NetworkResponse response;

	Client* client = new Client(guid, id);
	Dedicated::self->SetServerPlayers(make_pair(Client::GetClientCount(), Dedicated::connections));

	unsigned int result = Script::OnPlayerRequestGame(id);

	auto player = GameFactory::GetObject<Player>(id).get();

	// TODO hardcoded hack to not get DLC bases, no proper mod handling yet
	if (!result)
		result = DB::NPC::GetNPCNotIn(Player::GetBaseIDs(), [](const DB::NPC& data)
		{
			return (!(data.GetBase() & 0xFF000000) && !data.IsEssential() && !DB::Race::Lookup(data.GetRace())->IsChild());
		})->GetBase();

	const auto* npc = *DB::NPC::Lookup(result);

	player->SetReference(0x00000000);
	player->SetBase(result);

	auto cell = Player::GetSpawnCell();
	player->SetNetworkCell(cell);
	player->SetGameCell(cell);

	response.emplace_back(Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_GAME_BASE>(result),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	response.emplace_back(Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_UPDATE_CONSOLE>(0, player->GetPlayerConsoleEnabled()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	response.emplace_back(Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_UPDATE_CHAT>(0, player->GetPlayerChatboxEnabled(), player->GetPlayerChatboxLocked(), player->GetPlayerChatboxPos(), player->GetPlayerChatboxSize()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

	const auto& container = npc->GetBaseContainer();

	for (const auto* item : container)
	{
		// TODO see above
		if (item->GetItem() & 0xFF000000)
			continue;

		auto diff = player->AddItem(item->GetItem(), item->GetCount(), item->GetCondition(), true);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(id, Container::ToNetDiff(diff), Container::NetDiff()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

		player->ApplyDiff(diff);
	}

	unsigned int race = npc->GetRace();
	unsigned int old_race = player->GetActorRace();

	if (player->SetActorRace(race))
	{
		signed int age = DB::Race::Lookup(old_race)->GetAgeDifference(race);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_RACE>(id, race, age, age),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	signed int age = DB::Race::Lookup(npc->GetOriginalRace())->GetAgeDifference(race);
	player->SetActorAge(age);

	bool female = npc->IsFemale();

	if (player->SetActorFemale(female))
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_SEX>(id, female),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));
	}

	response.emplace_back(Network::CreateResponse(
		player->toPacket(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(client)));

	Script::SetBaseName(id, player->GetName().c_str());

	GameFactory::LeaveReference(player);

	Script::OnSpawn(id);

	return response;
}

NetworkResponse Server::Disconnect(RakNetGUID guid, Reason reason)
{
	NetworkResponse response;
	Client* client = Client::GetClientFromGUID(guid);

	if (client != nullptr)
	{
		NetworkID id = GameFactory::GetObject<Player>(client->GetPlayer())->GetNetworkID();
		Script::OnPlayerDisconnect(id, reason);
		delete client;

		GameFactory::DestroyInstance(id);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_OBJECT_REMOVE>(id),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)));

		Dedicated::self->SetServerPlayers(make_pair(Client::GetClientCount(), Dedicated::connections));
	}

	return response;
}

NetworkResponse Server::GetPos(RakNetGUID guid, FactoryObject<Object>& reference, double X, double Y, double Z)
{
	NetworkResponse response;
	bool result = (static_cast<bool>(reference->SetNetworkPos(Axis_X, X)) | static_cast<bool>(reference->SetNetworkPos(Axis_Y, Y)) | static_cast<bool>(reference->SetNetworkPos(Axis_Z, Z)));

	if (result)
	{
		reference->SetGamePos(Axis_X, X);
		reference->SetGamePos(Axis_Y, Y);
		reference->SetGamePos(Axis_Z, Z);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_POS>(reference->GetNetworkID(), X, Y, Z),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, Client::GetNetworkList(guid)));
	}

	return response;
}

NetworkResponse Server::GetAngle(RakNetGUID guid, FactoryObject<Object>& reference, unsigned char axis, double value)
{
	NetworkResponse response;
	bool result = static_cast<bool>(reference->SetAngle(axis, value));

	if (result)
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(reference->GetNetworkID(), axis, value),
			HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_GAME, Client::GetNetworkList(guid)));
	}

	return response;
}

NetworkResponse Server::GetCell(RakNetGUID guid, FactoryObject<Object>& reference, unsigned int cell)
{
	NetworkResponse response;
	bool result = static_cast<bool>(reference->SetNetworkCell(cell));

	if (result)
	{
		NetworkID id = reference->GetNetworkID();
		auto player = vaultcast<Player>(reference);
		reference->SetGameCell(cell);
		GameFactory::LeaveReference(reference);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, cell),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		if (player)
		{
			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, player->GetPlayerCellContext()),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid));

			GameFactory::LeaveReference(player.get());
		}

		Script::OnCellChange(id, cell);
	}

	return response;
}

NetworkResponse Server::GetLock(RakNetGUID guid, FactoryObject<Object>& reference, FactoryObject<Player>& player, unsigned int lock)
{
	NetworkResponse response;
	bool result = static_cast<bool>(reference->SetLockLevel(lock));

	if (result)
	{
		NetworkID id = reference->GetNetworkID();
		NetworkID player_id = player->GetNetworkID();
		GameFactory::LeaveReference(reference);
		GameFactory::LeaveReference(player);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_LOCK>(id, lock),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		Script::OnLockChange(id, player_id, lock);
	}

	return response;
}

NetworkResponse Server::GetContainerUpdate(RakNetGUID guid, FactoryObject<Container>& reference, const Container::NetDiff& ndiff, const Container::NetDiff& gdiff)
{
	NetworkID reference_id = reference->GetNetworkID();

	SingleResponse response[] = {Network::CreateResponse(
		PacketFactory::Create<pTypes::ID_UPDATE_CONTAINER>(reference_id, ndiff, gdiff),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid))
	};

	auto diff = Container::ToContainerDiff(ndiff);
	auto _gdiff = reference->ApplyDiff(diff);

	GameFactory::LeaveReference(reference);

	for (const auto& packet : gdiff.second)
	{
		NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, packet.get());
		FactoryObject<Item> item = GameFactory::GetObject<Item>(id).get();

		item->SetReference(0x00000000);

		unsigned int baseID = item->GetBase();
		unsigned int count = item->GetItemCount();
		double condition = item->GetItemCondition();
		_gdiff.remove_if([baseID](const pair<unsigned int, Container::Diff>& diff) { return diff.first == baseID; });

		GameFactory::LeaveReference(item);
		Script::OnActorDropItem(reference_id, baseID, count, condition);
	}

	for (const auto& id : gdiff.first)
	{
		auto _item = GameFactory::GetObject<Item>(id);

		if (!_item)
		{
#ifdef VAULTMP_DEBUG
			debug.print("WARNING (GetContainerUpdate): item ", dec, id, " not found. Has it already been deleted? ", GameFactory::IsDeleted(id) ? "YES" : "NO");
#endif
			continue;
		}

		auto& item = _item.get();

		unsigned int baseID = item->GetBase();
		_gdiff.remove_if([baseID](const pair<unsigned int, Container::Diff>& diff) { return diff.first == baseID; });

		unsigned int count = item->GetItemCount();
		double condition = item->GetItemCondition();
		unsigned int owner = item->GetOwner();

		GameFactory::DestroyInstance(item);

		Script::OnActorPickupItem(reference_id, baseID, count, condition, owner);
	}

	for (const auto& _diff : _gdiff)
	{
		if (_diff.second.equipped)
		{
			if (_diff.second.equipped > 0)
				Script::OnActorEquipItem(reference_id, _diff.first, _diff.second.condition);
			else if (_diff.second.equipped < 0)
				Script::OnActorUnequipItem(reference_id, _diff.first, _diff.second.condition);
		}
		else
			Script::OnContainerItemChange(reference_id, _diff.first, _diff.second.count, _diff.second.condition);
	}

	return NetworkResponse(make_move_iterator(begin(response)), make_move_iterator(end(response)));
}

NetworkResponse Server::GetActorValue(RakNetGUID guid, FactoryObject<Actor>& reference, bool base, unsigned char index, double value)
{
	NetworkResponse response;
	bool result;

	if (base)
		result = static_cast<bool>(reference->SetActorBaseValue(index, value));
	else
		result = static_cast<bool>(reference->SetActorValue(index, value));

	if (result)
	{
		NetworkID id = reference->GetNetworkID();
		GameFactory::LeaveReference(reference);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_VALUE>(id, base, index, value),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		Script::OnActorValueChange(id, index, base, value);
	}

	return response;
}

NetworkResponse Server::GetActorState(RakNetGUID guid, FactoryObject<Actor>& reference, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking)
{
	NetworkResponse response;
	bool result, _alerted, _sneaking, _weapon, _idle;

	_alerted = static_cast<bool>(reference->SetActorAlerted(alerted));
	_sneaking = static_cast<bool>(reference->SetActorSneaking(sneaking));
	_weapon = static_cast<bool>(reference->SetActorWeaponAnimation(weapon));
	_idle = static_cast<bool>(reference->SetActorIdleAnimation(idle));
	result = (static_cast<bool>(reference->SetActorMovingAnimation(moving)) | static_cast<bool>(reference->SetActorMovingXY(movingxy)) | _idle | _weapon | _alerted | _sneaking);

	if (result)
	{
		NetworkID id = reference->GetNetworkID();

		bool punching = _weapon && reference->IsActorPunching();
		bool power_punching = _weapon && reference->IsActorPowerPunching();
		bool firing = _weapon && reference->IsActorFiring();

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(id, idle, moving, movingxy, weapon, alerted, sneaking, !punching && !power_punching && firing),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		unsigned int baseID = reference->GetEquippedWeapon();

		GameFactory::LeaveReference(reference);

		if (_weapon)
		{
			if (power_punching)
				Script::OnActorPunch(id, true);
			else if (punching)
				Script::OnActorPunch(id, false);
			else if (firing)
			{
				const DB::Weapon* weapon = *DB::Weapon::Lookup(baseID);

				response.emplace_back(Network::CreateResponse(
					PacketFactory::Create<pTypes::ID_UPDATE_FIREWEAPON>(id, baseID, weapon->IsAutomatic() ? weapon->GetFireRate() : 0.00),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

				Script::OnActorFireWeapon(id, baseID);
			}
		}

		if (_alerted)
			Script::OnActorAlert(id, alerted);

		if (_sneaking)
			Script::OnActorSneak(id, sneaking);

		if (_idle)
		{
			const DB::Record* record = nullptr;

			if (idle)
				record = *DB::Record::Lookup(idle, "IDLE");

			response.emplace_back(Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_UPDATE_IDLE>(id, idle, record ? record->GetName() : ""),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));
		}
	}

	return response;
}

NetworkResponse Server::GetActorDead(RakNetGUID guid, FactoryObject<Actor>& reference, FactoryObject<Player>& killer, bool dead, unsigned short limbs, signed char cause)
{
	NetworkResponse response;
	bool result;

	result = static_cast<bool>(reference->SetActorDead(dead));

	if (result)
	{
		NetworkID id = reference->GetNetworkID();
		NetworkID killer_id = killer->GetNetworkID();
		GameFactory::LeaveReference(reference);
		GameFactory::LeaveReference(killer);

		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, dead, limbs, cause),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid)));

		if (dead)
		{
			Script::OnActorDeath(id, killer_id, limbs, cause);

			auto player = GameFactory::GetObject<Player>(id);

			if (player)
				Script::CreateTimerEx(reinterpret_cast<ScriptFunc>(&Script::Timer_Respawn), player->GetPlayerRespawnTime(), "l", player->GetNetworkID());
		}
		else
			Script::OnSpawn(id);
	}

	return response;
}

NetworkResponse Server::GetPlayerControl(RakNetGUID, FactoryObject<Player>& reference, unsigned char control, unsigned char key)
{
	NetworkResponse response;
	bool result;

	result = static_cast<bool>(reference->SetPlayerControl(control, key));

	if (result)
	{
		// maybe script call
	}

	return response;
}

NetworkResponse Server::ChatMessage(RakNetGUID guid, string message)
{
	Client* client = Client::GetClientFromGUID(guid);

	NetworkID id = GameFactory::GetObject<Player>(client->GetPlayer())->GetNetworkID();

	NetworkResponse response;

	bool result = Script::OnPlayerChat(id, message);

	if (result && !message.empty())
	{
		response.emplace_back(Network::CreateResponse(
			PacketFactory::Create<pTypes::ID_GAME_CHAT>(message),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)));
	}

	return response;
}
