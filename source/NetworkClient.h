#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include "Network.h"

/**
 * \brief Client network interface
 */

class NetworkClient : public Network
{
		friend class Game;

	private:

		NetworkClient();

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

	public:

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

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
