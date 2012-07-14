#include "NetworkClient.h"
#include "Bethesda.h"
#include "Game.h"

#ifdef VAULTMP_DEBUG
Debug* NetworkClient::debug = NULL;
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
				PacketFactory::CreatePacket(ID_GAME_END, ID_REASON_ERROR),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Game::server)
			};

		case ID_EVENT_GAME_STARTED:
			Network::ToggleDequeue(false);

			return NetworkResponse{Network::CreateResponse(
				PacketFactory::CreatePacket(ID_GAME_LOAD),
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

			switch (data->data[0])
			{
				case ID_GAME_MOD:
				{
					char modfile[MAX_MOD_FILE + 1];
					ZeroMemory(modfile, sizeof(modfile));
					unsigned int crc;
					PacketFactory::Access(packet, modfile, &crc);
					Bethesda::modfiles.push_back(pair<string, unsigned int>(string(modfile), crc));
					break;
				}

				case ID_GAME_START:
				{
#ifdef VAULTMP_DEBUG
					debug->PrintFormat("We were successfully authenticated (%s)", true, data->systemAddress.ToString());
					debug->Print("Initiating vaultmp game thread...", true);
#endif
					char cell[MAX_CELL_NAME + 1];
					ZeroMemory(cell, sizeof(cell));
					PacketFactory::Access(packet, cell);

					Bethesda::Initialize();
					Game::CenterOnCell(cell);
					Game::LoadEnvironment();

					response = NetworkClient::ProcessEvent(ID_EVENT_GAME_STARTED);
					break;
				}

				case ID_GAME_LOAD:
				{
					Game::Startup();
					response = NetworkClient::ProcessEvent(ID_EVENT_GAME_LOADED);
					break;
				}

				case ID_GAME_END:
				{
					unsigned char reason;
					PacketFactory::Access(packet, &reason);

					switch (reason)
					{
						case ID_REASON_KICK:
							throw VaultException("You have been kicked from the server");

						case ID_REASON_BAN:
							throw VaultException("You have been banned from the server");

						case ID_REASON_ERROR:
							throw VaultException("The server encountered an internal error");

						case ID_REASON_DENIED:
							throw VaultException("Your authentication has been denied");

						case ID_REASON_NONE:
							break;
					}

					break;
				}

				case ID_GAME_MESSAGE:
				{
					char message[MAX_MESSAGE_LENGTH + 1];
					ZeroMemory(message, sizeof(message));
					PacketFactory::Access(packet, message);
					Game::net_UIMessage(message);
					break;
				}

				case ID_GAME_CHAT:
				{
					char message[MAX_CHAT_LENGTH + 1];
					ZeroMemory(message, sizeof(message));
					PacketFactory::Access(packet, message);
					Game::net_ChatMessage(message);
					break;
				}

				case ID_OBJECT_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_OBJECT, packet);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::NewObject(reference);
					break;
				}

				case ID_ITEM_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, packet);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::NewItem(reference);
					break;
				}

				case ID_CONTAINER_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_CONTAINER, packet);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::NewContainer(reference);
					break;
				}

				case ID_ACTOR_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_ACTOR, packet);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::NewActor(reference);
					break;
				}

				case ID_PLAYER_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_PLAYER, packet);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::NewPlayer(reference);
					break;
				}

				case ID_OBJECT_REMOVE:
				{
					NetworkID id;
					PacketFactory::Access(packet, &id);
					FactoryObject reference = GameFactory::GetObject(id);
					Game::Delete(reference);
					break;
				}

				case ID_OBJECT_UPDATE:
				case ID_CONTAINER_UPDATE:
				case ID_ACTOR_UPDATE:
				case ID_PLAYER_UPDATE:
				{
					switch (data->data[1])
					{
						case ID_UPDATE_POS:
						{
							NetworkID id;
							double X, Y, Z;
							PacketFactory::Access(packet, &id, &X, &Y, &Z);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_SetPos(reference, X, Y, Z);
							break;
						}

						case ID_UPDATE_ANGLE:
						{
							NetworkID id;
							unsigned char axis;
							double value;
							PacketFactory::Access(packet, &id, &axis, &value);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_SetAngle(reference, axis, value);
							break;
						}

						case ID_UPDATE_CELL:
						{
							NetworkID id;
							unsigned int cell;
							PacketFactory::Access(packet, &id, &cell);
							vector<FactoryObject> reference = GameFactory::GetMultiple(vector<unsigned int> {GameFactory::LookupRefID(id), PLAYER_REFERENCE});
							Game::net_SetCell(reference[0], reference[1], cell);
							break;
						}

						case ID_UPDATE_CONTAINER:
						{
							NetworkID id;
							ContainerDiff diff;
							PacketFactory::Access(packet, &id, &diff);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_ContainerUpdate(reference, diff);
							break;
						}

						case ID_UPDATE_VALUE:
						{
							NetworkID id;
							bool base;
							unsigned char index;
							double value;
							PacketFactory::Access(packet, &id, &base, &index, &value);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_SetActorValue(reference, base, index, value);
							break;
						}

						case ID_UPDATE_STATE:
						{
							NetworkID id;
							unsigned char moving;
							unsigned char movingxy;
							unsigned char weapon;
							bool alerted;
							bool sneaking;
							PacketFactory::Access(packet, &id, &moving, &movingxy, &weapon, &alerted, &sneaking);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_SetActorState(reference, moving, movingxy, weapon, alerted, sneaking);
							break;
						}

						case ID_UPDATE_DEAD:
						{
							NetworkID id;
							bool dead;
							PacketFactory::Access(packet, &id, &dead);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_SetActorDead(reference, dead);
							break;
						}

						case ID_UPDATE_FIREWEAPON:
						{
							NetworkID id;
							unsigned int weapon;
							PacketFactory::Access(packet, &id, &weapon);
							FactoryObject reference = GameFactory::GetObject(id);
							Game::net_FireWeapon(reference, weapon);
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
