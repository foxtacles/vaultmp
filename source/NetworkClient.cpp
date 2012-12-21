#include "NetworkClient.h"
#include "Bethesda.h"
#include "Game.h"
#include "PacketFactory.h"

using namespace std;
using namespace RakNet;

#ifdef VAULTMP_DEBUG
DebugInput<NetworkClient> NetworkClient::debug;
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

			FactoryObject<Player> reference = GameFactory::GetObject<Player>(PLAYER_REFERENCE).get();

			return NetworkResponse{Network::CreateResponse(
				reference->toPacket(),
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

	switch (data->data[0])
	{
		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
#ifdef VAULTMP_DEBUG
			debug.print("Connection request accepted (", data->systemAddress.ToString(), ")");
#endif
			response = Game::Authenticate(Bethesda::password);
			break;
		}

		case ID_DISCONNECTION_NOTIFICATION:
		{
#ifdef VAULTMP_DEBUG
			debug.print("Connection closed (", data->systemAddress.ToString(), ")");
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
					debug.print("We were successfully authenticated (", data->systemAddress.ToString(), ")");
					debug.print("Initiating vaultmp game thread...");
#endif

					Bethesda::Initialize();

					Game::cellRefs->clear();
					Game::baseRaces.clear();
					Game::globals.clear();
					Game::weather = 0x00000000;

					Game::spawnFunc = function<void()>();

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

						case Reason::ID_REASON_QUIT:
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

				case pTypes::ID_GAME_GLOBAL:
				{
					unsigned int global;
					signed int value;
					PacketFactory::Access<pTypes::ID_GAME_GLOBAL>(packet, global, value);
					Game::net_SetGlobalValue(global, value);
					break;
				}

				case pTypes::ID_GAME_WEATHER:
				{
					unsigned int weather;
					PacketFactory::Access<pTypes::ID_GAME_WEATHER>(packet, weather);
					Game::net_SetWeather(weather);
					break;
				}

				case pTypes::ID_OBJECT_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_OBJECT, packet);
					auto reference = GameFactory::GetObject(id);
					Game::NewObject(reference.get());
					break;
				}

				case pTypes::ID_ITEM_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, packet);
					auto reference = GameFactory::GetObject<Item>(id);
					Game::NewItem(reference.get());
					break;
				}

				case pTypes::ID_CONTAINER_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_CONTAINER, packet);
					auto reference = GameFactory::GetObject<Container>(id);
					Game::NewContainer(reference.get());
					break;
				}

				case pTypes::ID_ACTOR_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_ACTOR, packet);
					auto reference = GameFactory::GetObject<Actor>(id);
					Game::NewActor(reference.get());
					break;
				}

				case pTypes::ID_PLAYER_NEW:
				{
					NetworkID id = GameFactory::CreateKnownInstance(ID_PLAYER, packet);
					auto reference = GameFactory::GetObject<Player>(id);
					Game::NewPlayer(reference.get());
					break;
				}

				case pTypes::ID_OBJECT_REMOVE:
				{
					NetworkID id;
					PacketFactory::Access<pTypes::ID_OBJECT_REMOVE>(packet, id);
					auto reference = GameFactory::GetObject(id);
					Game::Delete(reference.get());
					break;
				}

				case pTypes::ID_UPDATE_POS:
				{
					NetworkID id;
					double X, Y, Z;
					PacketFactory::Access<pTypes::ID_UPDATE_POS>(packet, id, X, Y, Z);
					auto reference = GameFactory::GetObject(id);
					Game::net_SetPos(reference.get(), X, Y, Z);
					break;
				}

				case pTypes::ID_UPDATE_ANGLE:
				{
					NetworkID id;
					unsigned char axis;
					double value;
					PacketFactory::Access<pTypes::ID_UPDATE_ANGLE>(packet, id, axis, value);
					auto reference = GameFactory::GetObject(id);
					Game::net_SetAngle(reference.get(), axis, value);
					break;
				}

				case pTypes::ID_UPDATE_CELL:
				{
					NetworkID id;
					unsigned int cell;
					PacketFactory::Access<pTypes::ID_UPDATE_CELL>(packet, id, cell);
					auto reference = GameFactory::GetMultiple<Object>(vector<unsigned int>{GameFactory::LookupRefID(id), PLAYER_REFERENCE});
					Game::net_SetCell(reference[0].get(), vaultcast<Player>(reference[1]).get(), cell);
					break;
				}

				case pTypes::ID_UPDATE_CONTAINER:
				{
					NetworkID id;
					pair<list<NetworkID>, vector<pPacket>> ndiff, gdiff;
					PacketFactory::Access<pTypes::ID_UPDATE_CONTAINER>(packet, id, ndiff, gdiff);
					auto reference = GameFactory::GetObject<Container>(id);
					Game::net_ContainerUpdate(reference.get(), ndiff, gdiff);
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
					Game::net_SetActorValue(reference.get(), base, index, value);
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
					Game::net_SetActorState(reference.get(), idle, moving, movingxy, weapon, alerted, sneaking, firing);
					break;
				}

				case pTypes::ID_UPDATE_RACE:
				{
					NetworkID id;
					unsigned int race;
					signed int age, delta_age;
					PacketFactory::Access<pTypes::ID_UPDATE_RACE>(packet, id, race, age, delta_age);
					auto reference = GameFactory::GetObject<Actor>(id);
					Game::net_SetActorRace(reference.get(), race, age, delta_age);
					break;
				}

				case pTypes::ID_UPDATE_SEX:
				{
					NetworkID id;
					bool female;
					PacketFactory::Access<pTypes::ID_UPDATE_SEX>(packet, id, female);
					auto reference = GameFactory::GetObject<Actor>(id);
					Game::net_SetActorFemale(reference.get(), female);
					break;
				}

				case pTypes::ID_UPDATE_DEAD:
				{
					NetworkID id;
					bool dead;
					unsigned short limbs;
					signed char cause;
					PacketFactory::Access<pTypes::ID_UPDATE_DEAD>(packet, id, dead, limbs, cause);
					auto reference = GameFactory::GetObject<Actor>(id);
					Game::net_SetActorDead(reference.get(), dead, limbs, cause);
					break;
				}

				case pTypes::ID_UPDATE_FIREWEAPON:
				{
					NetworkID id;
					unsigned int weapon;
					double rate;
					PacketFactory::Access<pTypes::ID_UPDATE_FIREWEAPON>(packet, id, weapon, rate);
					auto reference = GameFactory::GetObject<Actor>(id);
					Game::net_FireWeapon(reference.get(), weapon, rate);
					break;
				}

				case pTypes::ID_UPDATE_IDLE:
				{
					NetworkID id;
					unsigned int idle;
					string name;
					PacketFactory::Access<pTypes::ID_UPDATE_IDLE>(packet, id, idle, name);
					auto reference = GameFactory::GetObject<Actor>(id);
					Game::net_SetActorIdle(reference.get(), idle, name);
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
