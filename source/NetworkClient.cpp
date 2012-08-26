#include "NetworkClient.h"
#include "Bethesda.h"
#include "Game.h"
#include "PacketFactory.h"

#ifdef VAULTMP_DEBUG
Debug* NetworkClient::debug;
#endif

#ifdef VAULTMP_DEBUG
void NetworkClient::SetDebugHandler(Debug* debug)
{
	NetworkClient::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to NetworkClient class", true);
}
#endif

NetworkResponse NetworkClient::ProcessEvent(unsigned char id)
{
	switch (id)
	{
		case ID_EVENT_CLIENT_ERROR:
		case ID_EVENT_INTERFACE_LOST:
			return NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_GAME_END>(Interface::HasShutdown() ? Reason::ID_REASON_QUIT : Reason::ID_REASON_ERROR),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Game::server)
			};

		case ID_EVENT_GAME_STARTED:
			Network::ToggleDequeue(false);

			return NetworkResponse{Network::CreateResponse(
				PacketFactory::Create<pTypes::ID_GAME_LOAD>(),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Game::server)
			};

		case ID_EVENT_GAME_LOADED:
		{
			Network::ToggleDequeue(true);

			FactoryObject reference = GameFactory::GetObject(PLAYER_REFERENCE);
			Player* self = vaultcast<Player>(reference);

			return NetworkResponse{Network::CreateResponse(
				self->toPacket(),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Game::server)
			};
		}

		default:
			throw VaultException("Unhandled event type %d", id);
	}
}

NetworkResponse NetworkClient::ProcessPacket(Packet* data)
{
	NetworkResponse response;
	pDefault* packet;

	switch (data->data[0])
	{
		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
#ifdef VAULTMP_DEBUG
			debug->PrintFormat("Connection request accepted (%s)", true, data->systemAddress.ToString());
#endif
			response = Game::Authenticate(Bethesda::password);
			break;
		}

		case ID_DISCONNECTION_NOTIFICATION:
		{
#ifdef VAULTMP_DEBUG
			debug->PrintFormat("Connection closed (%s)", true, data->systemAddress.ToString());
#endif
			break;
		}

		case ID_INVALID_PASSWORD:
			throw VaultException("Dedicated server version mismatch.\nPlease download the most recent binaries from www.vaultmp.com");

		case ID_NO_FREE_INCOMING_CONNECTIONS:
			throw VaultException("The server is full");

		case ID_CONNECTION_ATTEMPT_FAILED:
			throw VaultException("Failed to connect to the server");

		case ID_CONNECTION_BANNED:
			throw VaultException("You are banned from the server");

		case ID_CONNECTION_LOST:
			throw VaultException("Lost connection to the server");

		case ID_UNCONNECTED_PONG:
			break;

		default:
		{
			pPacket _packet = PacketFactory::Init(data->data, data->length);
			const pDefault* packet = _packet.get();

			switch (static_cast<pTypes>(data->data[0]))
			{
				case pTypes::ID_GAME_MOD:
				{
					string modfile;
					unsigned int crc;
					PacketFactory::Access<pTypes::ID_GAME_MOD>(packet, modfile, crc);
					Bethesda::modfiles.emplace_back(move(modfile), crc);
					break;
				}

				case pTypes::ID_GAME_START:
				{
#ifdef VAULTMP_DEBUG
					debug->PrintFormat("We were successfully authenticated (%s)", true, data->systemAddress.ToString());
					debug->Print("Initiating vaultmp game thread...", true);
#endif

					Bethesda::Initialize();

					Game::cellRefs.clear();
					Game::spawnFunc = function<void()>();

					Game::LoadEnvironment();

					response = NetworkClient::ProcessEvent(ID_EVENT_GAME_STARTED);
					break;
				}

				case pTypes::ID_GAME_LOAD:
				{
					Game::Startup();
					response = NetworkClient::ProcessEvent(ID_EVENT_GAME_LOADED);
					break;
				}

				case pTypes::ID_GAME_END:
				{
					Reason reason;
					PacketFactory::Access<pTypes::ID_GAME_END>(packet, reason);

					switch (reason)
					{
						case Reason::ID_REASON_KICK:
							throw VaultException("You have been kicked from the server");

						case Reason::ID_REASON_BAN:
							throw VaultException("You have been banned from the server");

						case Reason::ID_REASON_ERROR:
							throw VaultException("The server encountered an internal error");

						case Reason::ID_REASON_DENIED:
							throw VaultException("Your authentication has been denied");

						case Reason::ID_REASON_NONE:
							break;
					}

					break;
				}

				case pTypes::ID_GAME_MESSAGE:
				{
					string message;
					PacketFactory::Access<pTypes::ID_GAME_MESSAGE>(packet, message);
					Game::net_UIMessage(move(message));
					break;
				}

				case pTypes::ID_GAME_CHAT:
				{
					string message;
					PacketFactory::Access<pTypes::ID_GAME_CHAT>(packet, message);
					Game::net_ChatMessage(move(message));
					break;
				}

				case pTypes::ID_OBJECT_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_OBJECT, packet);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::NewObject(reference);
					break;
				}

				case pTypes::ID_ITEM_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, packet);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::NewItem(reference);
					break;
				}

				case pTypes::ID_CONTAINER_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_CONTAINER, packet);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::NewContainer(reference);
					break;
				}

				case pTypes::ID_ACTOR_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_ACTOR, packet);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::NewActor(reference);
					break;
				}

				case pTypes::ID_PLAYER_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_PLAYER, packet);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::NewPlayer(reference);
					break;
				}

				case pTypes::ID_OBJECT_REMOVE:
				{
					NetworkID id;
					PacketFactory::Access<pTypes::ID_OBJECT_REMOVE>(packet, id);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::Delete(reference);
					break;
				}

				case pTypes::ID_UPDATE_POS:
				{
					NetworkID id;
					double X, Y, Z;
					PacketFactory::Access<pTypes::ID_UPDATE_POS>(packet, id, X, Y, Z);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::net_SetPos(reference, X, Y, Z);
					break;
				}

				case pTypes::ID_UPDATE_ANGLE:
				{
					NetworkID id;
					unsigned char axis;
					double value;
					PacketFactory::Access<pTypes::ID_UPDATE_ANGLE>(packet, id, axis, value);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::net_SetAngle(reference, axis, value);
					break;
				}

				case pTypes::ID_UPDATE_CELL:
				{
					NetworkID id;
					unsigned int cell;
					PacketFactory::Access<pTypes::ID_UPDATE_CELL>(packet, id, cell);
					vector<FactoryObject> reference = GameFactory::GetMultiple(vector<unsigned int> {GameFactory::LookupRefID(id), PLAYER_REFERENCE});
					Game::net_SetCell(reference[0], reference[1], cell);
					break;
				}

				case pTypes::ID_UPDATE_CONTAINER:
				{
					NetworkID id;
					pair<list<NetworkID>, vector<pPacket>> ndiff, gdiff;
					PacketFactory::Access<pTypes::ID_UPDATE_CONTAINER>(packet, id, ndiff, gdiff);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::net_ContainerUpdate(reference, ndiff, gdiff);
					break;
				}

				case pTypes::ID_UPDATE_VALUE:
				{
					NetworkID id;
					bool base;
					unsigned char index;
					double value;
					PacketFactory::Access<pTypes::ID_UPDATE_VALUE>(packet, id, base, index, value);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::net_SetActorValue(reference, base, index, value);
					break;
				}

				case pTypes::ID_UPDATE_STATE:
				{
					NetworkID id;
					unsigned char moving, movingxy, weapon;
					bool alerted, sneaking, firing;
					PacketFactory::Access<pTypes::ID_UPDATE_STATE>(packet, id, moving, movingxy, weapon, alerted, sneaking, firing);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::net_SetActorState(reference, moving, movingxy, weapon, alerted, sneaking, firing);
					break;
				}

				case pTypes::ID_UPDATE_DEAD:
				{
					NetworkID id;
					bool dead;
					unsigned short limbs;
					signed char cause;
					PacketFactory::Access<pTypes::ID_UPDATE_DEAD>(packet, id, dead, limbs, cause);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::net_SetActorDead(reference, dead, limbs, cause);
					break;
				}

				case pTypes::ID_UPDATE_FIREWEAPON:
				{
					NetworkID id;
					unsigned int weapon;
					double rate;
					PacketFactory::Access<pTypes::ID_UPDATE_FIREWEAPON>(packet, id, weapon, rate);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::net_FireWeapon(reference, weapon, rate);
					break;
				}

				case pTypes::ID_UPDATE_INTERIOR:
				{
					NetworkID id;
					string cell;
					bool spawn;
					PacketFactory::Access<pTypes::ID_UPDATE_INTERIOR>(packet, id, cell, spawn);
					Game::CenterOnCell(cell, spawn);
					break;
				}

				case pTypes::ID_UPDATE_EXTERIOR:
				{
					NetworkID id;
					unsigned int baseID;
					signed int x, y;
					bool spawn;
					PacketFactory::Access<pTypes::ID_UPDATE_EXTERIOR>(packet, id, baseID, x, y, spawn);
					Game::CenterOnWorld(baseID, x, y, spawn);
					break;
				}

				default:
					throw VaultException("Unhandled packet type %d", data->data[0]);
			}
		}
	}

	return response;
}
