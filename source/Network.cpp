#include "Network.hpp"

using namespace std;
using namespace RakNet;

NetworkIDManager Network::manager;
Network::NetworkQueue Network::queue;
CriticalSection Network::cs;
bool Network::dequeue = true;

#ifdef VAULTMP_DEBUG
DebugInput<Network> Network::debug;
#endif

void Network::Dispatch(RakPeerInterface* peer, NetworkResponse&& response)
{
	for (SingleResponse& s : move(response))
	{
#ifdef VAULTMP_DEBUG
		debug.print("Sending packet of type ", typeid(s.packet).name(), ", length ", dec, s.packet.length(), ", type ", static_cast<unsigned int>(s.packet.type()));
#endif

		for (RakNetGUID& guid : s.targets)
			peer->Send(reinterpret_cast<const char*>(s.packet.get()), s.packet.length(), get<0>(s.descriptor), get<1>(s.descriptor), get<2>(s.descriptor), guid, false);
	}
}

bool Network::Dispatch(RakPeerInterface* peer)
{
	if (queue.empty() || !dequeue)
		return false;

	cs.StartSession();

	const NetworkResponse& response = queue.back();

	for (const SingleResponse& s : response)
	{
#ifdef VAULTMP_DEBUG
		debug.print("Sending packet of type ", typeid(s.packet).name(), ", length ", dec, s.packet.length(), ", type ", static_cast<unsigned int>(s.packet.type()));
#endif

		for (const RakNetGUID& guid : s.targets)
			peer->Send(reinterpret_cast<const char*>(s.packet.get()), s.packet.length(), get<0>(s.descriptor), get<1>(s.descriptor), get<2>(s.descriptor), guid, false);
	}

	queue.pop_back();

	cs.EndSession();

	return true;
}

void Network::Queue(NetworkResponse&& response)
{
	cs.StartSession();

	queue.emplace_front(move(response));

	cs.EndSession();
}

void Network::Flush()
{
	cs.StartSession();

	queue.clear();

	cs.EndSession();
}
