#include "Network.h"
#include "PacketTypes.h"

NetworkIDManager Network::manager;
Network::NetworkQueue Network::queue;
CriticalSection Network::cs;
bool Network::dequeue = true;

#ifdef VAULTMP_DEBUG
Debug* Network::debug = NULL;
#endif

#ifdef VAULTMP_DEBUG
void Network::SetDebugHandler(Debug* debug)
{
	Network::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Network class", true);
}
#endif

Network::SingleResponse Network::CreateResponse(pPacket&& packet, PacketPriority priority, PacketReliability reliability, unsigned char channel, const vector<RakNetGUID>& targets)
{
	return SingleResponse(move(packet), PacketDescriptor(priority, reliability, channel), targets);
}

Network::SingleResponse Network::CreateResponse(pPacket&& packet, PacketPriority priority, PacketReliability reliability, unsigned char channel, RakNetGUID target)
{
	return SingleResponse(move(packet), PacketDescriptor(priority, reliability, channel), vector<RakNetGUID>{target});
}

void Network::Dispatch(RakPeerInterface* peer, NetworkResponse&& response)
{
	if (peer == NULL)
		throw VaultException("RakPeerInterface is NULL");

	for (SingleResponse& s : response)
	{
#ifdef VAULTMP_DEBUG
		if (debug)
			debug->PrintFormat("Sending packet of type %s, length %d, type %d", true, typeid(*s.packet).name(), s.packet->length(), *s.packet->get());
#endif

		for (RakNetGUID& guid : s.targets)
			peer->Send(reinterpret_cast<const char*>(s.packet->get()), s.packet->length(), get<0>(s.descriptor), get<1>(s.descriptor), get<2>(s.descriptor), guid, false);
	}
}

bool Network::Dispatch(RakPeerInterface* peer)
{
	if (queue.empty() || !dequeue)
		return false;

	if (peer == NULL)
		throw VaultException("RakPeerInterface is NULL");

	cs.StartSession();

	const NetworkResponse& response = queue.back();

	for (const SingleResponse& s : response)
	{
#ifdef VAULTMP_DEBUG
		if (debug)
			debug->PrintFormat("Sending packet of type %s, length %d, type %d", true, typeid(*s.packet).name(), s.packet->length(), *s.packet->get());
#endif

		for (const RakNetGUID& guid : s.targets)
			peer->Send(reinterpret_cast<const char*>(s.packet->get()), s.packet->length(), get<0>(s.descriptor), get<1>(s.descriptor), get<2>(s.descriptor), guid, false);
	}

	queue.pop_back();

	cs.EndSession();

	return true;
}

void Network::Queue(NetworkResponse&& response)
{
	cs.StartSession();

	queue.push_front(move(response));

	cs.EndSession();
}

void Network::Flush()
{
	cs.StartSession();

	queue.clear();

	cs.EndSession();
}
