#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "vaultserver.hpp"
#include "Network.hpp"

/**
 * \brief Server network interface
 */

class NetworkServer : public Network
{
		friend class Dedicated;

	private:
		NetworkServer() = delete;

#ifdef VAULTMP_DEBUG
		static DebugInput<NetworkServer> debug;
#endif

	public:
		/**
		 * \brief Processes an event of a given type
		 *
		 * Returns a NetworkResponse to send to the client(s)
		 */
		static NetworkResponse ProcessEvent(unsigned char id);
		/**
		 * \brief Processes a packet from a client
		 *
		 * Returns a NetworkResponse to send to the client(s)
		 */
		static NetworkResponse ProcessPacket(RakNet::Packet* data);

};

#endif
