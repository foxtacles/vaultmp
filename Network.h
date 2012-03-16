#ifndef NETWORK_H
#define NETWORK_H

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/NetworkIDManager.h"

#include "vaultmp.h"
#include "PacketFactory.h"
#include "VaultException.h"
#include "CriticalSection.h"

#include <vector>
#include <deque>

typedef pair<pair<pDefault*, vector<unsigned char> >, vector<RakNetGUID> > SingleResponse;
typedef vector<SingleResponse> NetworkResponse;
typedef deque<NetworkResponse> NetworkQueue;

using namespace RakNet;

/**
 * \brief The Network class provides basic facilities to create, send and queue packets
 *
 * NetworkClient and NetworkServer derive from this class
 */

class Network
{
	private:

		Network();

		static NetworkIDManager manager;
		static NetworkQueue queue;
		static CriticalSection cs;

	public:

		/**
		 * \brief Creates a SingleResponse given multiple network targets
		 *
		 * packet is a pDefault pointer retrieved via the PacketFactory
		 * priority specifies the RakNet priority (see PacketPriority.h)
		 * reliability specifies the RakNet reliability (see PacketPriority.h)
		 * channel sepcifies the RakNet channel to send this packet on
		 * targets is a STL vector containing RakNetGUID's
		 */
		static SingleResponse CreateResponse( pDefault* packet, unsigned char priority, unsigned char reliability, unsigned char channel, vector<RakNetGUID> targets );
		/**
		 * \brief Creates a SingleResponse given a single network target
		 *
		 * packet is a pDefault pointer retrieved via the PacketFactory
		 * priority specifies the RakNet priority (see PacketPriority.h)
		 * reliability specifies the RakNet reliability (see PacketPriority.h)
		 * channel sepcifies the RakNet channel to send this packet on
		 * target is a RakNetGUID
		 */
		static SingleResponse CreateResponse( pDefault* packet, unsigned char priority, unsigned char reliability, unsigned char channel, RakNetGUID target );
		/**
		 * \brief Creates a NetworkResponse from a SingleResponse
		 *
		 * The returned NetworkResponse can be used with Dispatch
		 */
		static NetworkResponse CompleteResponse( SingleResponse response );

		/**
		 * \brief Sends a NetworkResponse over RakPeerInterface peer
		 *
		 * This function effectively deallocates the packets inside the NetworkResponse
		 */
		static void Dispatch( RakPeerInterface* peer, NetworkResponse& response );
		/**
		 * \brief Returns a pointer to the static NetworkIDManager
		 *
		 * Although not a const pointer, you must not delete the object
		 */
		static NetworkIDManager* Manager();
		/**
		 * \brief Returns the next NetworkResponse in the queue
		 */
		static NetworkResponse Next();
		/**
		 * \brief Queues a NetworkResponse
		 */
		static void Queue( NetworkResponse response );
		/**
		 * \brief Flushes the queue
		 */
		static void Flush();

};

#endif
