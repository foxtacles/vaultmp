#include <string>
#include <map>
#include <stack>

#include "../RakNet/RakPeerInterface.h"

using namespace RakNet;
using namespace std;

class Client {

      private:
              static map<RakNetGUID, Client*> clients;
              static stack<int> clientIDs;

              RakNetGUID guid; int ID;
              string authname;
              string authpwd;

      public:
              Client(RakNetGUID guid, string authname, string authpwd);
              ~Client();

              static void SetMaximumClients(int clients);
              static int GetClientCount();
              static Client* GetClientFromGUID(RakNetGUID guid);

              RakNetGUID GetRakNetGUID();
              int GetClientID();
              string GetAuthName();
              string GetAuthPwd();
};
