#include "NetworkServer.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include "Server.hpp"
#include "Dedicated.hpp"
#include "Game.hpp"

using namespace std;
using namespace RakNet;

#ifdef VAULTMP_DEBUG
DebugInput<NetworkServer> NetworkServer::debug;
#endif

NetworkResponse NetworkServer::ProcessEvent(unsigned char id)
{
	switch (id)
	{
		case ID_EVENT_SERVER_ERROR:
			return {{
				PacketFactory::Create<pTypes::ID_GAME_END>(Reason::ID_REASON_ERROR),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr)}
			};

		default:
			throw VaultException("Unhandled event type %d", id).stacktrace();
	}
}

NetworkResponse NetworkServer::ProcessPacket(Packet* data)
{
	NetworkResponse response;

	switch (data->data[0])
	{
		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			Utils::timestamp();
			printf("Connected to MasterServer (%s)\n", data->systemAddress.ToString());
#ifdef VAULTMP_DEBUG
			debug.print("Connected to MasterServer (", data->systemAddress.ToString(), ")");
#endif

			Dedicated::master = data->systemAddress;
			Dedicated::Announce(true);
			break;
		}

		case ID_DISCONNECTION_NOTIFICATION:
		case ID_CONNECTION_LOST:
		{
			Utils::timestamp();

			switch (data->data[0])
			{
				case ID_DISCONNECTION_NOTIFICATION:
					printf("Client disconnected (%s)\n", data->systemAddress.ToString());
#ifdef VAULTMP_DEBUG
					debug.print("Client disconnected (", data->systemAddress.ToString(), ")");
#endif
					break;

				case ID_CONNECTION_LOST:
					printf("Lost connection (%s)\n", data->systemAddress.ToString());
#ifdef VAULTMP_DEBUG
					debug.print("Lost connection (", data->systemAddress.ToString(), ")");
#endif
					break;
			}

			response = Server::Disconnect(data->guid, data->data[0] == ID_DISCONNECTION_NOTIFICATION ? Reason::ID_REASON_NONE : Reason::ID_REASON_ERROR);
			break;
		}

		case ID_NEW_INCOMING_CONNECTION:
		{
			Utils::timestamp();
			printf("New incoming connection from %s\n", data->systemAddress.ToString());
#ifdef VAULTMP_DEBUG
			debug.print("New incoming connection from ", data->systemAddress.ToString());
#endif
			break;
		}

		case ID_INVALID_PASSWORD:
		{
			Utils::timestamp();
			printf("MasterServer version mismatch (%s)\n", data->systemAddress.ToString());
#ifdef VAULTMP_DEBUG
			debug.print("MasterServer version mismatch (", data->systemAddress.ToString(), ")");
#endif
			break;
		}

		case ID_CONNECTED_PING:
		case ID_UNCONNECTED_PING:
		case ID_CONNECTION_ATTEMPT_FAILED:
		case ID_ALREADY_CONNECTED:
			break;

		default:
		{
			pPacket packet = PacketFactory::Init(data->data, data->length);

			switch (packet.type())
			{
				case pTypes::ID_GAME_AUTH:
				{
					string name, pwd;
					PacketFactory::Access<pTypes::ID_GAME_AUTH>(packet, name, pwd);
					response = Server::Authenticate(data->guid, move(name), move(pwd));
					break;
				}

				case pTypes::ID_GAME_LOAD:
				{
					response = Server::LoadGame(data->guid);
					break;
				}

				case pTypes::ID_GAME_CHAT:
				{
					string message;
					PacketFactory::Access<pTypes::ID_GAME_CHAT>(packet, message);
					response = Server::ChatMessage(data->guid, move(message));
					break;
				}

				case pTypes::ID_GAME_END:
				{
					Reason reason;
					PacketFactory::Access<pTypes::ID_GAME_END>(packet, reason);
					response = Server::Disconnect(data->guid, reason);
					break;
				}

				case pTypes::ID_PLAYER_NEW:
				{
					NetworkID id = GameFactory::Create<Player, FailPolicy::Exception>(packet);
					response = Server::NewPlayer(data->guid, id);
					break;
				}

				case pTypes::ID_UPDATE_POS:
				{
					NetworkID id;
					float X, Y, Z;
					PacketFactory::Access<pTypes::ID_UPDATE_POS>(packet, id, X, Y, Z);
					auto reference = GameFactory::Get<Object>(id);
					response = Server::GetPos(data->guid, reference.get(), X, Y, Z);
					break;
				}

				case pTypes::ID_UPDATE_ANGLE:
				{
					NetworkID id;
					float X, Z;
					PacketFactory::Access<pTypes::ID_UPDATE_ANGLE>(packet, id, X, Z);
					auto reference = GameFactory::Get<Object>(id);
					response = Server::GetAngle(data->guid, reference.get(), X, 0.00, Z);
					break;
				}

				case pTypes::ID_UPDATE_CELL:
				{
					NetworkID id;
					unsigned int cell;
					float X, Y, Z;
					PacketFactory::Access<pTypes::ID_UPDATE_CELL>(packet, id, cell, X, Y, Z);
					auto reference = GameFactory::Get<Object>(id);
					response = Server::GetCell(data->guid, reference.get(), cell);
					break;
				}

				case pTypes::ID_UPDATE_ACTIVATE:
				{
					NetworkID id, actor;
					PacketFactory::Access<pTypes::ID_UPDATE_ACTIVATE>(packet, id, actor);
					auto reference = GameFactory::Get<Reference>({id, actor});
					response = Server::GetActivate(data->guid, reference[0].get(), reference[1].get());
					break;
				}

				case pTypes::ID_UPDATE_STATE:
				{
					NetworkID id;
					unsigned int idle;
					unsigned char moving, movingxy, weapon;
					bool alerted, sneaking, firing;
					PacketFactory::Access<pTypes::ID_UPDATE_STATE>(packet, id, idle, moving, movingxy, weapon, alerted, sneaking, firing);
					auto reference = GameFactory::Get<Actor>(id);
					response = Server::GetActorState(data->guid, reference.get(), idle, moving, movingxy, weapon, alerted, sneaking);
					break;
				}

				case pTypes::ID_UPDATE_DEAD:
				{
					NetworkID id;
					bool dead;
					unsigned short limbs;
					signed char cause;
					PacketFactory::Access<pTypes::ID_UPDATE_DEAD>(packet, id, dead, limbs, cause);
					auto reference = GameFactory::Get<Player>(id);
					response = Server::GetActorDead(data->guid, reference.get(), dead, limbs, cause);
					break;
				}

				case pTypes::ID_UPDATE_FIREWEAPON:
				{
					NetworkID id;
					unsigned int weapon;
					PacketFactory::Access<pTypes::ID_UPDATE_FIREWEAPON>(packet, id, weapon);
					auto reference = GameFactory::Get<Player>(id);
					response = Server::GetActorFireWeapon(data->guid, reference.get());
					break;
				}

				case pTypes::ID_UPDATE_CONTROL:
				{
					NetworkID id;
					unsigned char control, key;
					PacketFactory::Access<pTypes::ID_UPDATE_CONTROL>(packet, id, control, key);
					auto reference = GameFactory::Get<Player>(id);
					response = Server::GetPlayerControl(data->guid, reference.get(), control, key);
					break;
				}

				case pTypes::ID_UPDATE_WMODE:
				{
					bool enabled;
					PacketFactory::Access<pTypes::ID_UPDATE_WMODE>(packet, enabled);
					response = Server::GetWindowMode(data->guid, enabled);
					break;
				}

				case pTypes::ID_UPDATE_WCLICK:
				{
					NetworkID id;
					PacketFactory::Access<pTypes::ID_UPDATE_WCLICK>(packet, id);
					auto reference = GameFactory::Get<Window>(id);
					response = Server::GetWindowClick(data->guid, reference.get());
					break;
				}

				case pTypes::ID_UPDATE_WRETURN:
				{
					NetworkID id;
					PacketFactory::Access<pTypes::ID_UPDATE_WRETURN>(packet, id);
					auto reference = GameFactory::Get<Window>(id);
					response = Server::GetWindowReturn(data->guid, reference.get());
					break;
				}

				case pTypes::ID_UPDATE_WTEXT:
				{
					NetworkID id;
					string text;
					PacketFactory::Access<pTypes::ID_UPDATE_WTEXT>(packet, id, text);
					auto reference = GameFactory::Get<Window>(id);
					response = Server::GetWindowText(data->guid, reference.get(), text);
					break;
				}

				case pTypes::ID_UPDATE_WSELECTED:
				{
					NetworkID id;
					bool selected;
					PacketFactory::Access<pTypes::ID_UPDATE_WSELECTED>(packet, id, selected);
					auto reference = GameFactory::Get<Checkbox>(id);
					response = Server::GetCheckboxSelected(data->guid, reference.get(), selected);
					break;
				}

				case pTypes::ID_UPDATE_WRSELECTED:
				{
					NetworkID id, previous;
					bool selected;
					PacketFactory::Access<pTypes::ID_UPDATE_WRSELECTED>(packet, id, previous, selected);
					auto reference = GameFactory::Get<RadioButton>({id, previous});
					response = Server::GetRadioButtonSelected(data->guid, reference[0].get(), reference[1]);
					break;
				}

				case pTypes::ID_UPDATE_WLSELECTED:
				{
					NetworkID id;
					bool selected;
					PacketFactory::Access<pTypes::ID_UPDATE_WLSELECTED>(packet, id, selected);
					auto reference = GameFactory::Get<ListItem>(id);
					response = Server::GetListItemSelected(data->guid, reference.get(), selected);
					break;
				}

				default:
					throw VaultException("Unhandled packet type %d", data->data[0]).stacktrace();
			}
		}
	}

	return response;
}
