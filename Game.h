#ifndef GAME_H
#define GAME_H

#include "vaultmp.h"
#include "PacketFactory.h"
#include "Interface.h"
#include "Player.h"
#include "Network.h"
#include "VaultException.h"

#include <chrono>

/**
 * \brief Client game code, using the Interface to execute commands and communicate with the game
 *
 * Used by the command result handler and the NetworkClient class, also creates and queues packets
 */

class Game
{
		friend class NetworkClient;
		friend class Bethesda;

	private:

		Game();

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		static unsigned char game;
		static RakNetGUID server;

		static void AdjustZAngle( double& Z, double diff );

	public:

		/**
		 * \brief Initializes the Game class
		 */
		static void Initialize();
		/**
		 * \brief Builds an authenticate packet for the server
		 */
		static NetworkResponse Authenticate( string password );
		/**
		 * \brief Starts the game command schedule
		 */
		static void Startup();
		/**
		 * \brief Future set
		 */
        template <typename T>
		static void FutureSet( Lockable* data, T t );

        /**
         * Game functions
         */

		/**
		 * \brief Loads a savegame
		 */
		static void LoadGame( string savegame );
		/**
		 * \brief Loads the environment after savegame load
		 */
		static void LoadEnvironment();
		/**
		 * \brief Creates a new Object
		 */
		static void NewObject( FactoryObject reference );
		/**
		 * \brief Creates a new Item
		 */
		static void NewItem( FactoryObject reference );
		/**
		 * \brief Creates a new Container
		 */
		static void NewContainer( FactoryObject reference );
		/**
		 * \brief Creates a new Actor
		 */
		static void NewActor( FactoryObject reference );
		/**
		 * \brief Creates a new Player
		 */
		static void NewPlayer( FactoryObject reference );
		/**
		 * \brief Removes an Object from the game
		 */
		static void RemoveObject( FactoryObject& reference );
		/**
		 * \brief Enables / Disables an Object
		 */
		static void ToggleEnabled( FactoryObject reference );
		/**
		 * \brief Deletes an Object
		 */
		static void Delete( FactoryObject& reference );
		/**
		 * \brief Sets the name of an Object
		 */
		static void SetName( FactoryObject reference );
		/**
		 * \brief Puts an Actor into restrained / unrestrained state
		 */
		static void SetRestrained( FactoryObject reference, bool restrained );
		/**
		 * \brief Sets the position of an Object
		 */
		static void SetPos( FactoryObject reference  );
		/**
		 * \brief Sets the angles of an Object
		 */
		static void SetAngle( FactoryObject reference );
		/**
		 * \brief Moves an Object to another Object
		 */
		static void MoveTo( vector<FactoryObject> reference, bool cell = false, signed int key = 0 );
		/**
		 * \brief Sets an actor value of an Actor
		 */
		static void SetActorValue( FactoryObject reference, bool base, unsigned char index, signed int key = 0 );
		/**
		 * \brief Sets the sneaking state of an Actor
		 */
		static void SetActorSneaking( FactoryObject reference, signed int key = 0 );
		/**
		 * \brief Sets the alerted state of an Actor
		 */
		static void SetActorAlerted( FactoryObject reference, signed int key = 0 );
		/**
		 * \brief Sets the moving animation of an Actor
		 */
		static void SetActorMovingAnimation( FactoryObject reference, signed int key = 0 );

        /**
         * Network functions
         */

		/**
		 * \brief Network function to handle Object position
		 */
		static void net_SetPos( FactoryObject reference, double X, double Y, double Z);
		/**
		 * \brief Network function to handle Object position
		 */
		static void net_SetAngle( FactoryObject reference, unsigned char axis, double value );
		/**
		 * \brief Network function to handle Object cell
		 */
		static void net_SetCell( vector<FactoryObject> reference, unsigned int cell );
		/**
		 * \brief Network function to handle Actor value
		 */
		static void net_SetActorValue( FactoryObject reference, bool base, unsigned char index, double value );
		/**
		 * \brief Network function to handle Actor state
		 */
		static void net_SetActorState( FactoryObject reference, unsigned char index, unsigned char moving, bool alerted, bool sneaking );

        /**
         * Interface functions
         */

		/**
		 * \brief Handles GetPos command result
		 */
		static void GetPos( FactoryObject reference, unsigned char axis, double value );
		/**
		 * \brief Handles GetAngle command result
		 */
		static void GetAngle( FactoryObject reference, unsigned char axis, double value );
		/**
		 * \brief Handles GetParentCell command result
		 */
		static void GetParentCell( vector<FactoryObject> reference, unsigned int cell );
		/**
		 * \brief Handles GetActorValue command result
		 */
		static void GetActorValue( FactoryObject reference, bool base, unsigned char index, double value );
		/**
		 * \brief Handles GetActorState command result
		 */
		static void GetActorState( FactoryObject reference, unsigned char index, unsigned char moving, bool alerted, bool sneaking );
		/**
		 * \brief Handles GetControl command result
		 */
		static void GetControl( FactoryObject reference, unsigned char control, unsigned char key );
		/**
		 * \brief Handles ScanContainer command result
		 */
		static void ScanContainer( FactoryObject reference, vector<unsigned char>& data );

		/**
		 * \brief Handles a failed PlaceAtMe command
		 */
		static void Failure_PlaceAtMe( unsigned int refID, unsigned int baseID, unsigned int count, signed int key );

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler( Debug* debug );
#endif
};

#endif
