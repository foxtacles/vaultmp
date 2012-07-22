#include "PacketFactory.h"

pPacket PacketFactory::Init(const unsigned char* stream, unsigned int len)
{
	pDefault* packet;

	switch (static_cast<pTypes>(stream[0]))
	{
		case pTypes::ID_GAME_AUTH:
			packet = new pGameAuth(stream, len);
			break;

		case pTypes::ID_GAME_LOAD:
			packet = new pGameLoad(stream, len);
			break;

		case pTypes::ID_GAME_MOD:
			packet = new pGameMod(stream, len);
			break;

		case pTypes::ID_GAME_START:
			packet = new pGameStart(stream, len);
			break;

		case pTypes::ID_GAME_END:
			packet = new pGameEnd(stream, len);
			break;

		case pTypes::ID_GAME_MESSAGE:
			packet = new pGameMessage(stream, len);
			break;

		case pTypes::ID_GAME_CHAT:
			packet = new pGameChat(stream, len);
			break;

		case pTypes::ID_OBJECT_NEW:
			packet = new pObjectNew(stream, len);
			break;

		case pTypes::ID_ITEM_NEW:
			packet = new pItemNew(stream, len);
			break;

		case pTypes::ID_CONTAINER_NEW:
			packet = new pContainerNew(stream, len);
			break;

		case pTypes::ID_ACTOR_NEW:
			packet = new pActorNew(stream, len);
			break;

		case pTypes::ID_PLAYER_NEW:
			packet = new pPlayerNew(stream, len);
			break;

		case pTypes::ID_OBJECT_REMOVE:
			packet = new pObjectRemove(stream, len);
			break;

		case pTypes::ID_OBJECT_UPDATE:
		case pTypes::ID_CONTAINER_UPDATE:
		case pTypes::ID_ACTOR_UPDATE:
		case pTypes::ID_PLAYER_UPDATE:
		{
			if (len < 2)
				throw VaultException("Incomplete object packet type %d", stream[0]);

			switch (static_cast<pTypes>(stream[1]))
			{
				case pTypes::ID_UPDATE_POS:
					packet = new pObjectPos(stream, len);
					break;

				case pTypes::ID_UPDATE_ANGLE:
					packet = new pObjectAngle(stream, len);
					break;

				case pTypes::ID_UPDATE_CELL:
					packet = new pObjectCell(stream, len);
					break;

				case pTypes::ID_UPDATE_CONTAINER:
					packet = new pContainerUpdate(stream, len);
					break;

				case pTypes::ID_UPDATE_VALUE:
					packet = new pActorValue(stream, len);
					break;

				case pTypes::ID_UPDATE_STATE:
					packet = new pActorState(stream, len);
					break;

				case pTypes::ID_UPDATE_DEAD:
					packet = new pActorDead(stream, len);
					break;

				case pTypes::ID_UPDATE_FIREWEAPON:
					packet = new pActorFireweapon(stream, len);
					break;

				case pTypes::ID_UPDATE_CONTROL:
					packet = new pPlayerControl(stream, len);
					break;

				case pTypes::ID_UPDATE_INTERIOR:
					packet = new pPlayerInterior(stream, len);
					break;

				case pTypes::ID_UPDATE_EXTERIOR:
					packet = new pPlayerExterior(stream, len);
					break;

				default:
					throw VaultException("Unhandled object update packet type %d", stream[1]);
			}

			break;
		}

		default:
			throw VaultException("Unhandled packet type %d", stream[0]);
	}

	return pPacket(packet, FreePacket);
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
		case pTypes::ID_OBJECT_NEW:
		{
			const pObjectNew* data = dynamic_cast<const pObjectNew*>(packet);
			return reinterpret_cast<const unsigned char*>(&data->_data);
		}

		case pTypes::ID_CONTAINER_NEW:
		{
			const pContainerNew* data = dynamic_cast<const pContainerNew*>(packet);
			return reinterpret_cast<const unsigned char*>(data->_data);
		}

		case pTypes::ID_ACTOR_NEW:
		{
			const pActorNew* data = dynamic_cast<const pActorNew*>(packet);
			return reinterpret_cast<const unsigned char*>(data->_data);
		}

		default:
			throw VaultException("Unhandled packet type %d", packet->type.type);
	}
}

pPacket PacketFactory::ExtractPartial(const pDefault* packet)
{
	pDefault* _packet;

	switch (packet->type.type)
	{
		case pTypes::ID_ITEM_NEW:
		{
			const pItemNew* data = dynamic_cast<const pItemNew*>(packet);
			_packet = new pObjectNew(data->id, data->refID, data->baseID, data->_data._data_pObjectNew);
			break;
		}

		case pTypes::ID_CONTAINER_NEW:
		{
			const pContainerNew* data = dynamic_cast<const pContainerNew*>(packet);
			_packet = new pObjectNew(data->id, data->refID, data->baseID, *reinterpret_cast<_pObjectNew*>(data->_data));
			break;
		}

		case pTypes::ID_ACTOR_NEW:
		{
			const pActorNew* data = dynamic_cast<const pActorNew*>(packet);
			_packet = new pContainerNew(data->id, data->refID, data->baseID, data->_data);
			break;
		}

		case pTypes::ID_PLAYER_NEW:
		{
			const pPlayerNew* data = dynamic_cast<const pPlayerNew*>(packet);
			_packet = new pActorNew(data->id, data->refID, data->baseID, data->_data);
			break;
		}

		default:
			throw VaultException("Unhandled packet type %d", packet->type.type);
	}

	return pPacket(_packet, FreePacket);
}

void PacketFactory::FreePacket(pDefault* packet)
{
	delete packet;
}
