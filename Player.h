#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <map>
#include <list>

#include "Actor.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#include "RakNet/RakPeerInterface.h"

using namespace RakNet;
using namespace Data;
using namespace std;

class Player : public Actor
{

private:
    static map<RakNetGUID, Player*> playerlist;
    static bool initialized;

    static vector<string> GetRefs(string cmd, bool enabled, bool notself);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    void Startup(RakNetGUID guid);

    RakNetGUID guid;

public:
    Player(RakNetGUID guid, string base);
    Player(RakNetGUID guid, string ref, string base);
    ~Player();

    static void Initialize();
    static void DestroyInstances();

    static map<RakNetGUID, Player*> GetPlayerList();
    static Player* GetPlayerFromGUID(RakNetGUID guid);
    static Player* GetPlayerFromRefID(string refID);
    static vector<string> GetAllRefs(string cmd);
    static vector<string> GetAllRefs_NotSelf(string cmd);
    static vector<string> GetEnabledRefs(string cmd);
    static vector<string> GetEnabledRefs_NotSelf(string cmd);

    static Parameter Param_EnabledPlayers;
    static Parameter Param_EnabledPlayers_NotSelf;
    static Parameter Param_AllPlayers;
    static Parameter Param_AllPlayers_NotSelf;

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

};

#endif
