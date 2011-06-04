#ifndef SERVERENTRY_H
#define SERVERENTRY_H

#include <string>
#include <map>

using namespace std;

class ServerEntry {

      private:
              string name;
              string map;
              std::map<string, string> rules;
              pair<int, int> players;
              int ping;

      public:
              void SetServerName(string name);
              void SetServerMap(string map);
              void SetServerRule(string rule, string value);
              void SetServerPlayers(pair<int, int> players);
              void SetServerPing(int ping);

              string GetServerName();
              string GetServerMap();
              std::map<string, string> GetServerRules();
              pair<int, int> GetServerPlayers();
              int GetServerPing();

              ServerEntry();
              ServerEntry(string name, string map, pair<int, int> players, int ping);
};

#endif
