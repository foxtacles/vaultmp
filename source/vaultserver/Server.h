#ifndef SERVER_H
#define SERVER_H

#include "Dedicated.h"
#include "Client.h"
#include "Player.h"
#include "Window.h"
#include "Network.h"
#include "VaultException.h"
#include "GameFactory.h"

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
		static NetworkResponse GetPos(RakNet::RakNetGUID guid, FactoryObject& reference, double X, double Y, double Z);
		/**
		 * \brief Handles GetAngle network packet
		 */
		static NetworkResponse GetAngle(RakNet::RakNetGUID guid, FactoryObject& reference, unsigned char axis, double value);
		/**
		 * \brief Handles cell network packet
		 */
		static NetworkResponse GetCell(RakNet::RakNetGUID guid, FactoryObject& reference, unsigned int cell);
		/**
		 * \brief Handles lock update packet
		 */
		static NetworkResponse GetLock(RakNet::RakNetGUID guid, FactoryObject& reference, FactoryPlayer& player, unsigned int lock);
		/**
		 * \brief Handles container update network packet
		 */
		static NetworkResponse GetContainerUpdate(RakNet::RakNetGUID guid, FactoryContainer& reference, const ItemList::NetDiff& ndiff, const ItemList::NetDiff& gdiff);
		/**
		 * \brief Handles actor value network packet
		 */
		static NetworkResponse GetActorValue(RakNet::RakNetGUID guid, FactoryActor& reference, bool base, unsigned char index, double value);
		/**
		 * \brief Handles actor state network packet
		 */
		static NetworkResponse GetActorState(RakNet::RakNetGUID guid, FactoryActor& reference, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking);
		/**
		 * \brief Handles actor dead network packet
		 */
		static NetworkResponse GetActorDead(RakNet::RakNetGUID guid, FactoryActor& reference, FactoryPlayer& killer, bool dead, unsigned short limbs, signed char cause);
		/**
		 * \brief Handles player control network packet
		 */
		static NetworkResponse GetPlayerControl(RakNet::RakNetGUID guid, FactoryPlayer& reference, unsigned char control, unsigned char key);
		/**
		 * \brief Handles window mode network packet
		 */
		static NetworkResponse GetWindowMode(RakNet::RakNetGUID guid, bool enabled);
		/**
		 * \brief Handles window click network packet
		 */
		static NetworkResponse GetWindowClick(RakNet::RakNetGUID guid, FactoryWindow& reference);
		/**
		 * \brief Handles window click network packet
		 */
		static NetworkResponse GetWindowText(RakNet::RakNetGUID guid, FactoryWindow& reference, const std::string& text);
		/**
		 * \brief Handles player chat message network packet
		 */
		static NetworkResponse ChatMessage(RakNet::RakNetGUID guid, std::string message);
};

#endif
