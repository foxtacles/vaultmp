#include "Network.h"
#include "PacketTypes.h"

NetworkIDManager Network::manager;
NetworkQueue Network::queue;
CriticalSection Network::cs;
bool Network::dequeue = true;

SingleResponse Network::CreateResponse(pDefault* packet, unsigned char priority, unsigned char reliability, unsigned char channel, vector<RakNetGUID> targets)
{
	vector<unsigned char> data = vector<unsigned char>();
	data.push_back(priority);
	data.push_back(reliability);
	data.push_back(channel);
	SingleResponse response = SingleResponse(pair<pDefault*, vector<unsigned char> >(packet, data), targets);
	return response;
}

SingleResponse Network::CreateResponse(pDefault* packet, unsigned char priority, unsigned char reliability, unsigned char channel, RakNetGUID target)
{
	vector<RakNetGUID> targets = vector<RakNetGUID>();
	targets.push_back(target);
	return CreateResponse(packet, priority, reliability, channel, targets);
}

NetworkResponse Network::CompleteResponse(SingleResponse response)
{
	NetworkResponse complete;
	complete.push_back(response);
	return complete;
}

void Network::Dispatch(RakPeerInterface* peer, NetworkResponse& response)
{
	if (peer == NULL)
		throw VaultException("RakPeerInterface is NULL");

	NetworkResponse::iterator it;
	vector<RakNetGUID>::iterator it2;

	for (it = response.begin(); it != response.end(); ++it)
	{
		for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			peer->Send((char*) it->first.first->get(), it->first.first->length(), (PacketPriority) it->first.second.at(0), (PacketReliability) it->first.second.at(1), it->first.second.at(2), *it2, false);

		PacketFactory::FreePacket(it->first.first);
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

	deque<NetworkResponse>::iterator it;
	NetworkResponse::iterator it2;

	for (it = queue.begin(); it != queue.end(); ++it)
	{
		for (it2 = (*it).begin(); it2 != (*it).end(); ++it2)
			PacketFactory::FreePacket(it2->first.first);
	}

	queue.clear();

	cs.EndSession();
}
