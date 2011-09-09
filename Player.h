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

using namespace std;

/**
 * \brief Derives from Actor class and represents a player in-game
 */

class Player : public Actor
{
friend class GameFactory;
friend class PlayerFunctor;

private:

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

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
     */
    static const Parameter CreateFunctor(unsigned int flags);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

};

class PlayerFunctor : public VaultFunctor
{
public:
    PlayerFunctor(unsigned int flags) : VaultFunctor(flags) {};
    virtual ~PlayerFunctor();

    virtual vector<string> operator()();
};

#endif
