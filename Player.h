#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <map>
#include <vector>

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

    static vector<string> GetRefs(bool enabled = true, bool notself = false, bool enabled_disabled = false, bool self_notself = false);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    void Startup(RakNetGUID guid);

    RakNetGUID guid;
    string pwd;

public:
    Player(RakNetGUID guid, unsigned int baseID);
    Player(RakNetGUID guid, unsigned int refID, unsigned int baseID, string pwd);
    ~Player();

    RakNetGUID GetPlayerGUID();
    string GetPlayerPassword();

    static void Initialize();
    static void DestroyInstances();

    static map<RakNetGUID, Player*> GetPlayerList();
    static vector<RakNetGUID> GetPlayerNetworkList();
    static Player* GetPlayerFromGUID(RakNetGUID guid);
    static Player* GetPlayerFromRefID(unsigned int refID);
    static vector<string> GetAllRefs();
    static vector<string> GetAllRefs_NotSelf();
    static vector<string> GetEnabledRefs();
    static vector<string> GetEnabledRefs_NotSelf();

    static Parameter Param_EnabledPlayers;
    static Parameter Param_EnabledPlayers_NotSelf;
    static Parameter Param_AllPlayers;
    static Parameter Param_AllPlayers_NotSelf;

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

};

#endif
