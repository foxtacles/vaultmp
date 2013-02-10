#ifndef SERVER_H
#define SERVER_H

#include "Dedicated.h"
#include "Client.h"
#include "Player.h"
#include "Network.h"
#include "VaultException.h"

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
		Server() = delete;

#ifdef VAULTMP_DEBUG
		static DebugInput<Server> debug;
#endif

	public:
		/**
		 * \brief Authenticates a client
		 */
		static NetworkResponse Authenticate(RakNet::RakNetGUID guid, const std::string& name, const std::string& pwd);
		/**
		 * \brief Sends the game world data
		 */
		static NetworkResponse LoadGame(RakNet::RakNetGUID guid);
		/**
		 * \brief Creates a new Player
		 */
		static NetworkResponse NewPlayer(RakNet::RakNetGUID guid, RakNet::NetworkID id);
		/**
		 * \brief Disconnects a client
		 */
		static NetworkResponse Disconnect(RakNet::RakNetGUID guid, Reason reason);

		/**
		 * \brief Handles GetPos network packet
		 */
		static NetworkResponse GetPos(RakNet::RakNetGUID guid, const FactoryObject<Object>& reference, double X, double Y, double Z);
		/**
		 * \brief Handles GetAngle network packet
		 */
		static NetworkResponse GetAngle(RakNet::RakNetGUID guid, const FactoryObject<Object>& reference, unsigned char axis, double value);
		/**
		 * \brief Handles cell network packet
		 */
		static NetworkResponse GetCell(RakNet::RakNetGUID guid, const FactoryObject<Object>& reference, unsigned int cell);
		/**
		 * \brief Handles lock update packet
		 */
		static NetworkResponse GetLock(RakNet::RakNetGUID guid, const FactoryObject<Object>& reference, const FactoryObject<Player>& player, unsigned int lock);
		/**
		 * \brief Handles container update network packet
		 */
		static NetworkResponse GetContainerUpdate(RakNet::RakNetGUID guid, const FactoryObject<Container>& reference, const Container::NetDiff& ndiff, const Container::NetDiff& gdiff);
		/**
		 * \brief Handles actor value network packet
		 */
		static NetworkResponse GetActorValue(RakNet::RakNetGUID guid, const FactoryObject<Actor>& reference, bool base, unsigned char index, double value);
		/**
		 * \brief Handles actor state network packet
		 */
		static NetworkResponse GetActorState(RakNet::RakNetGUID guid, const FactoryObject<Actor>& reference, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking);
		/**
		 * \brief Handles actor dead network packet
		 */
		static NetworkResponse GetActorDead(RakNet::RakNetGUID guid, const FactoryObject<Actor>& reference, const FactoryObject<Player>& killer, bool dead, unsigned short limbs, signed char cause);
		/**
		 * \brief Handles player control network packet
		 */
		static NetworkResponse GetPlayerControl(RakNet::RakNetGUID guid, const FactoryObject<Player>& reference, unsigned char control, unsigned char key);
		/**
		 * \brief Handles player chat message network packet
		 */
		static NetworkResponse ChatMessage(RakNet::RakNetGUID guid, std::string message);
};

#endif
