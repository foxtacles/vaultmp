#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include "vaultmp.hpp"
#include "Network.hpp"

/**
 * \brief Client network interface
 */

class NetworkClient : public Network
{
		friend class Game;

	private:
		NetworkClient() = delete;

#ifdef VAULTMP_DEBUG
		static DebugInput<NetworkClient> debug;
#endif

	public:
		/**
		 * \brief Processes an event of a given type
		 *
		 * Returns a NetworkResponse to send to the server
		 */
		static NetworkResponse ProcessEvent(unsigned char id);
		/**
		 * \brief Processes a packet from the server
		 *
		 * Returns a NetworkResponse to send to the server
		 */
		static NetworkResponse ProcessPacket(RakNet::Packet* data);

};

#endif
