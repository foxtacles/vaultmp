#include "NetworkClient.h"
#include "Bethesda.h"
#include "Game.h"
#include "PacketTypes.h"

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
				PacketFactory::CreatePacket<pTypes::ID_GAME_END>(pTypes::ID_REASON_ERROR),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Game::server)
			};

		case ID_EVENT_GAME_STARTED:
			Network::ToggleDequeue(false);

			return NetworkResponse{Network::CreateResponse(
				PacketFactory::CreatePacket<pTypes::ID_GAME_LOAD>(),
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
			pPacket _packet = PacketFactory::CreatePacket(data->data, data->length);
			const pDefault* packet = _packet.get();

			switch (static_cast<pTypes>(data->data[0]))
			{
				case pTypes::ID_GAME_MOD:
				{
					char modfile[MAX_MOD_FILE + 1];
					ZeroMemory(modfile, sizeof(modfile));
					unsigned int crc;
					PacketFactory::Access(packet, modfile, &crc);
					Bethesda::modfiles.push_back(pair<string, unsigned int>(string(modfile), crc));
					break;
				}

				case pTypes::ID_GAME_START:
				{
#ifdef VAULTMP_DEBUG
					debug->PrintFormat("We were successfully authenticated (%s)", true, data->systemAddress.ToString());
					debug->Print("Initiating vaultmp game thread...", true);
#endif

					Bethesda::Initialize();
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
					pTypes reason;
					PacketFactory::Access(packet, &reason);

					switch (reason)
					{
						case pTypes::ID_REASON_KICK:
							throw VaultException("You have been kicked from the server");

						case pTypes::ID_REASON_BAN:
							throw VaultException("You have been banned from the server");

						case pTypes::ID_REASON_ERROR:
							throw VaultException("The server encountered an internal error");

						case pTypes::ID_REASON_DENIED:
							throw VaultException("Your authentication has been denied");

						case pTypes::ID_REASON_NONE:
							break;
					}

					break;
				}

				case pTypes::ID_GAME_MESSAGE:
				{
					char message[MAX_MESSAGE_LENGTH + 1];
					ZeroMemory(message, sizeof(message));
					PacketFactory::Access(packet, message);
					Game::net_UIMessage(message);
					break;
				}

				case pTypes::ID_GAME_CHAT:
				{
					char message[MAX_CHAT_LENGTH + 1];
					ZeroMemory(message, sizeof(message));
					PacketFactory::Access(packet, message);
					Game::net_ChatMessage(message);
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
					PacketFactory::Access(packet, &id);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::Delete(reference);
					break;
				}

				case pTypes::ID_OBJECT_UPDATE:
				case pTypes::ID_CONTAINER_UPDATE:
				case pTypes::ID_ACTOR_UPDATE:
				case pTypes::ID_PLAYER_UPDATE:
				{
					NetworkID id;

					switch (static_cast<pTypes>(data->data[1]))
					{
						case pTypes::ID_UPDATE_POS:
						{
							double X, Y, Z;
							PacketFactory::Access(packet, &id, &X, &Y, &Z);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_SetPos(reference, X, Y, Z);
							break;
						}

						case pTypes::ID_UPDATE_ANGLE:
						{
							unsigned char axis;
							double value;
							PacketFactory::Access(packet, &id, &axis, &value);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_SetAngle(reference, axis, value);
							break;
						}

						case pTypes::ID_UPDATE_CELL:
						{
							unsigned int cell;
							PacketFactory::Access(packet, &id, &cell);
							vector<FactoryObject> reference = GameFactory::GetMultiple(vector<unsigned int> {GameFactory::LookupRefID(id), PLAYER_REFERENCE});
							Game::net_SetCell(reference[0], reference[1], cell);
							break;
						}

						case pTypes::ID_UPDATE_CONTAINER:
						{
							ContainerDiff diff;
							PacketFactory::Access(packet, &id, &diff);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_ContainerUpdate(reference, diff);
							break;
						}

						case pTypes::ID_UPDATE_VALUE:
						{
							bool base;
							unsigned char index;
							double value;
							PacketFactory::Access(packet, &id, &base, &index, &value);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_SetActorValue(reference, base, index, value);
							break;
						}

						case pTypes::ID_UPDATE_STATE:
						{
							unsigned char moving, movingxy, weapon;
							bool alerted, sneaking;
							PacketFactory::Access(packet, &id, &moving, &movingxy, &weapon, &alerted, &sneaking);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_SetActorState(reference, moving, movingxy, weapon, alerted, sneaking);
							break;
						}

						case pTypes::ID_UPDATE_DEAD:
						{
							bool dead;
							unsigned short limbs;
							signed char cause;
							PacketFactory::Access(packet, &id, &dead, &limbs, &cause);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_SetActorDead(reference, dead, limbs, cause);
							break;
						}

						case pTypes::ID_UPDATE_FIREWEAPON:
						{
							unsigned int weapon;
							PacketFactory::Access(packet, &id, &weapon);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_FireWeapon(reference, weapon);
							break;
						}

						case pTypes::ID_UPDATE_INTERIOR:
						{
							char cell[MAX_CELL_NAME + 1];
							ZeroMemory(cell, sizeof(cell));
							PacketFactory::Access(packet, &id, cell);
							Game::CenterOnCell(cell);
							break;
						}

						case pTypes::ID_UPDATE_EXTERIOR:
						{
							unsigned int baseID;
							signed int x, y;
							PacketFactory::Access(packet, &id, &baseID, &x, &y);
							Game::CenterOnWorld(baseID, x, y);
							break;
						}

						default:
							throw VaultException("Unhandled object update packet type %d", (int) data->data[1]);
					}

					break;
				}

				default:
					throw VaultException("Unhandled packet type %d", (int) data->data[0]);
			}
		}
	}

	return response;
}
