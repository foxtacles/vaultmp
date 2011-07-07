#ifndef PLAYER_H
#define PLAYER_H

#define VAULTMP_DEBUG

#include <string>
#include <map>
#include <math.h>

#include "Client.h"
#include "Data.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#include "RakNet/RakPeerInterface.h"

using namespace RakNet;
using namespace Data;
using namespace std;

class Player {

      private:
              static map<RakNetGUID, string> players;
              static map<RakNetGUID, Player*> playersguids;
              static map<string, Player*> playersrefs;

              #ifdef VAULTMP_DEBUG
              static Debug* debug;
              #endif

              RakNetGUID guid;
              string refID;

              string name;
              float pos[3];
              float angle;
              DWORD gcell;
              DWORD ncell;
              float health;
              float baseHealth;
              float cond[6];
              bool dead;
              bool alerted;
              int moving;

              bool enabled;
              bool nowrite[MAX_SKIP_FLAGS];

      public:
              Player(RakNetGUID guid);
              ~Player();

              static map<RakNetGUID, string> GetPlayerList();
              static Player* GetPlayerFromGUID(RakNetGUID guid);
              static Player* GetPlayerFromRefID(string refID);
              static void DestroyInstances();

              #ifdef VAULTMP_DEBUG
              static void SetDebugHandler(Debug* debug);
              #endif

              string GetPlayerName();
              float GetPlayerPos(int cell);
              float GetPlayerAngle();
              DWORD GetPlayerGameCell();
              DWORD GetPlayerNetworkCell();
              float GetPlayerHealth();
              float GetPlayerBaseHealth();
              float GetPlayerCondition(int cell);
              bool IsPlayerDead();
              bool IsPlayerAlerted();
              int GetPlayerMoving();
              string GetPlayerRefID();
              bool GetPlayerEnabled();

              pPlayerUpdate GetPlayerUpdateStruct();
              pPlayerStateUpdate GetPlayerStateUpdateStruct();
              pPlayerCellUpdate GetPlayerCellUpdateStruct();
              bool UpdatePlayerUpdateStruct(pPlayerUpdate* data);
              bool UpdatePlayerStateUpdateStruct(pPlayerStateUpdate* data);
              bool UpdatePlayerCellUpdateStruct(pPlayerCellUpdate* data);

              bool SetPlayerName(string name);
              bool SetPlayerPos(int cell, float pos);
              bool SetPlayerAngle(float angle);
              bool SetPlayerGameCell(DWORD cell);
              bool SetPlayerNetworkCell(DWORD cell);
              bool SetPlayerHealth(float health);
              bool SetPlayerBaseHealth(float baseHealth);
              bool SetPlayerCondition(int cell, float cond);
              bool SetPlayerDead(bool dead);
              bool SetPlayerAlerted(bool alerted);
              bool SetPlayerMoving(int moving);
              bool SetPlayerRefID(string refID);
              bool SetPlayerEnabled(bool enabled);

              bool ToggleNoOverride(int skipflag, bool toggle);
              bool GetPlayerOverrideFlag(int skipflag);

              bool IsPlayerNearPoint(float X, float Y, float Z, float R);
              bool IsCoordinateInRange(int cell, float XYZ, float R);
};

#endif
