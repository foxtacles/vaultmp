#ifndef DATA_H
#define DATA_H

#include <string>
#include <list>
#include <future>
#include <vector>
#include <map>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakString.h"
#include "RakNet/MessageIdentifiers.h"

#include "Utils.h"
#include "VaultException.h"
#include "VaultFunctor.h"

static const unsigned int PLAYER_REFERENCE  = 0x00000014;
static const unsigned int PLAYER_BASE       = 0x00000007;

using namespace std;
using namespace RakNet;

/* Shared data structures and tables */

namespace Data
{
	template <typename R, typename T>
	inline static R storeIn(T t)
	{
		R r;
		*reinterpret_cast<T*>(&r) = t;
		return r;
	}
	template <typename R, typename T>
	inline static T getFrom(R r)
	{
		return *reinterpret_cast<T*>(&r);
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
}

#endif
