#ifndef NETWORK_H
#define NETWORK_H

#include "vaultmp.hpp"
#include "packet/PacketFactory.hpp"
#include "CriticalSection.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

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

			public:
				SingleResponse(pPacket&& packet, PacketPriority priority, PacketReliability reliability, unsigned char channel, const std::vector<RakNet::RakNetGUID>& targets) : packet(std::move(packet)), descriptor(priority, reliability, channel), targets(targets) {}
				SingleResponse(pPacket&& packet, PacketPriority priority, PacketReliability reliability, unsigned char channel, RakNet::RakNetGUID target) : packet(std::move(packet)), descriptor(priority, reliability, channel), targets{target} {}
				~SingleResponse() = default;

				SingleResponse(SingleResponse&&) = default;
				SingleResponse& operator= (SingleResponse&&) = default;

				// hack: initializer lists reference static memory... this ctor enables moving the packet
				// NOT A COPY CTOR
				SingleResponse(const SingleResponse& response) : SingleResponse(std::move(const_cast<SingleResponse&>(response))) {}

				const std::vector<RakNet::RakNetGUID>& get_targets() const { return targets; }
				const pPacket* get_packet() const { return &packet; }
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
