#ifndef DATA_H
#define DATA_H

#include <tuple>

#include "RakNet.h"
#include "Utils.h"
#include "VaultException.h"
#include "VaultFunctor.h"

static const unsigned int PLAYER_REFERENCE  = 0x00000014;
static const unsigned int PLAYER_BASE       = 0x00000007;
static const unsigned int RACE_CAUCASIAN	= 0x00000019;
static const unsigned int DEFAULT_WEATHER	= 0x0000015E;

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
