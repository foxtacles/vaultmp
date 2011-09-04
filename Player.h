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

using namespace Data;
using namespace std;

class Player : public Actor
{
friend class GameFactory;

private:
    static vector<string> GetRefs(bool enabled = true, bool notself = false, bool enabled_disabled = false, bool self_notself = false);

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
    static vector<string> GetAllRefs();
    static vector<string> GetAllRefs_NotSelf();
    static vector<string> GetEnabledRefs();
    static vector<string> GetEnabledRefs_NotSelf();

    static const Parameter Param_EnabledPlayers;
    static const Parameter Param_EnabledPlayers_NotSelf;
    static const Parameter Param_AllPlayers;
    static const Parameter Param_AllPlayers_NotSelf;

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

};

#endif
