#include <string>
#include <map>

#include "Client.h"

#include "RakNet/RakPeerInterface.h"

using namespace RakNet;
using namespace std;

class Player {

      private:
              static map<RakNetGUID, string> players;
              static map<RakNetGUID, Player*> playersguids;

              RakNetGUID guid;
              string refID;

              string name;
              float pos[3];

      public:
              Player(RakNetGUID guid);
              ~Player();

              static map<RakNetGUID, string> GetPlayerList();
              static Player* GetPlayerFromGUID(RakNetGUID guid);

              string GetPlayerName();
              float GetPlayerPos(int cell);
              string GetPlayerRefID();

              void SetPlayerName(string name);
              void SetPlayerPos(int cell, float pos);
              void SetPlayerRefID(string refID);
};
