#ifndef NETWORK_H
#define NETWORK_H

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/NetworkIDManager.h"

#include "vaultmp.h"
#include "VaultException.h"
#include "CriticalSection.h"
#include "PacketFactory.h"

#include <vector>
#include <tuple>
#include <deque>

using namespace RakNet;

/**
 * \brief The Network class provides basic facilities to create, send and queue packets
 *
 * NetworkClient and NetworkServer derive from this class
 */

class Network
{
	typedef tuple<PacketPriority, PacketReliability, unsigned char> PacketDescriptor;

	public:
		class SingleResponse {

			friend class Network;

			private:
				pPacket packet;
				PacketDescriptor descriptor;
				vector<RakNetGUID> targets;

				SingleResponse(pPacket&& packet, PacketDescriptor descriptor, const vector<RakNetGUID>& targets) : packet(move(packet)), descriptor(descriptor), targets(targets) {}
				SingleResponse(pPacket&& packet, PacketDescriptor descriptor, RakNetGUID target) : packet(move(packet)), descriptor(descriptor), targets(vector<RakNetGUID>{target}) {}

			public:
				~SingleResponse() = default;

				SingleResponse(SingleResponse&&) = default;
				SingleResponse& operator= (SingleResponse&&) = default;

				// hack: initializer lists reference static memory... this ctor enables moving the packet
				// NOT A COPY CTOR
				SingleResponse(const SingleResponse& response) : SingleResponse(move(const_cast<SingleResponse&>(response))) {}

				const vector<RakNetGUID>& get_targets() const { return targets; }
				const pDefault* get_packet() const { return packet.get(); }
		};

		typedef vector<SingleResponse> NetworkResponse;

	private:
		Network() = delete;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		typedef deque<NetworkResponse> NetworkQueue;

		static NetworkIDManager manager;
		static NetworkQueue queue;
		static CriticalSection cs;
		static bool dequeue;

	public:

		/**
		 * \brief Creates a SingleResponse given multiple network targets
		 *
		 * packet is a pPacket retrieved via the PacketFactory
		 * priority specifies the RakNet priority (see PacketPriority.h)
		 * reliability specifies the RakNet reliability (see PacketPriority.h)
		 * channel sepcifies the RakNet channel to send this packet on
		 * targets is a STL vector containing RakNetGUID's
		 */
		static SingleResponse CreateResponse(pPacket&& packet, PacketPriority priority, PacketReliability reliability, unsigned char channel, const vector<RakNetGUID>& targets);
		/**
		 * \brief Creates a SingleResponse given a single network target
		 *
		 * packet is a pPacket retrieved via the PacketFactory
		 * priority specifies the RakNet priority (see PacketPriority.h)
		 * reliability specifies the RakNet reliability (see PacketPriority.h)
		 * channel sepcifies the RakNet channel to send this packet on
		 * target is a RakNetGUID
		 */
		static SingleResponse CreateResponse(pPacket&& packet, PacketPriority priority, PacketReliability reliability, unsigned char channel, RakNetGUID target);
		/**
		 * \brief Sends a NetworkResponse over RakPeerInterface peer
		 *
		 * This function effectively deallocates the packets inside the NetworkResponse
		 */
		static void Dispatch(RakPeerInterface* peer, NetworkResponse&& response);
		/**
		 * \brief Sends the next NetworkResponse in the queue over RakPeerInterface peer
		 *
		 * This function effectively deallocates the packet
		 */
		static bool Dispatch(RakPeerInterface* peer);
		/**
		 * \brief Returns a pointer to the static NetworkIDManager
		 *
		 * Although not a const pointer, you must not delete the object
		 */
		static NetworkIDManager* Manager() { return &manager; }
		/**
		 * \brief Queues a NetworkResponse
		 */
		static void Queue(NetworkResponse&& response);
		/**
		 * \brief Toggles dequeueing
		 */
		static void ToggleDequeue(bool toggle) { Network::dequeue = toggle; }
		/**
		 * \brief Flushes the queue
		 */
		static void Flush();

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif
};

using NetworkResponse = Network::NetworkResponse;
using SingleResponse = Network::SingleResponse;

#endif
