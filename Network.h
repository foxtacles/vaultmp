#ifndef NETWORK_H
#define NETWORK_H

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakSleep.h"

#include "vaultmp.h"
#include "PacketTypes.h"
#include "VaultException.h"

#include <vector>

typedef vector<pair<pDefault*, vector<unsigned char> > > NetworkResponse;

enum
{
    CHANNEL_SYSTEM,
    CHANNEL_GAME,
    CHANNEL_PLAYER_DATA,
};

using namespace RakNet;

class Network
{
private:

    Network();

    static pair<pDefault*, vector<unsigned char> > CreateResponse(pDefault* packet, unsigned char priority, unsigned char reliability, unsigned char channel);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

public:

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    static NetworkResponse ProcessEvent(unsigned char id, RakNetGUID guid);
    static NetworkResponse ProcessPacket(Packet* data, RakNetGUID guid);

    static void Dispatch(RakPeerInterface* peer, NetworkResponse& response, const SystemAddress& target, bool broadcast = false);

};

#endif
