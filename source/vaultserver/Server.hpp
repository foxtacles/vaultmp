#ifndef SERVER_H
#define SERVER_H

#include "vaultserver.hpp"
#include "Player.hpp"
#include "Checkbox.hpp"
#include "RadioButton.hpp"
#include "ListItem.hpp"
#include "Network.hpp"

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
		static NetworkResponse GetPos(RakNet::RakNetGUID guid, FactoryObject& reference, float X, float Y, float Z);
		/**
		 * \brief Handles GetAngle network packet
		 */
		static NetworkResponse GetAngle(RakNet::RakNetGUID guid, FactoryObject& reference, float X, float Y, float Z);
		/**
		 * \brief Handles cell network packet
		 */
		static NetworkResponse GetCell(RakNet::RakNetGUID guid, FactoryObject& reference, unsigned int cell);
		/**
		 * \brief Handles activate network packet
		 */
		static NetworkResponse GetActivate(RakNet::RakNetGUID guid, FactoryReference& reference, FactoryReference& action);
		/**
		 * \brief Handles actor state network packet
		 */
		static NetworkResponse GetActorState(RakNet::RakNetGUID guid, FactoryActor& reference, unsigned int idle, unsigned char moving, unsigned char movingxy, unsigned char weapon, bool alerted, bool sneaking);
		/**
		 * \brief Handles actor dead network packet
		 */
		static NetworkResponse GetActorDead(RakNet::RakNetGUID guid, FactoryPlayer& reference, bool dead, unsigned short limbs, signed char cause);
		/**
		 * \brief Handles actor fire weapon network packet
		 */
		static NetworkResponse GetActorFireWeapon(RakNet::RakNetGUID guid, FactoryPlayer& reference);
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
		 * \brief Handles window return network packet
		 */
		static NetworkResponse GetWindowReturn(RakNet::RakNetGUID guid, FactoryWindow& reference);
		/**
		 * \brief Handles window text network packet
		 */
		static NetworkResponse GetWindowText(RakNet::RakNetGUID guid, FactoryWindow& reference, const std::string& text);
		/**
		 * \brief Handles checkbox selected network packet
		 */
		static NetworkResponse GetCheckboxSelected(RakNet::RakNetGUID guid, FactoryCheckbox& reference, bool selected);
		/**
		 * \brief Handles radio button selected network packet
		 */
		static NetworkResponse GetRadioButtonSelected(RakNet::RakNetGUID guid, FactoryRadioButton& reference, ExpectedRadioButton& previous);
		/**
		 * \brief Handles list item selected network packet
		 */
		static NetworkResponse GetListItemSelected(RakNet::RakNetGUID guid, FactoryListItem& reference, bool selected);
		/**
		 * \brief Handles player chat message network packet
		 */
		static NetworkResponse ChatMessage(RakNet::RakNetGUID guid, const std::string& message);
};

#endif
