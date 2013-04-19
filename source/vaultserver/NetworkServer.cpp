#include "NetworkServer.h"
#include "Dedicated.h"
#include "GameFactory.h"

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
			return {Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_GAME_END>(Reason::ID_REASON_ERROR),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Client::GetNetworkList(nullptr))
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
			pPacket _packet = PacketFactory::Init(data->data, data->length);
			const pDefault* packet = _packet.get();

			switch (static_cast<pTypes>(data->data[0]))
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
					NetworkID id = GameFactory::CreateKnownInstance(ID_PLAYER, packet);
					response = Server::NewPlayer(data->guid, id);
					break;
				}

				case pTypes::ID_UPDATE_POS:
				{
					NetworkID id;
					double X, Y, Z;
					PacketFactory::Access<pTypes::ID_UPDATE_POS>(packet, id, X, Y, Z);
					auto reference = GameFactory::GetObject(id);
					response = Server::GetPos(data->guid, reference.get(), X, Y, Z);
					break;
				}

				case pTypes::ID_UPDATE_ANGLE:
				{
					NetworkID id;
					unsigned char axis;
					double value;
					PacketFactory::Access<pTypes::ID_UPDATE_ANGLE>(packet, id, axis, value);
					auto reference = GameFactory::GetObject(id);
					response = Server::GetAngle(data->guid, reference.get(), axis, value);
					break;
				}

				case pTypes::ID_UPDATE_CELL:
				{
					NetworkID id;
					unsigned int cell;
					PacketFactory::Access<pTypes::ID_UPDATE_CELL>(packet, id, cell);
					auto reference = GameFactory::GetObject(id);
					response = Server::GetCell(data->guid, reference.get(), cell);
					break;
				}

				case pTypes::ID_UPDATE_LOCK:
				{
					NetworkID id;
					unsigned int lock;
					PacketFactory::Access<pTypes::ID_UPDATE_LOCK>(packet, id, lock);
					auto reference = GameFactory::GetMultiple(vector<NetworkID>{id, Client::GetClientFromGUID(data->guid)->GetPlayer()});
					auto player = vaultcast<Player>(reference[1]);
					GameFactory::LeaveReference(reference[1].get());
					response = Server::GetLock(data->guid, reference[0].get(), player.get(), lock);
					break;
				}

				case pTypes::ID_UPDATE_CONTAINER:
				{
					NetworkID id;
					pair<list<NetworkID>, vector<pPacket>> ndiff, gdiff;
					PacketFactory::Access<pTypes::ID_UPDATE_CONTAINER>(packet, id, ndiff, gdiff);
					auto reference = GameFactory::GetObject<Container>(id);
					response = Server::GetContainerUpdate(data->guid, reference.get(), ndiff, gdiff);
					break;
				}

				case pTypes::ID_UPDATE_VALUE:
				{
					NetworkID id;
					bool base;
					unsigned char index;
					double value;
					PacketFactory::Access<pTypes::ID_UPDATE_VALUE>(packet, id, base, index, value);
					auto reference = GameFactory::GetObject<Actor>(id);
					response = Server::GetActorValue(data->guid, reference.get(), base, index, value);
					break;
				}

				case pTypes::ID_UPDATE_STATE:
				{
					NetworkID id;
					unsigned int idle;
					unsigned char moving, movingxy, weapon;
					bool alerted, sneaking, firing;
					PacketFactory::Access<pTypes::ID_UPDATE_STATE>(packet, id, idle, moving, movingxy, weapon, alerted, sneaking, firing);
					auto reference = GameFactory::GetObject<Actor>(id);
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
					auto reference = GameFactory::GetMultiple<Actor>(vector<NetworkID>{id, Client::GetClientFromGUID(data->guid)->GetPlayer()});
					auto player = vaultcast<Player>(reference[1]);
					GameFactory::LeaveReference(reference[1].get());
					response = Server::GetActorDead(data->guid, reference[0].get(), player.get(), dead, limbs, cause);
					break;
				}

				case pTypes::ID_UPDATE_CONTROL:
				{
					NetworkID id;
					unsigned char control, key;
					PacketFactory::Access<pTypes::ID_UPDATE_CONTROL>(packet, id, control, key);
					auto reference = GameFactory::GetObject<Player>(id);
					response = Server::GetPlayerControl(data->guid, reference.get(), control, key);
					break;
				}

				default:
					throw VaultException("Unhandled packet type %d", data->data[0]).stacktrace();
			}
		}
	}

	return response;
}
