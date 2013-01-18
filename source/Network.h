#ifndef NETWORK_H
#define NETWORK_H

#include "RakNet.h"

#include "vaultmp.h"
#include "VaultException.h"
#include "CriticalSection.h"
#include "PacketFactory.h"

#include <vector>
#include <tuple>
#include <deque>

/**
 * \brief The Network class provides basic facilities to create, send and queue packets
 *
 * NetworkClient and NetworkServer derive from this class
 */

class Network
{
	private:
		typedef std::tuple<PacketPriority, PacketReliability, unsigned char> PacketDescriptor;

	public:
		class SingleResponse {

			friend class Network;

			private:
				pPacket packet;
				PacketDescriptor descriptor;
				std::vector<RakNet::RakNetGUID> targets;

				SingleResponse(pPacket&& packet, PacketDescriptor descriptor, const std::vector<RakNet::RakNetGUID>& targets) : packet(std::move(packet)), descriptor(descriptor), targets(targets) {}
				SingleResponse(pPacket&& packet, PacketDescriptor descriptor, RakNet::RakNetGUID target) : packet(std::move(packet)), descriptor(descriptor), targets(std::vector<RakNet::RakNetGUID>{target}) {}

			public:
				~SingleResponse() = default;

				SingleResponse(SingleResponse&&) = default;
				SingleResponse& operator= (SingleResponse&&) = default;

				// hack: initializer lists reference static memory... this ctor enables moving the packet
				// NOT A COPY CTOR
				SingleResponse(const SingleResponse& response) : SingleResponse(std::move(const_cast<SingleResponse&>(response))) {}

				const std::vector<RakNet::RakNetGUID>& get_targets() const { return targets; }
				const pDefault* get_packet() const { return packet.get(); }
		};

		typedef std::vector<SingleResponse> NetworkResponse;

	private:
		Network() = delete;

#ifdef VAULTMP_DEBUG
		static DebugInput<Network> debug;
#endif

		typedef std::deque<NetworkResponse> NetworkQueue;

		static RakNet::NetworkIDManager manager;
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
		static SingleResponse CreateResponse(pPacket&& packet, PacketPriority priority, PacketReliability reliability, unsigned char channel, const std::vector<RakNet::RakNetGUID>& targets);
		/**
		 * \brief Creates a SingleResponse given a single network target
		 *
		 * packet is a pPacket retrieved via the PacketFactory
		 * priority specifies the RakNet priority (see PacketPriority.h)
		 * reliability specifies the RakNet reliability (see PacketPriority.h)
		 * channel sepcifies the RakNet channel to send this packet on
		 * target is a RakNetGUID
		 */
		static SingleResponse CreateResponse(pPacket&& packet, PacketPriority priority, PacketReliability reliability, unsigned char channel, RakNet::RakNetGUID target);
		/**
		 * \brief Sends a NetworkResponse over RakPeerInterface peer
		 *
		 * This function effectively deallocates the packets inside the NetworkResponse
		 */
		static void Dispatch(RakNet::RakPeerInterface* peer, NetworkResponse&& response);
		/**
		 * \brief Sends the next NetworkResponse in the queue over RakPeerInterface peer
		 *
		 * This function effectively deallocates the packet
		 */
		static bool Dispatch(RakNet::RakPeerInterface* peer);
		/**
		 * \brief Returns a pointer to the static NetworkIDManager
		 *
		 * Although not a const pointer, you must not delete the object
		 */
		static RakNet::NetworkIDManager* Manager() { return &manager; }
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
};

using NetworkResponse = Network::NetworkResponse;
using SingleResponse = Network::SingleResponse;

#endif
