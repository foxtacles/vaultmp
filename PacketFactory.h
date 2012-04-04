#ifndef PACKETFACTORY_H
#define PACKETFACTORY_H

#include "vaultmp.h"
#include "VaultException.h"
#include "Data.h"

using namespace Data;

enum
{
	ID_EVENT_INTERFACE_LOST,
	ID_EVENT_CLIENT_ERROR,
	ID_EVENT_SERVER_ERROR,
	ID_EVENT_GAME_STARTED,
	ID_EVENT_GAME_LOADED,
	ID_EVENT_AUTH_RECEIVED,
	ID_EVENT_CLOSE_RECEIVED,
};

enum
{
	ID_GAME_AUTH = ID_GAME_FIRST,
	ID_GAME_LOAD,
	ID_GAME_MOD,
	ID_GAME_START,
	ID_GAME_END,

    ID_OBJECT_NEW,
    ID_ITEM_NEW,
    ID_CONTAINER_NEW,
    ID_ACTOR_NEW,
	ID_PLAYER_NEW,

	ID_OBJECT_REMOVE,

	ID_OBJECT_UPDATE,
	ID_ACTOR_UPDATE,
	ID_PLAYER_UPDATE,
};

enum
{
	ID_UPDATE_POS,
	ID_UPDATE_ANGLE,
	ID_UPDATE_CELL,
	ID_UPDATE_VALUE,
	ID_UPDATE_STATE,
	ID_UPDATE_CONTROL,
};

enum
{
	ID_REASON_KICK = 0,
	ID_REASON_BAN,
	ID_REASON_ERROR,
	ID_REASON_DENIED,
	ID_REASON_NONE,
};

class pDefault;

class PacketFactory
{
	private:
		PacketFactory();

	public:
		static pDefault* CreatePacket( unsigned char type, ... );

		static pDefault* CreatePacket( unsigned char* stream, unsigned int len );

		static void Access( const pDefault* packet, ... );

		static NetworkID ExtractNetworkID( const pDefault* packet);

		static unsigned int ExtractReference( const pDefault* packet);

		static unsigned int ExtractBase( const pDefault* packet);

		static const unsigned char* ExtractRawData(const pDefault* packet);

		static pDefault* ExtractPartial(const pDefault* packet);

		static void FreePacket(pDefault* packet);
};

#include "PacketTypes.h"

#endif
