#include "Network.h"
#include "PacketTypes.h"

NetworkIDManager Network::manager;
NetworkQueue Network::queue;
CriticalSection Network::cs;
bool Network::dequeue = true;

SingleResponse Network::CreateResponse(pDefault* packet, PacketPriority priority, PacketReliability reliability, unsigned char channel, vector<RakNetGUID> targets)
{
	return SingleResponse(pair<pDefault*, PacketDescriptor>(packet, PacketDescriptor(priority, reliability, channel)), targets);
}

SingleResponse Network::CreateResponse(pDefault* packet, PacketPriority priority, PacketReliability reliability, unsigned char channel, RakNetGUID target)
{
	return CreateResponse(packet, priority, reliability, channel, vector<RakNetGUID>{target});
}

NetworkResponse Network::CompleteResponse(SingleResponse response)
{
	return NetworkResponse{response};
}

void Network::Dispatch(RakPeerInterface* peer, NetworkResponse& response)
{
	if (peer == NULL)
		throw VaultException("RakPeerInterface is NULL");

	for (SingleResponse& s : response)
	{
		for (RakNetGUID& guid : s.second)
			peer->Send(reinterpret_cast<const char*>(s.first.first->get()), s.first.first->length(), get<0>(s.first.second), get<1>(s.first.second), get<2>(s.first.second), guid, false);

		PacketFactory::FreePacket(s.first.first);
	}
}

NetworkIDManager* Network::Manager()
{
	return &manager;
}

NetworkResponse Network::Next()
{
	NetworkResponse response;

	if (!queue.empty() && dequeue)
	{
		cs.StartSession();

		response = queue.back();
		queue.pop_back();

		cs.EndSession();
	}

	return response;
}

void Network::Queue(NetworkResponse response)
{
	cs.StartSession();

	queue.push_front(response);

	cs.EndSession();
}

void Network::ToggleDequeue(bool toggle)
{
	Network::dequeue = toggle;
}

void Network::Flush()
{
	cs.StartSession();

	for (NetworkResponse& n : queue)
	{
		for (SingleResponse& s : n)
			PacketFactory::FreePacket(s.first.first);
	}

	queue.clear();

	cs.EndSession();
}
