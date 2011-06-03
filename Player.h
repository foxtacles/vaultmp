#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <map>
#include <math.h>

#include "Client.h"

#include "RakNet/RakPeerInterface.h"

using namespace RakNet;
using namespace std;

class Player {

      private:
              static map<RakNetGUID, string> players;
              static map<RakNetGUID, Player*> playersguids;
              static map<string, Player*> playersrefs;

              RakNetGUID guid;
              string refID;

              string name;
              float pos[3];
              float angle;
              float health;
              float baseHealth;
              float cond[6];
              bool dead;
              int moving;

      public:
              Player(RakNetGUID guid);
              ~Player();

              static map<RakNetGUID, string> GetPlayerList();
              static Player* GetPlayerFromGUID(RakNetGUID guid);
              static Player* GetPlayerFromRefID(string refID);

              string GetPlayerName();
              float GetPlayerPos(int cell);
              float GetPlayerAngle();
              float GetPlayerHealth();
              float GetPlayerBaseHealth();
              float GetPlayerCondition(int cell);
              bool IsPlayerDead();
              int GetPlayerMoving();
              string GetPlayerRefID();

              void SetPlayerName(string name);
              void SetPlayerPos(int cell, float pos);
              void SetPlayerAngle(float angle);
              void SetPlayerHealth(float health);
              void SetPlayerBaseHealth(float baseHealth);
              void SetPlayerCondition(int cell, float cond);
              void SetPlayerDead(bool dead);
              void SetPlayerMoving(int moving);
              void SetPlayerRefID(string refID);

              bool IsPlayerNearPoint(float X, float Y, float Z, float R);
};

#endif
