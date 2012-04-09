#ifndef SERVER_H
#define SERVER_H

#include "Dedicated.h"
#include "Client.h"
#include "../PacketTypes.h"
#include "../Player.h"
#include "../Network.h"
#include "../VaultException.h"

/**
 * \brief Server game code, communicating with loaded scripts
 *
 * Creates and returns packets
 */

class Server
{
		friend class NetworkServer;
		friend class Dedicated;

	private:

		Server();

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

	public:

		/**
		 * \brief Authenticates a client
		 */
		static NetworkResponse Authenticate( RakNetGUID guid, string name, string pwd );
		/**
		 * \brief Sends the game world data
		 */
		static NetworkResponse LoadGame( RakNetGUID guid );
		/**
		 * \brief Creates a new Player
		 */
		static NetworkResponse NewPlayer( RakNetGUID guid, NetworkID id );
		/**
		 * \brief Disconnects a client
		 */
		static NetworkResponse Disconnect( RakNetGUID guid, unsigned char reason );

		/**
		 * \brief Handles GetPos network packet
		 */
		static NetworkResponse GetPos( RakNetGUID guid, FactoryObject reference, double X, double Y, double Z );
		/**
		 * \brief Handles GetAngle network packet
		 */
		static NetworkResponse GetAngle( RakNetGUID guid, FactoryObject reference, unsigned char axis, double value );
		/**
		 * \brief Handles cell network packet
		 */
		static NetworkResponse GetCell( RakNetGUID guid, FactoryObject reference, unsigned int cell );
		/**
		 * \brief Handles container update network packet
		 */
		static NetworkResponse GetContainerUpdate( RakNetGUID guid, FactoryObject reference, ContainerDiff diff );
		/**
		 * \brief Handles actor value network packet
		 */
		static NetworkResponse GetActorValue( RakNetGUID guid, FactoryObject reference, bool base, unsigned char index, double value );
		/**
		 * \brief Handles actor state network packet
		 */
		static NetworkResponse GetActorState( RakNetGUID guid, FactoryObject reference, unsigned char index, unsigned char moving, bool alerted, bool sneaking );
		/**
		 * \brief Handles player control network packet
		 */
		static NetworkResponse GetPlayerControl( RakNetGUID guid, FactoryObject reference, unsigned char control, unsigned char key );

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler( Debug* debug );
#endif
};

#endif
