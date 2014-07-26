#include "NetworkClient.hpp"
#include "Bethesda.hpp"
#include "Interface.hpp"
#include "Game.hpp"

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
			return {{
				PacketFactory::Create<pTypes::ID_GAME_END>(Interface::HasShutdown() ? Reason::ID_REASON_QUIT : Reason::ID_REASON_ERROR),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Game::server}
			};

		case ID_EVENT_GAME_STARTED:
			Network::ToggleDequeue(false);

			return {{
				PacketFactory::Create<pTypes::ID_GAME_LOAD>(),
				HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Game::server}
			};

		case ID_EVENT_GAME_LOADED:
		{
			Network::ToggleDequeue(true);

			return GameFactory::Operate<Player>(PLAYER_REFERENCE, [](Player* player) {
				return NetworkResponse{{
					player->toPacket(),
					HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_GAME, Game::server}
				};
			});
		}

		default:
			throw VaultException("Unhandled event type %d", id).stacktrace();
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
			throw VaultException("Dedicated server version mismatch.\nPlease download the most recent binaries from www.vaultmp.com").stacktrace();

		case ID_NO_FREE_INCOMING_CONNECTIONS:
			throw VaultException("The server is full").stacktrace();

		case ID_CONNECTION_ATTEMPT_FAILED:
			throw VaultException("Failed to connect to the server").stacktrace();

		case ID_CONNECTION_BANNED:
			throw VaultException("You are banned from the server").stacktrace();

		case ID_CONNECTION_LOST:
			throw VaultException("Lost connection to the server").stacktrace();

		case ID_UNCONNECTED_PONG:
			break;

		default:
		{
			pPacket packet = PacketFactory::Init(data->data, data->length);

			switch (packet.type())
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
					Game::Initialize();

					Game::DisablePlayerControls(true, true, true, false, true);

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
							throw VaultException("You have been kicked from the server").stacktrace();

						case Reason::ID_REASON_BAN:
							throw VaultException("You have been banned from the server").stacktrace();

						case Reason::ID_REASON_ERROR:
							throw VaultException("The server encountered an internal error").stacktrace();

						case Reason::ID_REASON_DENIED:
							throw VaultException("Your authentication has been denied").stacktrace();

						case Reason::ID_REASON_QUIT:
						case Reason::ID_REASON_NONE:
							break;
					}

					break;
				}

				case pTypes::ID_GAME_MESSAGE:
				{
					string message;
					unsigned char emoticon;
					PacketFactory::Access<pTypes::ID_GAME_MESSAGE>(packet, message, emoticon);
					Game::net_UIMessage(move(message), emoticon);
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

				case pTypes::ID_GAME_BASE:
				{
					unsigned int base;
					PacketFactory::Access<pTypes::ID_GAME_BASE>(packet, base);
					Game::net_SetBase(base);
					break;
				}

				case pTypes::ID_GAME_DELETED:
				{
					Game::DeletedObjects deletedStatic;
					PacketFactory::Access<pTypes::ID_GAME_DELETED>(packet, deletedStatic);
					Game::net_SetDeletedStatic(move(deletedStatic));
					break;
				}

				case pTypes::ID_REFERENCE_NEW:
					GameFactory::Create<Reference, FailPolicy::Exception>(packet);
					/*GameFactory::Operate<Reference>(GameFactory::Create<Reference>(packet.get()), [](FactoryReference& reference) {
					});*/ break;

				case pTypes::ID_OBJECT_NEW:
					GameFactory::Operate<Object, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<Object, FailPolicy::Exception>(packet), [](FactoryObject& object) {
						Game::NewObject(object);
					}); break;

				case pTypes::ID_VOLATILE_NEW:
				{
					NetworkID id;
					unsigned int baseID;
					float aX, aY, aZ;
					PacketFactory::Access<pTypes::ID_VOLATILE_NEW>(packet, id, baseID, aX, aY, aZ);
					auto reference = GameFactory::Get<Object>(id);
					Game::NewVolatile(reference.get(), baseID, aX, aY, aZ);
					break;
				}

				case pTypes::ID_ITEM_NEW:
					GameFactory::Operate<Item, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<Item, FailPolicy::Exception>(packet), [](FactoryItem& item) {
						Game::NewItem(item);
					}); break;

				case pTypes::ID_CONTAINER_NEW:
					GameFactory::Operate<Container, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<Container, FailPolicy::Exception>(packet), [](FactoryContainer& container) {
						Game::NewContainer(container);
					}); break;

				case pTypes::ID_ACTOR_NEW:
					GameFactory::Operate<Actor, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<Actor, FailPolicy::Exception>(packet), [](FactoryActor& actor) {
						Game::NewActor(actor);
					}); break;

				case pTypes::ID_PLAYER_NEW:
					GameFactory::Operate<Player, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<Player, FailPolicy::Exception>(packet), [](FactoryPlayer& player) {
						Game::NewPlayer(player);
					}); break;

				case pTypes::ID_OBJECT_REMOVE:
				{
					NetworkID id;
					bool silent;
					PacketFactory::Access<pTypes::ID_OBJECT_REMOVE>(packet, id, silent);
					auto reference = GameFactory::Get<Object>(id);
					Game::DestroyObject(reference.get(), silent);
					break;
				}

				case pTypes::ID_WINDOW_NEW:
					GameFactory::Operate<Window, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<Window, FailPolicy::Exception>(packet), [](FactoryWindow& window) {
						Game::NewWindow(window);
					}); break;

				case pTypes::ID_BUTTON_NEW:
					GameFactory::Operate<Button, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<Button, FailPolicy::Exception>(packet), [](FactoryButton& button) {
						Game::NewButton(button);
					}); break;

				case pTypes::ID_TEXT_NEW:
					GameFactory::Operate<Text, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<Text, FailPolicy::Exception>(packet), [](FactoryText& text) {
						Game::NewText(text);
					}); break;

				case pTypes::ID_EDIT_NEW:
					GameFactory::Operate<Edit, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<Edit, FailPolicy::Exception>(packet), [](FactoryEdit& edit) {
						Game::NewEdit(edit);
					}); break;

				case pTypes::ID_CHECKBOX_NEW:
					GameFactory::Operate<Checkbox, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<Checkbox, FailPolicy::Exception>(packet), [](FactoryCheckbox& checkbox) {
						Game::NewCheckbox(checkbox);
					}); break;

				case pTypes::ID_RADIOBUTTON_NEW:
					GameFactory::Operate<RadioButton, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<RadioButton, FailPolicy::Exception>(packet), [](FactoryRadioButton& radiobutton) {
						Game::NewRadioButton(radiobutton);
					}); break;

				case pTypes::ID_LISTITEM_NEW:
					GameFactory::Operate<ListItem, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<ListItem, FailPolicy::Exception>(packet), [](FactoryListItem& listitem) {
						Game::NewListItem(listitem);
					}); break;

				case pTypes::ID_LIST_NEW:
					GameFactory::Operate<List, EXCEPTION_FACTORY_VALIDATED>(GameFactory::Create<List, FailPolicy::Exception>(packet), [](FactoryList& list) {
						Game::NewList(list);
					}); break;

				case pTypes::ID_WINDOW_REMOVE:
				{
					NetworkID id;
					PacketFactory::Access<pTypes::ID_WINDOW_REMOVE>(packet, id);
					auto reference = GameFactory::Get<Window>(id);
					Game::DestroyWindow(reference.get());
					break;
				}

				case pTypes::ID_LISTITEM_REMOVE:
				{
					NetworkID id;
					PacketFactory::Access<pTypes::ID_LISTITEM_REMOVE>(packet, id);
					auto reference = GameFactory::Get<ListItem>(id);
					Game::DestroyListItem(reference.get());
					break;
				}

				case pTypes::ID_UPDATE_NAME:
				{
					NetworkID id;
					string name;
					PacketFactory::Access<pTypes::ID_UPDATE_NAME>(packet, id, name);
					auto reference = GameFactory::Get<Object>(id);
					Game::net_SetName(reference.get(), move(name));
					break;
				}

				case pTypes::ID_UPDATE_POS:
				{
					NetworkID id;
					float X, Y, Z;
					PacketFactory::Access<pTypes::ID_UPDATE_POS>(packet, id, X, Y, Z);
					auto reference = GameFactory::Get<Object>(id);
					Game::net_SetPos(reference.get(), X, Y, Z);
					break;
				}

				case pTypes::ID_UPDATE_ANGLE:
				{
					NetworkID id;
					float X, Z;
					PacketFactory::Access<pTypes::ID_UPDATE_ANGLE>(packet, id, X, Z);
					auto reference = GameFactory::Get<Object>(id);
					Game::net_SetAngle(reference.get(), X, 0.00, Z);
					break;
				}

				case pTypes::ID_UPDATE_CELL:
				{
					NetworkID id;
					unsigned int cell;
					float X, Y, Z;
					PacketFactory::Access<pTypes::ID_UPDATE_CELL>(packet, id, cell, X, Y, Z);
					auto reference = GameFactory::Get<Object>({id, GameFactory::Lookup<Object>(PLAYER_REFERENCE)});
					auto player = vaultcast_swap<Player>(move(reference[1]));
					Game::net_SetCell(reference[0].get(), player.get(), cell, X, Y, Z);
					break;
				}

				case pTypes::ID_UPDATE_LOCK:
				{
					NetworkID id;
					unsigned int lock;
					PacketFactory::Access<pTypes::ID_UPDATE_LOCK>(packet, id, lock);
					auto reference = GameFactory::Get<Object>(id);
					Game::net_SetLock(reference.get(), lock);
					break;
				}

				case pTypes::ID_UPDATE_OWNER:
				{
					NetworkID id;
					unsigned int owner;
					PacketFactory::Access<pTypes::ID_UPDATE_OWNER>(packet, id, owner);
					auto reference = GameFactory::Get<Object>(id);
					Game::net_SetOwner(reference.get(), owner);
					break;
				}

				case pTypes::ID_UPDATE_ACTIVATE:
				{
					NetworkID id, actor;
					PacketFactory::Access<pTypes::ID_UPDATE_ACTIVATE>(packet, id, actor);
					auto reference = GameFactory::Get<Reference>({id, actor});
					Game::net_GetActivate(reference[0].get(), reference[1].get());
					break;
				}

				case pTypes::ID_UPDATE_COUNT:
				{
					NetworkID id;
					unsigned int count;
					bool silent;
					PacketFactory::Access<pTypes::ID_UPDATE_COUNT>(packet, id, count, silent);
					auto reference = GameFactory::Get<Item>(id);
					Game::net_SetItemCount(reference.get(), count, silent);
					break;
				}

				case pTypes::ID_UPDATE_CONDITION:
				{
					NetworkID id;
					float condition;
					unsigned int health;
					PacketFactory::Access<pTypes::ID_UPDATE_CONDITION>(packet, id, condition, health);
					auto reference = GameFactory::Get<Item>(id);
					Game::net_SetItemCondition(reference.get(), condition, health);
					break;
				}

				case pTypes::ID_UPDATE_EQUIPPED:
				{
					NetworkID id;
					bool equipped, silent, stick;
					PacketFactory::Access<pTypes::ID_UPDATE_EQUIPPED>(packet, id, equipped, silent, stick);
					auto reference = GameFactory::Get<Item>(id);
					Game::net_SetItemEquipped(reference.get(), equipped, silent, stick);
					break;
				}

				case pTypes::ID_UPDATE_VALUE:
				{
					NetworkID id;
					bool base;
					unsigned char index;
					float value;
					PacketFactory::Access<pTypes::ID_UPDATE_VALUE>(packet, id, base, index, value);
					auto reference = GameFactory::Get<Actor>(id);
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
					auto reference = GameFactory::Get<Actor>(id);
					Game::net_SetActorState(reference.get(), idle, moving, movingxy, weapon, alerted, sneaking, firing);
					break;
				}

				case pTypes::ID_UPDATE_RACE:
				{
					NetworkID id;
					unsigned int race;
					signed int age, delta_age;
					PacketFactory::Access<pTypes::ID_UPDATE_RACE>(packet, id, race, age, delta_age);
					auto reference = GameFactory::Get<Actor>(id);
					Game::net_SetActorRace(reference.get(), race, age, delta_age);
					break;
				}

				case pTypes::ID_UPDATE_SEX:
				{
					NetworkID id;
					bool female;
					PacketFactory::Access<pTypes::ID_UPDATE_SEX>(packet, id, female);
					auto reference = GameFactory::Get<Actor>(id);
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
					auto reference = GameFactory::Get<Actor>(id);
					Game::net_SetActorDead(reference.get(), dead, limbs, cause);
					break;
				}

				case pTypes::ID_UPDATE_FIREWEAPON:
				{
					NetworkID id;
					unsigned int weapon;
					PacketFactory::Access<pTypes::ID_UPDATE_FIREWEAPON>(packet, id, weapon);
					auto reference = GameFactory::Get<Actor>(id);
					Game::net_FireWeapon(reference.get(), weapon);
					break;
				}

				case pTypes::ID_UPDATE_SOUND:
				{
					NetworkID id;
					unsigned int sound;
					PacketFactory::Access<pTypes::ID_UPDATE_SOUND>(packet, id, sound);
					auto reference = GameFactory::Get<Object>(id);
					Game::net_PlaySound(reference.get(), sound);
					break;
				}

				case pTypes::ID_UPDATE_IDLE:
				{
					NetworkID id;
					unsigned int idle;
					string name;
					PacketFactory::Access<pTypes::ID_UPDATE_IDLE>(packet, id, idle, name);
					auto reference = GameFactory::Get<Actor>(id);
					Game::net_SetActorIdle(reference.get(), idle, name);
					break;
				}

				case pTypes::ID_UPDATE_INTERIOR:
				{
					NetworkID id;
					string cell;
					bool spawn;
					PacketFactory::Access<pTypes::ID_UPDATE_INTERIOR>(packet, id, cell, spawn);
					Game::net_UpdateInterior(cell, spawn);
					break;
				}

				case pTypes::ID_UPDATE_EXTERIOR:
				{
					NetworkID id;
					unsigned int baseID;
					signed int x, y;
					bool spawn;
					PacketFactory::Access<pTypes::ID_UPDATE_EXTERIOR>(packet, id, baseID, x, y, spawn);
					Game::net_UpdateExterior(baseID, x, y, spawn);
					break;
				}

				case pTypes::ID_UPDATE_CONTEXT:
				{
					NetworkID id;
					Player::CellContext context;
					bool spawn;
					PacketFactory::Access<pTypes::ID_UPDATE_CONTEXT>(packet, id, context, spawn);
					Game::net_UpdateContext(context, spawn);
					break;
				}

				case pTypes::ID_UPDATE_CONSOLE:
				{
					NetworkID id;
					bool enabled;
					PacketFactory::Access<pTypes::ID_UPDATE_CONSOLE>(packet, id, enabled);
					Game::net_UpdateConsole(enabled);
					break;
				}

				case pTypes::ID_UPDATE_WPOS:
				{
					NetworkID id;
					tuple<float, float, float, float> pos;
					PacketFactory::Access<pTypes::ID_UPDATE_WPOS>(packet, id, pos);
					auto reference = GameFactory::Get<Window>(id);
					Game::net_UpdateWindowPos(reference.get(), pos);
					break;
				}

				case pTypes::ID_UPDATE_WSIZE:
				{
					NetworkID id;
					tuple<float, float, float, float> size;
					PacketFactory::Access<pTypes::ID_UPDATE_WSIZE>(packet, id, size);
					auto reference = GameFactory::Get<Window>(id);
					Game::net_UpdateWindowSize(reference.get(), size);
					break;
				}

				case pTypes::ID_UPDATE_WVISIBLE:
				{
					NetworkID id;
					bool visible;
					PacketFactory::Access<pTypes::ID_UPDATE_WVISIBLE>(packet, id, visible);
					auto reference = GameFactory::Get<Window>(id);
					Game::net_UpdateWindowVisible(reference.get(), visible);
					break;
				}

				case pTypes::ID_UPDATE_WLOCKED:
				{
					NetworkID id;
					bool locked;
					PacketFactory::Access<pTypes::ID_UPDATE_WVISIBLE>(packet, id, locked);
					auto reference = GameFactory::Get<Window>(id);
					Game::net_UpdateWindowLocked(reference.get(), locked);
					break;
				}

				case pTypes::ID_UPDATE_WTEXT:
				{
					NetworkID id;
					string text;
					PacketFactory::Access<pTypes::ID_UPDATE_WTEXT>(packet, id, text);
					auto reference = GameFactory::Get<Window>(id);
					Game::net_UpdateWindowText(reference.get(), text);
					break;
				}

				case pTypes::ID_UPDATE_WMAXLEN:
				{
					NetworkID id;
					unsigned int length;
					PacketFactory::Access<pTypes::ID_UPDATE_WMAXLEN>(packet, id, length);
					auto reference = GameFactory::Get<Edit>(id);
					Game::net_UpdateEditMaxLength(reference.get(), length);
					break;
				}

				case pTypes::ID_UPDATE_WVALID:
				{
					NetworkID id;
					string validation;
					PacketFactory::Access<pTypes::ID_UPDATE_WVALID>(packet, id, validation);
					auto reference = GameFactory::Get<Edit>(id);
					Game::net_UpdateEditValidation(reference.get(), validation);
					break;
				}

				case pTypes::ID_UPDATE_WSELECTED:
				{
					NetworkID id;
					bool selected;
					PacketFactory::Access<pTypes::ID_UPDATE_WSELECTED>(packet, id, selected);
					auto reference = GameFactory::Get<Checkbox>(id);
					Game::net_UpdateCheckboxSelected(reference.get(), selected);
					break;
				}

				case pTypes::ID_UPDATE_WRSELECTED:
				{
					NetworkID id, previous;
					bool selected;
					PacketFactory::Access<pTypes::ID_UPDATE_WRSELECTED>(packet, id, previous, selected);
					auto reference = GameFactory::Get<RadioButton>({id, previous});
					Game::net_UpdateRadioButtonSelected(reference[0].get(), reference[1], selected);
					break;
				}

				case pTypes::ID_UPDATE_WGROUP:
				{
					NetworkID id;
					unsigned int group;
					PacketFactory::Access<pTypes::ID_UPDATE_WGROUP>(packet, id, group);
					auto reference = GameFactory::Get<RadioButton>(id);
					Game::net_UpdateRadioButtonGroup(reference.get(), group);
					break;
				}

				case pTypes::ID_UPDATE_WLSELECTED:
				{
					NetworkID id;
					bool selected;
					PacketFactory::Access<pTypes::ID_UPDATE_WLSELECTED>(packet, id, selected);
					auto reference = GameFactory::Get<ListItem>(id);
					Game::net_UpdateListItemSelected(reference.get(), selected);
					break;
				}

				case pTypes::ID_UPDATE_WLTEXT:
				{
					NetworkID id;
					string text;
					PacketFactory::Access<pTypes::ID_UPDATE_WLTEXT>(packet, id, text);
					auto reference = GameFactory::Get<ListItem>(id);
					Game::net_UpdateListItemText(reference.get(), text);
					break;
				}

				case pTypes::ID_UPDATE_WLMULTI:
				{
					NetworkID id;
					bool multiselect;
					PacketFactory::Access<pTypes::ID_UPDATE_WLMULTI>(packet, id, multiselect);
					auto reference = GameFactory::Get<List>(id);
					Game::net_UpdateListMultiSelect(reference.get(), multiselect);
					break;
				}

				case pTypes::ID_UPDATE_WMODE:
				{
					bool enabled;
					PacketFactory::Access<pTypes::ID_UPDATE_WMODE>(packet, enabled);
					Game::net_UpdateWindowMode(enabled);
					break;
				}

				default:
					throw VaultException("Unhandled packet type %d", data->data[0]).stacktrace();
			}
		}
	}

	return response;
}
