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

typedef pair<pair<pDefault*, vector<unsigned char> >, vector<RakNetGUID> > SingleResponse;
typedef vector<SingleResponse> NetworkResponse;

using namespace RakNet;

class Network
{
private:

    Network();

public:

    static SingleResponse CreateResponse(pDefault* packet, unsigned char priority, unsigned char reliability, unsigned char channel, vector<RakNetGUID> targets);
    static SingleResponse CreateResponse(pDefault* packet, unsigned char priority, unsigned char reliability, unsigned char channel, RakNetGUID target);
    static NetworkResponse CompleteResponse(SingleResponse response);

    static void Dispatch(RakPeerInterface* peer, NetworkResponse& response);

};

#endif
