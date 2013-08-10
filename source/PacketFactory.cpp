#include "PacketFactory.h"

#ifdef VAULTMP_DEBUG
DebugInput<PacketFactory> PacketFactory::debug;
#endif

using namespace std;

pPacket PacketFactory::Init(const unsigned char* stream, unsigned int len)
{
	pDefault* packet;

	switch (static_cast<pTypes>(stream[0]))
	{
		case pTypes::ID_GAME_AUTH:
			packet = new typename pTypesMap<pTypes::ID_GAME_AUTH>::type(stream, len);
			break;

		case pTypes::ID_GAME_LOAD:
			packet = new typename pTypesMap<pTypes::ID_GAME_LOAD>::type(stream, len);
			break;

		case pTypes::ID_GAME_MOD:
			packet = new typename pTypesMap<pTypes::ID_GAME_MOD>::type(stream, len);
			break;

		case pTypes::ID_GAME_START:
			packet = new typename pTypesMap<pTypes::ID_GAME_START>::type(stream, len);
			break;

		case pTypes::ID_GAME_END:
			packet = new typename pTypesMap<pTypes::ID_GAME_END>::type(stream, len);
			break;

		case pTypes::ID_GAME_MESSAGE:
			packet = new typename pTypesMap<pTypes::ID_GAME_MESSAGE>::type(stream, len);
			break;

		case pTypes::ID_GAME_CHAT:
			packet = new typename pTypesMap<pTypes::ID_GAME_CHAT>::type(stream, len);
			break;

		case pTypes::ID_GAME_GLOBAL:
			packet = new typename pTypesMap<pTypes::ID_GAME_GLOBAL>::type(stream, len);
			break;

		case pTypes::ID_GAME_WEATHER:
			packet = new typename pTypesMap<pTypes::ID_GAME_WEATHER>::type(stream, len);
			break;

		case pTypes::ID_GAME_BASE:
			packet = new typename pTypesMap<pTypes::ID_GAME_BASE>::type(stream, len);
			break;

		case pTypes::ID_GAME_DELETED:
			packet = new typename pTypesMap<pTypes::ID_GAME_DELETED>::type(stream, len);
			break;

		case pTypes::ID_OBJECT_NEW:
			packet = new typename pTypesMap<pTypes::ID_OBJECT_NEW>::type(stream, len);
			break;

		case pTypes::ID_ITEM_NEW:
			packet = new typename pTypesMap<pTypes::ID_ITEM_NEW>::type(stream, len);
			break;

		case pTypes::ID_CONTAINER_NEW:
			packet = new typename pTypesMap<pTypes::ID_CONTAINER_NEW>::type(stream, len);
			break;

		case pTypes::ID_ACTOR_NEW:
			packet = new typename pTypesMap<pTypes::ID_ACTOR_NEW>::type(stream, len);
			break;

		case pTypes::ID_PLAYER_NEW:
			packet = new typename pTypesMap<pTypes::ID_PLAYER_NEW>::type(stream, len);
			break;

		case pTypes::ID_OBJECT_REMOVE:
			packet = new typename pTypesMap<pTypes::ID_OBJECT_REMOVE>::type(stream, len);
			break;

		case pTypes::ID_WINDOW_NEW:
			packet = new typename pTypesMap<pTypes::ID_WINDOW_NEW>::type(stream, len);
			break;

		case pTypes::ID_BUTTON_NEW:
			packet = new typename pTypesMap<pTypes::ID_BUTTON_NEW>::type(stream, len);
			break;

		case pTypes::ID_TEXT_NEW:
			packet = new typename pTypesMap<pTypes::ID_TEXT_NEW>::type(stream, len);
			break;

		case pTypes::ID_EDIT_NEW:
			packet = new typename pTypesMap<pTypes::ID_EDIT_NEW>::type(stream, len);
			break;

		case pTypes::ID_WINDOW_REMOVE:
			packet = new typename pTypesMap<pTypes::ID_WINDOW_REMOVE>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_NAME:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_NAME>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_POS:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_POS>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_ANGLE:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_ANGLE>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_CELL:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_CELL>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_LOCK:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_LOCK>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_OWNER:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_OWNER>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_ACTIVATE:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_ACTIVATE>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_COUNT:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_COUNT>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_CONDITION:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_CONDITION>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_EQUIPPED:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_EQUIPPED>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_VALUE:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_VALUE>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_STATE:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_STATE>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_RACE:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_RACE>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_SEX:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_SEX>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_DEAD:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_DEAD>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_FIREWEAPON:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_FIREWEAPON>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_IDLE:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_IDLE>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_CONTROL:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_CONTROL>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_INTERIOR:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_INTERIOR>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_EXTERIOR:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_EXTERIOR>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_CONTEXT:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_CONTEXT>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_CONSOLE:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_CONSOLE>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_WPOS:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_WPOS>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_WSIZE:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_WSIZE>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_WVISIBLE:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_WVISIBLE>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_WLOCKED:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_WLOCKED>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_WTEXT:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_WTEXT>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_WMAXLEN:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_WMAXLEN>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_WVALID:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_WVALID>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_WCLICK:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_WCLICK>::type(stream, len);
			break;

		case pTypes::ID_UPDATE_WMODE:
			packet = new typename pTypesMap<pTypes::ID_UPDATE_WMODE>::type(stream, len);
			break;

		default:
			throw VaultException("Unhandled packet type %d", stream[0]).stacktrace();
	}

#ifdef VAULTMP_DEBUG
	debug.print("Constructing packet of type ", typeid(*packet).name(), ", length ", dec, packet->length(), ", type ", static_cast<unsigned int>(packet->type()));
#endif

	return pPacket(packet);
}
