#ifndef NETWORK_H
#define NETWORK_H

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/NetworkIDManager.h"

#include "vaultmp.h"
#include "PacketTypes.h"
#include "VaultException.h"
#include "CriticalSection.h"

#include <vector>
#include <deque>

typedef pair<pair<pDefault*, vector<unsigned char> >, vector<RakNetGUID> > SingleResponse;
typedef vector<SingleResponse> NetworkResponse;
typedef deque<NetworkResponse> NetworkQueue;

using namespace RakNet;

class Network
{
private:

    Network();

    static NetworkIDManager manager;
    static NetworkQueue queue;
    static CriticalSection cs;

public:

    static SingleResponse CreateResponse(pDefault* packet, unsigned char priority, unsigned char reliability, unsigned char channel, vector<RakNetGUID> targets);
    static SingleResponse CreateResponse(pDefault* packet, unsigned char priority, unsigned char reliability, unsigned char channel, RakNetGUID target);
    static NetworkResponse CompleteResponse(SingleResponse response);

    static void Dispatch(RakPeerInterface* peer, NetworkResponse& response);
    static NetworkIDManager* Manager();
    static NetworkResponse Next();
    static void Queue(NetworkResponse response);
    static void Flush();

};

#endif
