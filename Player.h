#ifndef PLAYER_H
#define PLAYER_H

#define TYPECLASS
#include "GameFactory.h"

#include <string>
#include <map>
#include <vector>

#include "Actor.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

const unsigned int FLAG_MOVCONTROLS      = FLAG_NOTALERTED << 1;

using namespace std;

/**
 * \brief Derives from Actor class and represents a player in-game
 *
 * Data specific to Players are a set of controls
 */

class Player : public Actor
{
friend class GameFactory;
friend class PlayerFunctor;

private:

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    map<unsigned char, pair<Value<unsigned char>, Value<bool> > > player_Controls;

    Player(const Player&);
    Player& operator=(const Player&);

protected:
    Player(unsigned int refID, unsigned int baseID);
    virtual ~Player();

public:

    /**
     * \brief Retrieves all Players
     *
     * The function obtains a lock for every Player. You have to manually release the obtained STL vector of Player pointers via GameFactory::LeaveReference
     */
    static vector<Player*> GetPlayerList();

    /**
     * \brief Creates a Parameter containing a VaultFunctor initialized with the given flags
     *
     * Used to pass Player references matching the provided flags to the Interface
     * Can also be used to pass data of a given Player to the Interface
     */
    static const Parameter CreateFunctor(unsigned int flags, NetworkID player = 0);


    /**
     * \brief Retrieves the key associated to the Player's control
     */
    unsigned char GetPlayerControl(unsigned char control) const;
    /**
     * \brief Given a control code, returns its enabled state
     */
    bool GetPlayerControlEnabled(unsigned char control) const;

    /**
     * \brief Associates a key to the Player's control code
     */
    Lockable* SetPlayerControl(unsigned char control, unsigned char key);
    /**
     * \brief Sets the enabled state of the given control code
     */
    Lockable* SetPlayerControlEnabled(unsigned char control, bool state);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

};

class PlayerFunctor : public VaultFunctor
{
private:
    NetworkID player;

public:
    PlayerFunctor(unsigned int flags, NetworkID player) : player(player), VaultFunctor(flags) {};
    virtual ~PlayerFunctor();

    virtual vector<string> operator()();
};

#endif
