#ifndef DATA_H
#define DATA_H

#include "RakNet.hpp"

static const unsigned int PLAYER_REFERENCE  = 0x00000014;
static const unsigned int PLAYER_BASE       = 0x00000007;
static const unsigned int RACE_CAUCASIAN    = 0x00000019;
static const unsigned int DEFAULT_WEATHER   = 0x0000015E;

template <typename R, typename T>
inline static R storeIn(const T& t)
{
	R r;
	*reinterpret_cast<T*>(&r) = t;
	return r;
}

template <typename T, typename R>
inline static T getFrom(const R& r)
{
	return *reinterpret_cast<const T*>(&r);
}

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

enum class Reason : unsigned char
{
	ID_REASON_KICK,
	ID_REASON_BAN,
	ID_REASON_ERROR,
	ID_REASON_DENIED,
	ID_REASON_QUIT,
	ID_REASON_NONE,
};

enum
{
	ID_MASTER_QUERY = ID_USER_PACKET_ENUM,
	ID_MASTER_ANNOUNCE,
	ID_MASTER_UPDATE,
	ID_GAME_FIRST,
};

enum
{
	CHANNEL_SYSTEM,
	CHANNEL_GAME,
	CHANNEL_CHAT,
};

#endif
