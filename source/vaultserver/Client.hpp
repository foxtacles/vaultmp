#ifndef CLIENT_H
#define CLIENT_H

#include "vaultserver.hpp"
#include "Guarded.hpp"
#include "RakNet.hpp"

#include <vector>
#include <stack>
#include <map>

class Player;

/**
 * \brief The Client class contains information about a connected client
 */

class Client
{
	private:
		static Guarded<> cs;
		static std::map<RakNet::RakNetGUID, Client*> clients;
		static std::stack<unsigned int> clientID;

		RakNet::RakNetGUID guid;
		unsigned int ID;
		RakNet::NetworkID player;

		Client(const Client&) = delete;
		Client& operator=(const Client&) = delete;

	public:
		Client(RakNet::RakNetGUID guid, RakNet::NetworkID player);
		~Client();

		/**
		 * \brief Sets the maximum amount of connected clients. This should be called in the initial startup procedure of the server
		 */
		static void SetMaximumClients(unsigned int clients);
		/**
		 * \brief Returns the amount of connected clients
		 */
		static unsigned int GetClientCount();
		/**
		 * \brief Given the RakNetGUID, returns the Client
		 */
		static Client* GetClientFromGUID(RakNet::RakNetGUID guid);
		/**
		 * \brief Given the ID, returns the Client
		 */
		static Client* GetClientFromID(unsigned int id);
		/**
		 * \brief Given the NetworkID, returns the Client
		 */
		static Client* GetClientFromPlayer(RakNet::NetworkID id);
		/**
		 * \brief Returns a STL vector containing every RakNetGUID
		 *
		 * except (optional, Client*) - excludes a RakNetGUID from the result
		 */
		static std::vector<RakNet::RakNetGUID> GetNetworkList(Client* except = nullptr);
		/**
		 * \brief Returns a STL vector containing every RakNetGUID
		 *
		 * except (optional, RakNetGUID) - excludes a RakNetGUID from the result
		 */
		static std::vector<RakNet::RakNetGUID> GetNetworkList(RakNet::RakNetGUID except = RakNet::UNASSIGNED_RAKNET_GUID);
		/**
		 * \brief Returns a STL vector containing the RakNetGUIDs matching the player IDs
		 */
		static std::vector<RakNet::RakNetGUID> GetNetworkList(const std::vector<RakNet::NetworkID>& players, RakNet::RakNetGUID except = RakNet::UNASSIGNED_RAKNET_GUID);

		/**
		 * \brief Returns the RakNetGUID
		 */
		RakNet::RakNetGUID GetGUID();
		/**
		 * \brief Returns the ID
		 */
		unsigned int GetID();
		/**
		 * \brief Returns the NetworkID of the coressponding Player instance
		 */
		RakNet::NetworkID GetPlayer();

};

#endif
