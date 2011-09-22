#ifndef GAME_H
#define GAME_H

#include "vaultmp.h"
#include "PacketTypes.h"
#include "Interface.h"
#include "Player.h"
#include "Network.h"
#include "VaultException.h"

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

    static void AdjustZAngle(double& Z, double diff);

public:

    /**
     * \brief Initializes the Game class
     */
    static void Initialize();
    /**
     * \brief Starts the game command schedule
     */
    static void Startup();
    /**
     * \brief Loads a savegame
     */
    static void LoadGame(string savegame);
    /**
     * \brief Creates a new Player
     */
    static void NewPlayer(NetworkID id, unsigned int baseID, string name);
    /**
     * \brief Removes a Player from the game
     */
    static void PlayerLeft(FactoryObject& reference);
    /**
     * \brief Enables / Disables an Object
     */
    static void Enable(FactoryObject reference, bool enable);
    /**
     * \brief Deletes an Object
     */
    static void Delete(FactoryObject& reference);
    /**
     * \brief Sets the name of an Object
     */
    static void SetName(FactoryObject reference, string name);
    /**
     * \brief Puts an Actor into restrained / unrestrained state
     */
    static void SetRestrained(FactoryObject reference, bool restrained, unsigned int delay = 2);
    /**
     * \brief Sets the position of an Object
     */
    static void SetPos(FactoryObject reference, double X, double Y, double Z);
    /**
     * \brief Resets the position of an Object
     */
    static void SetPos(FactoryObject reference);
    /**
     * \brief Sets the angle of an Object
     */
    static void SetAngle(FactoryObject reference, unsigned char axis, double value);
    /**
     * \brief Resets the angle of an Object
     */
    static void SetAngle(FactoryObject reference, unsigned char axis);
    /**
     * \brief Sets the network cell of an Object
     */
    static void SetNetworkCell(vector<FactoryObject> reference, unsigned int cell);
    /**
     * \brief Sets an actor value of an Actor
     */
    static void SetActorValue(FactoryObject reference, bool base, unsigned char index, double value);
    /**
     * \brief Sets the running animation, alerted and sneaking state of an Actor
     */
    static void SetActorState(FactoryObject reference, unsigned char index, unsigned char moving, bool alerted, bool sneaking);
    /**
     * \brief Moves an Object to another Object
     */
    static void MoveTo(vector<FactoryObject> reference, bool cell = false);


    /**
     * \brief Builds an authenticate packet for the server
     */
    static NetworkResponse Authenticate(string password);
    /**
     * \brief Handles PlaceAtMe command result
     */
    static void PlaceAtMe(Lockable* data, unsigned int refID);
    /**
     * \brief Handles GetPos command result
     */
    static void GetPos(FactoryObject reference, unsigned char axis, double value);
    /**
     * \brief Handles GetAngle command result
     */
    static void GetAngle(FactoryObject reference, unsigned char axis, double value);
    /**
     * \brief Handles GetParentCell command result
     */
    static void GetParentCell(vector<FactoryObject> reference, unsigned int cell);
    /**
     * \brief Handles GetActorValue command result
     */
    static void GetActorValue(FactoryObject reference, bool base, unsigned char index, double value);
    /**
     * \brief Handles GetActorState command result
     */
    static void GetActorState(FactoryObject reference, unsigned char index, unsigned char moving, bool alerted, bool sneaking);
    /**
     * \brief Handles GetControl command result
     */
    static void GetControl(FactoryObject reference, unsigned char control, unsigned char key);
    /**
     * \brief Handles ScanContainer command result
     */
    static void ScanContainer(FactoryObject reference, vector<unsigned char>& data);

    /**
     * \brief Handles a failed PlaceAtMe command
     */
    static void Failure_PlaceAtMe(unsigned int refID, unsigned int baseID, unsigned int count, signed int key);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif
};

#endif
