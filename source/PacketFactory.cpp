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

		case pTypes::ID_GAME_GLOBAL:
			packet = new pGameGlobal(stream, len);
			break;

		case pTypes::ID_GAME_WEATHER:
			packet = new pGameWeather(stream, len);
			break;

		case pTypes::ID_GAME_BASE:
			packet = new pGameBase(stream, len);
			break;

		case pTypes::ID_GAME_DELETED:
			packet = new pGameDeleted(stream, len);
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

		case pTypes::ID_WINDOW_NEW:
			packet = new pWindowNew(stream, len);
			break;

		case pTypes::ID_BUTTON_NEW:
			packet = new pButtonNew(stream, len);
			break;

		case pTypes::ID_TEXT_NEW:
			packet = new pTextNew(stream, len);
			break;

		case pTypes::ID_EDIT_NEW:
			packet = new pEditNew(stream, len);
			break;

		case pTypes::ID_WINDOW_REMOVE:
			packet = new pWindowRemove(stream, len);
			break;

		case pTypes::ID_UPDATE_NAME:
			packet = new pObjectName(stream, len);
			break;

		case pTypes::ID_UPDATE_POS:
			packet = new pObjectPos(stream, len);
			break;

		case pTypes::ID_UPDATE_ANGLE:
			packet = new pObjectAngle(stream, len);
			break;

		case pTypes::ID_UPDATE_CELL:
			packet = new pObjectCell(stream, len);
			break;

		case pTypes::ID_UPDATE_LOCK:
			packet = new pObjectLock(stream, len);
			break;

		case pTypes::ID_UPDATE_OWNER:
			packet = new pObjectOwner(stream, len);
			break;

		case pTypes::ID_UPDATE_COUNT:
			packet = new pItemCount(stream, len);
			break;

		case pTypes::ID_UPDATE_CONDITION:
			packet = new pItemCondition(stream, len);
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

		case pTypes::ID_UPDATE_RACE:
			packet = new pActorRace(stream, len);
			break;

		case pTypes::ID_UPDATE_SEX:
			packet = new pActorSex(stream, len);
			break;

		case pTypes::ID_UPDATE_DEAD:
			packet = new pActorDead(stream, len);
			break;

		case pTypes::ID_UPDATE_FIREWEAPON:
			packet = new pActorFireweapon(stream, len);
			break;

		case pTypes::ID_UPDATE_IDLE:
			packet = new pActorIdle(stream, len);
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

		case pTypes::ID_UPDATE_CONTEXT:
			packet = new pPlayerContext(stream, len);
			break;

		case pTypes::ID_UPDATE_CONSOLE:
			packet = new pPlayerConsole(stream, len);
			break;

		case pTypes::ID_UPDATE_WINDOW:
			packet = new pGuiWindow(stream, len);
			break;

		case pTypes::ID_UPDATE_TEXT:
			packet = new pGuiText(stream, len);
			break;

		default:
			throw VaultException("Unhandled packet type %d", stream[0]).stacktrace();
	}

#ifdef VAULTMP_DEBUG
	debug.print("Constructing packet of type ", typeid(*packet).name(), ", length ", dec, packet->length(), ", type ", static_cast<unsigned int>(*packet->get()));
#endif

	return pPacket(packet);
}
