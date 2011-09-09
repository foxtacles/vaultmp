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

public:

    /**
     * \brief Initializes the game
     */
    static void Initialize();
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
    static void PlayerLeft(NetworkID id);
    /**
     * \brief Enables / Disables an Object
     */
    static void Enable(NetworkID id, bool enable);
    /**
     * \brief Deletes an Object
     */
    static void Delete(NetworkID id);
    /**
     * \brief Sets the name of an Object
     */
    static void SetName(NetworkID id, string name);
    /**
     * \brief Resets the name of an Object
     */
    static void SetName(unsigned int refID);
    /**
     * \brief Puts an Actor into restrained / unrestrained state
     */
    static void SetRestrained(NetworkID id, bool restrained, unsigned int delay = 2);
    /**
     * \brief Sets the position of an Object
     */
    static void SetPos(NetworkID id, unsigned char axis, double value);
    /**
     * \brief Sets the angle of an Object
     */
    static void SetAngle(NetworkID id, unsigned char axis, double value);
    /**
     * \brief Resets the angle of an Object
     */
    static void SetAngle(unsigned int refID, unsigned char axis);
    /**
     * \brief Sets the network cell of an Object
     */
    static void SetNetworkCell(NetworkID id, unsigned int cell);
    /**
     * \brief Sets an actor value of an Actor
     */
    static void SetActorValue(NetworkID id, bool base, unsigned char index, double value);
    /**
     * \brief Sets the running animation and alerted state of an Actor
     */
    static void SetActorState(NetworkID id, unsigned char index, bool alerted);
    /**
     * \brief Moves an Object to another Object
     */
    static void MoveTo(NetworkID id, NetworkID id2, bool cell = false);


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
    static void GetPos(unsigned int refID, unsigned char axis, double value);
    /**
     * \brief Handles GetAngle command result
     */
    static void GetAngle(unsigned int refID, unsigned char axis, double value);
    /**
     * \brief Handles GetParentCell command result
     */
    static void GetParentCell(unsigned int refID, unsigned int cell);
    /**
     * \brief Handles GetActorValue command result
     */
    static void GetActorValue(unsigned int refID, bool base, unsigned char index, double value);
    /**
     * \brief Handles GetActorState  command result
     */
    static void GetActorState(unsigned int refID, unsigned char index, bool alerted);


    /**
     * \brief Handles a failed PlaceAtMe command
     */
    static void Failure_PlaceAtMe(unsigned int refID, unsigned int baseID, unsigned int count, signed int key);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif
};

#endif
