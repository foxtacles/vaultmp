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
    static vector<Player*> GetPlayerList();

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
