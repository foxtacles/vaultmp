#include "Server.hpp"
#include "Script.hpp"
#include "Client.hpp"
#include "ServerEntry.hpp"
#include "Game.hpp"

#ifdef VAULTMP_DEBUG
DebugInput<Server> Server::debug;
#endif

using namespace std;
using namespace RakNet;
using namespace Values;

NetworkResponse Server::Authenticate(RakNetGUID guid, const string& name, const string& pwd)
{
	NetworkResponse response;

	Script::CBR<Script::CBI("OnClientAuthenticate")> result = true;
	Script::Call<Script::CBI("OnClientAuthenticate")>(result, name.c_str(), pwd.c_str());

	if (result)
	{
		for (const auto& mod : Dedicated::modfiles)
			response.emplace_back(
				PacketFactory::Create<pTypes::ID_GAME_MOD>(mod.first, mod.second),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_START>(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);
	}
	else
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_END>(Reason::ID_REASON_DENIED),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

	return response;
}

NetworkResponse Server::LoadGame(RakNetGUID guid)
{
	NetworkResponse response;

	response.emplace_back(
		PacketFactory::Create<pTypes::ID_GAME_DELETED>(Script::GetDeletedStatic()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

	unsigned int cellID = Player::GetSpawnCell();
	auto cell = DB::Exterior::Lookup(cellID);

	if (cell)
	{
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_EXTERIOR>(0, cell->GetWorld(), cell->GetX(), cell->GetY(), true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(0, cell->GetAdjacents(), true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);
	}
	else
	{
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_INTERIOR>(0, DB::Record::Lookup(cellID, "CELL")->GetName(), true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(0, Player::CellContext{{cellID, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}}, true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);
	}

	response.emplace_back(
		PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameYear, Script::GetGameYear()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

	response.emplace_back(
		PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameMonth, Script::GetGameMonth()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

	response.emplace_back(
		PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameDay, Script::GetGameDay()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

	response.emplace_back(
		PacketFactory::Create<pTypes::ID_GAME_GLOBAL>(Global_GameHour, Script::GetGameHour()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

	response.emplace_back(
		PacketFactory::Create<pTypes::ID_GAME_WEATHER>(Script::GetGameWeather()),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

	response.emplace_back(
		PacketFactory::Create<pTypes::ID_GAME_LOAD>(),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

	return response;
}

NetworkResponse Server::NewPlayer(RakNetGUID guid, NetworkID id)
{
	NetworkResponse response;

	Client* client = new Client(guid, id);
	Dedicated::self->SetServerPlayers({Client::GetClientCount(), Dedicated::connections});

	Script::AttachWindow(id, GameFactory::Operate<Window>(GameFactory::Create<Window, FailPolicy::Exception>(get<0>(Window::GUI_MAIN_POS), get<1>(Window::GUI_MAIN_POS), get<0>(Window::GUI_MAIN_SIZE), get<1>(Window::GUI_MAIN_SIZE), true, false, Window::GUI_MAIN_TEXT), [](Window* window) {
		window->SetLabel(Window::GUI_MAIN_LABEL);
		return window->GetNetworkID();
	}));

	Script::CBR<Script::CBI("OnPlayerRequestGame")> result = 0x00000000;
	Script::Call<Script::CBI("OnPlayerRequestGame")>(result, id);

	auto player_name = GameFactory::Operate<Player>(id, [&response, guid, id, client, &result](Player* player) {
		auto baseIDs = Player::GetBaseIDs();

		// TODO hardcoded hack to not get DLC bases, no proper mod handling yet
		if (!result)
			result = DB::NPC::GetNPC([&baseIDs](const DB::NPC& data)
			{
				return find(baseIDs.begin(), baseIDs.end(), data.GetBase()) == baseIDs.end() && (!(data.GetBase() & 0xFF000000) && !data.IsEssential() && !DB::Race::Lookup(data.GetRace())->IsChild());
			})->GetBase();

		const auto* npc = *DB::NPC::Lookup(result);

		player->SetReference(0x00000000);
		player->SetBase(result);

		auto cell = Player::GetSpawnCell();
		player->SetNetworkCell(cell);
		player->SetGameCell(cell);

		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_BASE>(result),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_CONSOLE>(0, player->GetPlayerConsoleEnabled()),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

		unsigned int race = npc->GetRace();
		unsigned int old_race = player->GetActorRace();

		if (player->SetActorRace(race))
		{
			signed int age = DB::Race::Lookup(old_race)->GetAgeDifference(race);

			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_RACE>(id, race, age, age),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);
		}

		signed int age = DB::Race::Lookup(npc->GetOriginalRace())->GetAgeDifference(race);
		player->SetActorAge(age);

		bool female = npc->IsFemale();

		if (player->SetActorFemale(female))
		{
			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_SEX>(id, female),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);
		}

		response.emplace_back(
			player->toPacket(),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(client));

		return player->GetName();
	});

	GameFactory::Operate<Reference, EXCEPTION_FACTORY_VALIDATED>(GameFactory::GetByType(ALL_REFERENCES), [&response, guid, id](FactoryReferences& references) {
		partition(references.begin(), references.end(), [](FactoryReference& reference) { return reference->IsPersistent() && reference->GetReference() != PLAYER_REFERENCE; });

		for (auto& reference : references)
		{
			if (reference->GetNetworkID() == id)
				continue;

			auto item = vaultcast<Item>(reference);

			if (item && item->GetItemContainer())
				continue;

			response.emplace_back(
				reference->toPacket(),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

			GameFactory::Free(reference);
		}
	});

	Script::SetBaseName(id, player_name.c_str());

	Script::Call<Script::CBI("OnSpawn")>(id);

	return response;
}

NetworkResponse Server::Disconnect(RakNetGUID guid, Reason reason)
{
	NetworkResponse response;
	Client* client = Client::GetClientFromGUID(guid);

	if (client != nullptr)
	{
		NetworkID id = GameFactory::Get<Player>(client->GetPlayer())->GetNetworkID();
		Script::Call<Script::CBI("OnPlayerDisconnect")>(id, reason);
		delete client;

		GameFactory::Destroy(Script::GetPlayerChatboxWindow(id));
		GameFactory::Destroy(id);

		response.emplace_back(
			PacketFactory::Create<pTypes::ID_OBJECT_REMOVE>(id, true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr));

		Dedicated::self->SetServerPlayers({Client::GetClientCount(), Dedicated::connections});
	}

	return response;
}

NetworkResponse Server::GetPos(RakNetGUID guid, FactoryObject& reference, float X, float Y, float Z)
{
	NetworkResponse response;

	if (!DB::Record::IsValidCoordinate(reference->GetNetworkCell(), X, Y, Z))
		return response;

	bool result = static_cast<bool>(reference->SetNetworkPos(tuple<float, float, float>{X, Y, Z}));

	if (result)
	{
		reference->SetGamePos(tuple<float, float, float>{X, Y, Z});

		unsigned int cell = reference->GetNetworkCell();

		if (reference->SetGameCell(cell))
		{
			NetworkID id = reference->GetNetworkID();
			reference->SetGameCell(cell);

			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, cell, X, Y, Z),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid));

			GameFactory::Operate<Player, RETURN_VALIDATED>(id, [&response, guid, id](Player* player) {
				response.emplace_back(
					PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, player->GetPlayerCellContext(), false),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);
			});

			GameFactory::Free(reference);
			Script::Call<Script::CBI("OnCellChange")>(id, cell);
		}
		else
			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_POS>(reference->GetNetworkID(), X, Y, Z),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid));
	}

	return response;
}

NetworkResponse Server::GetAngle(RakNetGUID guid, FactoryObject& reference, float X, float Y, float Z)
{
	NetworkResponse response;
	bool result = static_cast<bool>(reference->SetAngle(tuple<float, float, float>{X, Y, Z}));

	if (result)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_ANGLE>(reference->GetNetworkID(), X, Z),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid));

	return response;
}

NetworkResponse Server::GetCell(RakNetGUID guid, FactoryObject& reference, unsigned int cell)
{
	NetworkResponse response;

	const auto& pos = reference->GetNetworkPos();
	bool valid = DB::Record::IsValidCoordinate(cell, get<0>(pos), get<1>(pos), get<2>(pos));
	bool result = static_cast<bool>(reference->SetNetworkCell(cell)) && valid;

	if (result)
	{
		NetworkID id = reference->GetNetworkID();
		reference->SetGameCell(cell);

		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_CELL>(id, cell, get<0>(pos), get<1>(pos), get<2>(pos)),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid));

		GameFactory::Operate<Player, RETURN_VALIDATED>(id, [&response, guid, id](Player* player) {
			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_CONTEXT>(id, player->GetPlayerCellContext(), false),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);
		});

		GameFactory::Free(reference);
		Script::Call<Script::CBI("OnCellChange")>(id, cell);
	}

	return response;
}

NetworkResponse Server::GetActivate(RakNetGUID guid, FactoryReference& reference, FactoryReference& actor)
{
	NetworkResponse response;

	NetworkID reference_id = reference->GetNetworkID();
	NetworkID actor_id = actor->GetNetworkID();

	if (reference->IsPersistent() && !DB::Reference::Lookup(reference->GetReference())->GetType().compare("DOOR"))
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_ACTIVATE>(reference_id, actor_id),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guid);

	GameFactory::Free(reference);
	GameFactory::Free(actor);

	Script::Call<Script::CBI("OnActivate")>(reference_id, actor_id);

	return response;
}

NetworkResponse Server::GetActorState(RakNetGUID guid, FactoryActor& reference, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking)
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

		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_STATE>(id, idle, moving, movingxy, weapon, alerted, sneaking, !punching && !power_punching && firing),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid));

		if (_idle)
		{
			auto record = DB::Record::Lookup(idle, "IDLE");

			response.emplace_back(
				PacketFactory::Create<pTypes::ID_UPDATE_IDLE>(id, idle, record ? record->GetName() : ""),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid));
		}

		if (_weapon)
		{
			if (power_punching)
			{
				GameFactory::Free(reference);
				Script::Call<Script::CBI("OnActorPunch")>(id, true);
			}
			else if (punching)
			{
				GameFactory::Free(reference);
				Script::Call<Script::CBI("OnActorPunch")>(id, false);
			}
			else
				GameFactory::Free(reference);
		}
		else
			GameFactory::Free(reference);

		if (_alerted)
			Script::Call<Script::CBI("OnActorAlert")>(id, alerted);

		if (_sneaking)
			Script::Call<Script::CBI("OnActorSneak")>(id, sneaking);
	}

	return response;
}

NetworkResponse Server::GetActorDead(RakNetGUID guid, FactoryPlayer& reference, bool dead, unsigned short, signed char)
{
	NetworkResponse response;
	bool result;

	if (dead) // only used by the client to notify about respawn of player
		return response;

	result = static_cast<bool>(reference->SetActorDead(dead, 0x0000, Death_None));

	if (result)
	{
		NetworkID id = reference->GetNetworkID();

		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_DEAD>(id, dead, 0, 0),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid));

		GameFactory::Free(reference);

		Script::Call<Script::CBI("OnSpawn")>(id);
	}

	return response;
}

NetworkResponse Server::GetActorFireWeapon(RakNet::RakNetGUID guid, FactoryPlayer& reference)
{
	NetworkResponse response;
	NetworkID id = reference->GetNetworkID();

	unsigned int baseID = reference->GetEquippedWeapon();

	response.emplace_back(
		PacketFactory::Create<pTypes::ID_UPDATE_FIREWEAPON>(id, baseID),
		HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(guid));

	GameFactory::Free(reference);

	Script::Call<Script::CBI("OnActorFireWeapon")>(id, baseID);

	return response;
}

NetworkResponse Server::GetPlayerControl(RakNetGUID, FactoryPlayer& reference, unsigned char control, unsigned char key)
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

NetworkResponse Server::GetWindowMode(RakNetGUID guid, bool enabled)
{
	NetworkResponse response;

	Script::Call<Script::CBI("OnWindowMode")>(Client::GetClientFromGUID(guid)->GetPlayer(), enabled);

	return response;
}

NetworkResponse Server::GetWindowClick(RakNetGUID guid, FactoryWindow& reference)
{
	NetworkResponse response;

	NetworkID id = reference->GetNetworkID();
	GameFactory::Free(reference);

	Script::Call<Script::CBI("OnWindowClick")>(id, Client::GetClientFromGUID(guid)->GetPlayer());

	return response;
}

NetworkResponse Server::GetWindowReturn(RakNetGUID guid, FactoryWindow& reference)
{
	NetworkResponse response;

	NetworkID id = reference->GetNetworkID();
	GameFactory::Free(reference);

	Script::Call<Script::CBI("OnWindowReturn")>(id, Client::GetClientFromGUID(guid)->GetPlayer());

	return response;
}

NetworkResponse Server::GetWindowText(RakNetGUID guid, FactoryWindow& reference, const string& text)
{
	NetworkResponse response;

	NetworkID id = reference->GetNetworkID();
	reference->SetText(text);
	GameFactory::Free(reference);

	NetworkID root = Script::GetWindowRoot(id);

	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root), guid));

	if (!guids.empty())
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_WTEXT>(id, text),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids);

	Script::Call<Script::CBI("OnWindowTextChange")>(id, Client::GetClientFromGUID(guid)->GetPlayer(), text.c_str());

	return response;
}

NetworkResponse Server::GetCheckboxSelected(RakNetGUID guid, FactoryCheckbox& reference, bool selected)
{
	NetworkResponse response;

	NetworkID id = reference->GetNetworkID();
	reference->SetSelected(selected);
	GameFactory::Free(reference);

	NetworkID root = Script::GetWindowRoot(id);

	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root), guid));

	if (!guids.empty())
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_WSELECTED>(id, selected),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids);

	Script::Call<Script::CBI("OnCheckboxSelect")>(id, Client::GetClientFromGUID(guid)->GetPlayer(), selected);

	return response;
}

NetworkResponse Server::GetRadioButtonSelected(RakNetGUID guid, FactoryRadioButton& reference, ExpectedRadioButton& previous)
{
	NetworkResponse response;

	NetworkID id = reference->GetNetworkID();
	NetworkID previous_id = 0;

	reference->SetSelected(true);
	GameFactory::Free(reference);

	if (previous)
	{
		previous_id = previous->GetNetworkID();
		previous->SetSelected(false);
		GameFactory::Free(previous.get());
	}

	NetworkID root = Script::GetWindowRoot(id);

	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root), guid));

	if (!guids.empty())
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_WRSELECTED>(id, previous_id, true),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids);

	Script::Call<Script::CBI("OnRadioButtonSelect")>(id, previous_id, Client::GetClientFromGUID(guid)->GetPlayer());

	return response;
}

 NetworkResponse Server::GetListItemSelected(RakNetGUID guid, FactoryListItem& reference, bool selected)
 {
	NetworkResponse response;

	NetworkID id = reference->GetNetworkID();
	NetworkID list = reference->GetItemContainer();
	reference->SetSelected(selected);
	GameFactory::Free(reference);

	NetworkID root = Script::GetWindowRoot(list);

	vector<RakNetGUID> guids(Client::GetNetworkList(Player::GetWindowPlayers(root), guid));

	if (!guids.empty())
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_UPDATE_WLSELECTED>(id, selected),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, guids);

	Script::Call<Script::CBI("OnListItemSelect")>(id, Client::GetClientFromGUID(guid)->GetPlayer(), selected);

	return response;
 }

NetworkResponse Server::ChatMessage(RakNetGUID guid, const string& message)
{
	Client* client = Client::GetClientFromGUID(guid);

	NetworkID id = GameFactory::Get<Player>(client->GetPlayer())->GetNetworkID();

	NetworkResponse response;

	char _message[MAX_CHAT_LENGTH + 1];
	ZeroMemory(_message, sizeof(_message));
	strncpy(_message, message.c_str(), sizeof(_message) - 1);

	Script::CBR<Script::CBI("OnPlayerChat")> result = true;
	Script::Call<Script::CBI("OnPlayerChat"), true>(result, id, static_cast<char*>(_message));

	if (result && *_message)
		response.emplace_back(
			PacketFactory::Create<pTypes::ID_GAME_CHAT>(_message),
			HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr));

	return response;
}
