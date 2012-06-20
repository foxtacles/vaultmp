#include "PacketFactory.h"
#include "PacketTypes.h"

pPacket PacketFactory::CreatePacket(unsigned char type, ...)
{
	va_list args;
	va_start(args, type);
	pDefault* packet;

	switch (type)
	{
		case ID_GAME_AUTH:
		{
			const char* name = va_arg(args, const char*);
			const char* pwd = va_arg(args, const char*);
			packet = new pGameAuth(name, pwd);
			break;
		}

		case ID_GAME_LOAD:
		{
			packet = new pGameLoad();
			break;
		}

		case ID_GAME_MOD:
		{
			const char* modfile = va_arg(args, const char*);
			unsigned int crc = va_arg(args, unsigned int);
			packet = new pGameMod(modfile, crc);
			break;
		}

		case ID_GAME_START:
		{
			const char* savegame = va_arg(args, const char*);
			unsigned int crc = va_arg(args, unsigned int);
			packet = new pGameStart(savegame, crc);
			break;
		}

		case ID_GAME_END:
		{
			unsigned char reason = (unsigned char) va_arg(args, unsigned int);
			packet = new pGameEnd(reason);
			break;
		}

		case ID_GAME_MESSAGE:
		{
			const char* message = va_arg(args, const char*);
			packet = new pGameMessage(message);
			break;
		}

		case ID_GAME_CHAT:
		{
			const char* message = va_arg(args, const char*);
			packet = new pGameChat(message);
			break;
		}

		case ID_OBJECT_NEW:
		{
			NetworkID id = va_arg(args, NetworkID);
			unsigned int refID = va_arg(args, unsigned int);
			unsigned int baseID = va_arg(args, unsigned int);
			char* name = va_arg(args, char*);
			double X = va_arg(args, double);
			double Y = va_arg(args, double);
			double Z = va_arg(args, double);
			double aX = va_arg(args, double);
			double aY = va_arg(args, double);
			double aZ = va_arg(args, double);
			unsigned int cell = va_arg(args, unsigned int);
			bool enabled = (bool) va_arg(args, unsigned int);
			packet = new pObjectNew(id, refID, baseID, name, X, Y, Z, aX, aY, aZ, cell, enabled);
			break;
		}

		case ID_ITEM_NEW:
		{
			const pDefault* pObjectNew = va_arg(args, pDefault*);
			unsigned int count = va_arg(args, unsigned int);
			double condition = va_arg(args, double);
			bool equipped = (bool) va_arg(args, unsigned int);
			bool silent = (bool) va_arg(args, unsigned int);
			bool stick = (bool) va_arg(args, unsigned int);
			packet = new pItemNew(pObjectNew, count, condition, equipped, silent, stick);
			break;
		}

		case ID_CONTAINER_NEW:
		{
			const pDefault* pObjectNew = va_arg(args, pDefault*);
			vector<pPacket>* pItemNew = va_arg(args, vector<pPacket>*);
			packet = new pContainerNew(pObjectNew, *pItemNew);
			break;
		}

		case ID_ACTOR_NEW:
		{
			const pDefault* pContainerNew = va_arg(args, pDefault*);
			map<unsigned char, double>* values = (map<unsigned char, double>*) va_arg(args, void*); // compile error when placing map<unsigned char, double>* as 2nd argument?
			map<unsigned char, double>* baseValues = (map<unsigned char, double>*) va_arg(args, void*);
			unsigned char moving = (unsigned char) va_arg(args, unsigned int);
			unsigned char moving_xy = (unsigned char) va_arg(args, unsigned int);
			bool alerted = (bool) va_arg(args, unsigned int);
			bool sneaking = (bool) va_arg(args, unsigned int);
			bool dead = (bool) va_arg(args, unsigned int);
			packet = new pActorNew(pContainerNew, *values, *baseValues, moving, moving_xy, alerted, sneaking, dead);
			break;
		}

		case ID_PLAYER_NEW:
		{
			const pDefault* pActorNew = va_arg(args, pDefault*);
			map<unsigned char, pair<unsigned char, bool> >* controls = (map<unsigned char, pair<unsigned char, bool> >*) va_arg(args, void*);
			packet = new pPlayerNew(pActorNew, *controls);
			break;
		}

		case ID_OBJECT_REMOVE:
		{
			NetworkID id = va_arg(args, NetworkID);
			packet = new pObjectRemove(id);
			break;
		}

		case ID_UPDATE_POS:
		{
			NetworkID id = va_arg(args, NetworkID);
			double X = va_arg(args, double);
			double Y = va_arg(args, double);
			double Z = va_arg(args, double);
			packet = new pObjectPos(id, X, Y, Z);
			break;
		}

		case ID_UPDATE_ANGLE:
		{
			NetworkID id = va_arg(args, NetworkID);
			unsigned char axis = (unsigned char) va_arg(args, unsigned int);
			double value = va_arg(args, double);
			packet = new pObjectAngle(id, axis, value);
			break;
		}

		case ID_UPDATE_CELL:
		{
			NetworkID id = va_arg(args, NetworkID);
			unsigned int cell = va_arg(args, unsigned int);
			packet = new pObjectCell(id, cell);
			break;
		}

		case ID_UPDATE_CONTAINER:
		{
			NetworkID id = va_arg(args, NetworkID);
			const ContainerDiff* diff = va_arg(args, ContainerDiff*);
			packet = new pContainerUpdate(id, *diff);
			break;
		}

		case ID_UPDATE_VALUE:
		{
			NetworkID id = va_arg(args, NetworkID);
			bool base = (bool) va_arg(args, unsigned int);
			unsigned char index = (unsigned char) va_arg(args, unsigned int);
			double value = va_arg(args, double);
			packet = new pActorValue(id, base, index, value);
			break;
		}

		case ID_UPDATE_STATE:
		{
			NetworkID id = va_arg(args, NetworkID);
			unsigned char moving = (unsigned char) va_arg(args, unsigned int);
			unsigned char movingxy = (unsigned char) va_arg(args, unsigned int);
			unsigned char weapon = (unsigned char) va_arg(args, unsigned int);
			bool alerted = (bool) va_arg(args, unsigned int);
			bool sneaking = (bool) va_arg(args, unsigned int);
			packet = new pActorState(id, moving, movingxy, weapon, alerted, sneaking);
			break;
		}

		case ID_UPDATE_DEAD:
		{
			NetworkID id = va_arg(args, NetworkID);
			bool dead = (bool) va_arg(args, unsigned int);
			packet = new pActorDead(id, dead);
			break;
		}

		case ID_UPDATE_CONTROL:
		{
			NetworkID id = va_arg(args, NetworkID);
			unsigned char control = (unsigned char) va_arg(args, unsigned int);
			unsigned char key = (unsigned char) va_arg(args, unsigned int);
			packet = new pPlayerControl(id, control, key);
			break;
		}

		default:
			throw VaultException("Unhandled packet type %d", (int) type);
	}

	va_end(args);

	return pPacket(packet, FreePacket);
}

pPacket PacketFactory::CreatePacket(unsigned char* stream, unsigned int len)
{
	pDefault* packet;

	switch (stream[0])
	{
		case ID_GAME_AUTH:
			packet = new pGameAuth(stream, len);
			break;

		case ID_GAME_LOAD:
			packet = new pGameLoad(stream, len);
			break;

		case ID_GAME_MOD:
			packet = new pGameMod(stream, len);
			break;

		case ID_GAME_START:
			packet = new pGameStart(stream, len);
			break;

		case ID_GAME_END:
			packet = new pGameEnd(stream, len);
			break;

		case ID_GAME_MESSAGE:
			packet = new pGameMessage(stream, len);
			break;

		case ID_GAME_CHAT:
			packet = new pGameChat(stream, len);
			break;

		case ID_OBJECT_NEW:
			packet = new pObjectNew(stream, len);
			break;

		case ID_ITEM_NEW:
			packet = new pItemNew(stream, len);
			break;

		case ID_CONTAINER_NEW:
			packet = new pContainerNew(stream, len);
			break;

		case ID_ACTOR_NEW:
			packet = new pActorNew(stream, len);
			break;

		case ID_PLAYER_NEW:
			packet = new pPlayerNew(stream, len);
			break;

		case ID_OBJECT_REMOVE:
			packet = new pObjectRemove(stream, len);
			break;

		case ID_OBJECT_UPDATE:
		case ID_CONTAINER_UPDATE:
		case ID_ACTOR_UPDATE:
		case ID_PLAYER_UPDATE:
		{
			if (len < 2)
				throw VaultException("Incomplete object packet type %d", (int) stream[0]);

			switch (stream[1])
			{
				case ID_UPDATE_POS:
					packet = new pObjectPos(stream, len);
					break;

				case ID_UPDATE_ANGLE:
					packet = new pObjectAngle(stream, len);
					break;

				case ID_UPDATE_CELL:
					packet = new pObjectCell(stream, len);
					break;

				case ID_UPDATE_CONTAINER:
					packet = new pContainerUpdate(stream, len);
					break;

				case ID_UPDATE_VALUE:
					packet = new pActorValue(stream, len);
					break;

				case ID_UPDATE_STATE:
					packet = new pActorState(stream, len);
					break;

				case ID_UPDATE_DEAD:
					packet = new pActorDead(stream, len);
					break;

				case ID_UPDATE_CONTROL:
					packet = new pPlayerControl(stream, len);
					break;

				default:
					throw VaultException("Unhandled object update packet type %d", (int) stream[1]);
			}

			break;
		}

		default:
			throw VaultException("Unhandled packet type %d", (int) stream[0]);
	}

	return pPacket(packet, FreePacket);
}

void PacketFactory::Access(const pDefault* packet, ...)
{
	va_list args;
	va_start(args, packet);

	try
	{
		switch (packet->type.type)
		{
			case ID_GAME_AUTH:
			{
				const pGameAuth* data = dynamic_cast<const pGameAuth*>(packet);
				char* name = va_arg(args, char*);
				char* pwd = va_arg(args, char*);
				strncpy(name, data->_data.name, sizeof(data->_data.name));
				strncpy(pwd, data->_data.pwd, sizeof(data->_data.pwd));
				break;
			}

			case ID_GAME_LOAD:
			{
				//const pGameLoad* data = dynamic_cast<const pGameLoad*>(packet);
				break;
			}

			case ID_GAME_MOD:
			{
				const pGameMod* data = dynamic_cast<const pGameMod*>(packet);
				char* modfile = va_arg(args, char*);
				unsigned int* crc = va_arg(args, unsigned int*);
				strncpy(modfile, data->_data.modfile, sizeof(data->_data.modfile));
				*crc = data->_data.crc;
				break;
			}

			case ID_GAME_START:
			{
				const pGameStart* data = dynamic_cast<const pGameStart*>(packet);
				char* savegame = va_arg(args, char*);
				unsigned int* crc = va_arg(args, unsigned int*);
				strncpy(savegame, data->_data.savegame, sizeof(data->_data.savegame));
				*crc = data->_data.crc;
				break;
			}

			case ID_GAME_END:
			{
				const pGameEnd* data = dynamic_cast<const pGameEnd*>(packet);
				unsigned char* reason = va_arg(args, unsigned char*);
				*reason = data->reason.type;
				break;
			}

			case ID_GAME_MESSAGE:
			{
				const pGameMessage* data = dynamic_cast<const pGameMessage*>(packet);
				char* message = va_arg(args, char*);
				strncpy(message, data->message, sizeof(data->message));
				break;
			}

			case ID_GAME_CHAT:
			{
				const pGameChat* data = dynamic_cast<const pGameChat*>(packet);
				char* message = va_arg(args, char*);
				strncpy(message, data->message, sizeof(data->message));
				break;
			}

			case ID_OBJECT_NEW:
			{
				const pObjectNew* data = dynamic_cast<const pObjectNew*>(packet);
				NetworkID* id = va_arg(args, NetworkID*);
				unsigned int* refID = va_arg(args, unsigned int*);
				unsigned int* baseID = va_arg(args, unsigned int*);
				char* name = va_arg(args, char*);
				double* X = va_arg(args, double*);
				double* Y = va_arg(args, double*);
				double* Z = va_arg(args, double*);
				double* aX = va_arg(args, double*);
				double* aY = va_arg(args, double*);
				double* aZ = va_arg(args, double*);
				unsigned int* cell = va_arg(args, unsigned int*);
				bool* enabled = va_arg(args, bool*);
				*id = data->id;
				*refID = data->refID;
				*baseID = data->baseID;
				strncpy(name, data->_data.name, sizeof(data->_data.name));
				*X = data->_data.X;
				*Y = data->_data.Y;
				*Z = data->_data.Z;
				*aX = data->_data.aX;
				*aY = data->_data.aY;
				*aZ = data->_data.aZ;
				*cell = data->_data.cell;
				*enabled = data->_data.enabled;
				break;
			}

			case ID_ITEM_NEW:
			{
				const pItemNew* data = dynamic_cast<const pItemNew*>(packet);
				unsigned int* count = va_arg(args, unsigned int*);
				double* condition = va_arg(args, double*);
				bool* equipped = va_arg(args, bool*);
				bool* silent = va_arg(args, bool*);
				bool* stick = va_arg(args, bool*);
				*count = data->_data.count;
				*condition = data->_data.condition;
				*equipped = data->_data.equipped;
				*silent = data->_data.silent;
				*stick = data->_data.stick;
				break;
			}

			case ID_CONTAINER_NEW:
			{
				const pContainerNew* data = dynamic_cast<const pContainerNew*>(packet);
				vector<pPacket>* _pItemNew = va_arg(args, vector<pPacket>*);

				_pItemNew->clear();
				unsigned int length = pItemNew::as_packet_length();
				unsigned int size = *reinterpret_cast<unsigned int*>(&data->_data[pObjectNew::data_length()]);
				unsigned int at = pObjectNew::data_length() + sizeof(unsigned int);

				_pItemNew->reserve(size);
				for (unsigned int i = 0; i < size; ++i, at += length)
					_pItemNew->push_back(pPacket(new pItemNew(&data->_data[at], length), FreePacket));

				break;
			}

			case ID_ACTOR_NEW:
			{
				const pActorNew* data = dynamic_cast<const pActorNew*>(packet);
				map<unsigned char, double>* values = (map<unsigned char, double>*) va_arg(args, void*);
				map<unsigned char, double>* baseValues = (map<unsigned char, double>*) va_arg(args, void*);
				unsigned char* moving = va_arg(args, unsigned char*);
				unsigned char* moving_xy = va_arg(args, unsigned char*);
				bool* alerted = va_arg(args, bool*);
				bool* sneaking = va_arg(args, bool*);
				bool* dead = va_arg(args, bool*);

				values->clear();
				baseValues->clear();
				unsigned int length = sizeof(unsigned char) + sizeof(double);
				unsigned int size = *reinterpret_cast<unsigned int*>(&data->_data[pContainerNew::data_length(data->_data)]);
				unsigned int at = pContainerNew::data_length(data->_data) + sizeof(unsigned int);

				for (unsigned int i = 0; i < size; ++i, at += length)
					values->insert(pair<unsigned char, double>(*reinterpret_cast<unsigned char*>(&data->_data[at]), *reinterpret_cast<double*>(&data->_data[at + sizeof(unsigned char)])));

				for (unsigned int i = 0; i < size; ++i, at += length)
					baseValues->insert(pair<unsigned char, double>(*reinterpret_cast<unsigned char*>(&data->_data[at]), *reinterpret_cast<double*>(&data->_data[at + sizeof(unsigned char)])));

				*moving = *reinterpret_cast<unsigned char*>(&data->_data[at]);
				at += sizeof(unsigned char);

				*moving_xy = *reinterpret_cast<unsigned char*>(&data->_data[at]);
				at += sizeof(unsigned char);

				*alerted = *reinterpret_cast<bool*>(&data->_data[at]);
				at += sizeof(bool);

				*sneaking = *reinterpret_cast<bool*>(&data->_data[at]);
				at += sizeof(bool);

				*dead = *reinterpret_cast<bool*>(&data->_data[at]);
				break;
			}

			case ID_PLAYER_NEW:
			{
				const pPlayerNew* data = dynamic_cast<const pPlayerNew*>(packet);
				map<unsigned char, pair<unsigned char, bool> >* controls = (map<unsigned char, pair<unsigned char, bool> >*) va_arg(args, void*);

				controls->clear();
				unsigned int length = sizeof(unsigned char) + sizeof(unsigned char) + sizeof(bool);
				unsigned int size = *reinterpret_cast<unsigned int*>(&data->_data[pActorNew::data_length(data->_data)]);
				unsigned int at = pActorNew::data_length(data->_data) + sizeof(unsigned int);

				for (unsigned int i = 0; i < size; ++i, at += length)
					controls->insert(pair<unsigned char, pair<unsigned char, bool> >(*reinterpret_cast<unsigned char*>(&data->_data[at]), pair<unsigned char, bool>(*reinterpret_cast<double*>(&data->_data[at + sizeof(unsigned char)]), *reinterpret_cast<double*>(&data->_data[at + (sizeof(unsigned char) * 2)]))));

				break;
			}

			case ID_OBJECT_REMOVE:
			{
				const pObjectRemove* data = dynamic_cast<const pObjectRemove*>(packet);
				NetworkID* id = va_arg(args, NetworkID*);
				*id = data->id;
				break;
			}

			case ID_OBJECT_UPDATE:
			case ID_CONTAINER_UPDATE:
			case ID_ACTOR_UPDATE:
			case ID_PLAYER_UPDATE:
			{
				const pObjectUpdateDefault* data = dynamic_cast<const pObjectUpdateDefault*>(packet);

				switch (data->sub_type.type)
				{
					case ID_UPDATE_POS:
					{
						const pObjectPos* update = dynamic_cast<const pObjectPos*>(data);
						NetworkID* id = va_arg(args, NetworkID*);
						double* X = va_arg(args, double*);
						double* Y = va_arg(args, double*);
						double* Z = va_arg(args, double*);
						*id = update->id;
						*X = update->_data.X;
						*Y = update->_data.Y;
						*Z = update->_data.Z;
						break;
					}

					case ID_UPDATE_ANGLE:
					{
						const pObjectAngle* update = dynamic_cast<const pObjectAngle*>(data);
						NetworkID* id = va_arg(args, NetworkID*);
						unsigned char* axis = va_arg(args, unsigned char*);
						double* value = va_arg(args, double*);
						*id = update->id;
						*axis = update->_data.axis;
						*value = update->_data.value;
						break;
					}

					case ID_UPDATE_CELL:
					{
						const pObjectCell* update = dynamic_cast<const pObjectCell*>(data);
						NetworkID* id = va_arg(args, NetworkID*);
						unsigned int* cell = va_arg(args, unsigned int*);
						*id = update->id;
						*cell = update->cell;
						break;
					}

					case ID_UPDATE_CONTAINER:
					{
						const pContainerUpdate* update = dynamic_cast<const pContainerUpdate*>(data);
						NetworkID* id = va_arg(args, NetworkID*);
						ContainerDiff* diff = va_arg(args, ContainerDiff*);

						*id = update->id;

						diff->first.clear();
						diff->second.clear();
						unsigned int length = sizeof(NetworkID);
						unsigned int size = *reinterpret_cast<unsigned int*>(&update->_data[0]);
						unsigned int at = sizeof(unsigned int);

						for (unsigned int i = 0; i < size; ++i, at += length)
							diff->first.push_back(*reinterpret_cast<NetworkID*>(&update->_data[at]));

						length = pItemNew::as_packet_length();
						size = *reinterpret_cast<unsigned int*>(&update->_data[at]);
						at += sizeof(unsigned int);

						for (unsigned int i = 0; i < size; ++i, at += length)
						{
							pItemNew* item = new pItemNew(&update->_data[at], length);
							NetworkID id = GameFactory::CreateKnownInstance(ID_ITEM, item);
							diff->second.push_back(id);
							PacketFactory::FreePacket(item);
						}

						break;
					}

					case ID_UPDATE_VALUE:
					{
						const pActorValue* update = dynamic_cast<const pActorValue*>(data);
						NetworkID* id = va_arg(args, NetworkID*);
						bool* base = va_arg(args, bool*);
						unsigned char* index = va_arg(args, unsigned char*);
						double* value = va_arg(args, double*);
						*id = update->id;
						*base = update->_data.base;
						*index = update->_data.index;
						*value = update->_data.value;
						break;
					}

					case ID_UPDATE_STATE:
					{
						const pActorState* update = dynamic_cast<const pActorState*>(data);
						NetworkID* id = va_arg(args, NetworkID*);
						unsigned char* moving = va_arg(args, unsigned char*);
						unsigned char* movingxy = va_arg(args, unsigned char*);
						unsigned char* weapon = va_arg(args, unsigned char*);
						bool* alerted = va_arg(args, bool*);
						bool* sneaking = va_arg(args, bool*);
						*id = update->id;
						*moving = update->_data.moving;
						*movingxy = update->_data.movingxy;
						*weapon = update->_data.weapon;
						*alerted = update->_data.alerted;
						*sneaking = update->_data.sneaking;
						break;
					}

					case ID_UPDATE_DEAD:
					{
						const pActorDead* update = dynamic_cast<const pActorDead*>(data);
						NetworkID* id = va_arg(args, NetworkID*);
						bool* dead = va_arg(args, bool*);
						*id = update->id;
						*dead = update->_data.dead;
						break;
					}

					case ID_UPDATE_CONTROL:
					{
						const pPlayerControl* update = dynamic_cast<const pPlayerControl*>(data);
						NetworkID* id = va_arg(args, NetworkID*);
						unsigned char* control = va_arg(args, unsigned char*);
						unsigned char* key = va_arg(args, unsigned char*);
						*id = update->id;
						*control = update->_data.control;
						*key = update->_data.key;
						break;
					}
				}

				break;
			}

			default:
				throw VaultException("Unhandled packet type %d", (int) packet->type.type);
		}
	}

	catch (...)
	{
		va_end(args);
		throw;
	}

	va_end(args);
}

NetworkID PacketFactory::ExtractNetworkID(const pDefault* packet)
{
	const pObjectDefault* data = dynamic_cast<const pObjectDefault*>(packet);
	return data->id;
}

unsigned int PacketFactory::ExtractReference(const pDefault* packet)
{
	const pObjectNewDefault* data = dynamic_cast<const pObjectNewDefault*>(packet);
	return data->refID;
}

unsigned int PacketFactory::ExtractBase(const pDefault* packet)
{
	const pObjectNewDefault* data = dynamic_cast<const pObjectNewDefault*>(packet);
	return data->baseID;
}

const unsigned char* PacketFactory::ExtractRawData(const pDefault* packet)
{
	switch (packet->type.type)
	{
		case ID_OBJECT_NEW:
		{
			const pObjectNew* data = dynamic_cast<const pObjectNew*>(packet);
			return reinterpret_cast<const unsigned char*>(&data->_data);
		}

		case ID_CONTAINER_NEW:
		{
			const pContainerNew* data = dynamic_cast<const pContainerNew*>(packet);
			return reinterpret_cast<const unsigned char*>(data->_data);
		}

		case ID_ACTOR_NEW:
		{
			const pActorNew* data = dynamic_cast<const pActorNew*>(packet);
			return reinterpret_cast<const unsigned char*>(data->_data);
		}

		default:
			throw VaultException("Unhandled packet type %d", (int) packet->type.type);
	}
}

pPacket PacketFactory::ExtractPartial(const pDefault* packet)
{
	pDefault* _packet;

	switch (packet->type.type)
	{
		case ID_ITEM_NEW:
		{
			const pItemNew* data = dynamic_cast<const pItemNew*>(packet);
			_packet = new pObjectNew(data->id, data->refID, data->baseID, data->_data._data_pObjectNew);
			break;
		}

		case ID_CONTAINER_NEW:
		{
			const pContainerNew* data = dynamic_cast<const pContainerNew*>(packet);
			_packet = new pObjectNew(data->id, data->refID, data->baseID, *reinterpret_cast<_pObjectNew*>(data->_data));
			break;
		}

		case ID_ACTOR_NEW:
		{
			const pActorNew* data = dynamic_cast<const pActorNew*>(packet);
			_packet = new pContainerNew(data->id, data->refID, data->baseID, data->_data);
			break;
		}

		case ID_PLAYER_NEW:
		{
			const pPlayerNew* data = dynamic_cast<const pPlayerNew*>(packet);
			_packet = new pActorNew(data->id, data->refID, data->baseID, data->_data);
			break;
		}

		default:
			throw VaultException("Unhandled packet type %d", (int) packet->type.type);
	}

	return pPacket(_packet, FreePacket);
}

void PacketFactory::FreePacket(pDefault* packet)
{
	delete packet;
}
