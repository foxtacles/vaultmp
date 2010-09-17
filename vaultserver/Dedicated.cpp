#include "Dedicated.h"

using namespace RakNet;
using namespace std;

RakPeerInterface* Dedicated::peer;
SocketDescriptor* Dedicated::sockdescr;
AMX* Dedicated::script;

#ifdef ANNOUNCE
SystemAddress Dedicated::master;
TimeMS Dedicated::announcetime;
#endif

ServerEntry Dedicated::self;

enum {
    ID_MASTER_QUERY = ID_USER_PACKET_ENUM,
    ID_MASTER_ANNOUNCE,
    ID_MASTER_UPDATE,
    ID_GAME_INIT,
    ID_GAME_RUN,
    ID_GAME_END
};

bool Dedicated::thread;

void Dedicated::TerminateThread()
{
      thread = false;
}

#ifdef ANNOUNCE

void Dedicated::Announce(bool announce)
{
    if (peer->GetConnectionState(master) == IS_CONNECTED)
    {
        BitStream query;

        if (announce)
        {
            query.Write((MessageID) ID_MASTER_ANNOUNCE);
            query.Write((bool) true);

            RakString name(self.GetServerName().c_str()); RakString map(self.GetServerMap().c_str());
            int players = self.GetServerPlayers().first; int playersMax = self.GetServerPlayers().second;
            std::map<string, string> rules = self.GetServerRules();

            query.Write(name);
            query.Write(map);
            query.Write(players);
            query.Write(playersMax);
            query.Write(rules.size());

            for (std::map<string, string>::const_iterator i = rules.begin(); i != rules.end(); ++i)
            {
                RakString key(i->first.c_str());
                RakString value(i->second.c_str());
                query.Write(key);
                query.Write(value);
            }
        }
        else
        {
            query.Write((MessageID) ID_MASTER_ANNOUNCE);
            query.Write((bool) false);
        }

        peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, master, false, 0);
    }
    else
    {
        Utils::timestamp();
        printf("Lost connection to MasterServer (%s)\n", master.ToString());
    }

    announcetime = GetTimeMS();
}

#endif

DWORD WINAPI Dedicated::DedicatedThread(LPVOID data)
{
      sockdescr = new SocketDescriptor(RAKNET_PORT, 0);
      peer = RakPeerInterface::GetInstance();
      peer->Startup(RAKNET_CONNECTIONS, sockdescr, 1, THREAD_PRIORITY_NORMAL);
      #ifdef ANNOUNCE
      peer->SetMaximumIncomingConnections(RAKNET_CONNECTIONS - 1);
      #else
      peer->SetMaximumIncomingConnections(RAKNET_CONNECTIONS);
      #endif

      Packet* packet;

      #ifdef ANNOUNCE
      master.SetBinaryAddress(RAKNET_MASTER_ADDRESS);
      master.port = RAKNET_MASTER_PORT;
      peer->Connect(RAKNET_MASTER_ADDRESS, RAKNET_MASTER_PORT, 0, 0, 0, 0, 3, 500, 0);
      announcetime = GetTimeMS();
      #endif

      self.SetServerName("testserver");
      self.SetServerMap("asdf");
      self.SetServerPlayers(pair<int, int>(0, 32));
      self.SetServerPing(123);
      self.SetServerRule("asdf", "kokolores");
      self.SetServerRule("blub", "moep");

      while (thread)
      {
            for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
            {
                switch (packet->data[0])
                {
                    case ID_NEW_INCOMING_CONNECTION:
                        Utils::timestamp();
                        printf("New incoming connection from %s\n", packet->systemAddress.ToString());
                        break;
                    case ID_DISCONNECTION_NOTIFICATION:
                        Utils::timestamp();
                        printf("Client disconnected (%s)\n", packet->systemAddress.ToString());
                        break;
                    case ID_CONNECTION_LOST:
                        Utils::timestamp();
                        printf("Lost connection (%s)\n", packet->systemAddress.ToString());
                        break;
                    case ID_CONNECTION_REQUEST_ACCEPTED:
                    {
                        master = packet->systemAddress;
                        Announce(true);
                        Utils::timestamp();
                        printf("Connected to MasterServer (%s)\n", packet->systemAddress.ToString());
                        break;
                    }
                    case ID_MASTER_UPDATE:
                    {
                        #ifdef QUERY

                        BitStream query(packet->data, packet->length, false);
                        query.IgnoreBytes(sizeof(MessageID));

                        SystemAddress addr;
                        query.Read(addr);
                        query.Reset();

                        query.Write((MessageID) ID_MASTER_UPDATE);
                        query.Write(addr);

                        RakString name(self.GetServerName().c_str()); RakString map(self.GetServerMap().c_str());
                        int players = self.GetServerPlayers().first; int playersMax = self.GetServerPlayers().second;
                        std::map<string, string> rules = self.GetServerRules();

                        query.Write(name);
                        query.Write(map);
                        query.Write(players);
                        query.Write(playersMax);
                        query.Write((int) rules.size());

                        for (std::map<string, string>::const_iterator i = rules.begin(); i != rules.end(); ++i)
                        {
                            RakString key(i->first.c_str());
                            RakString value(i->second.c_str());
                            query.Write(key);
                            query.Write(value);
                        }

                        peer->Send(&query, LOW_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);

                        Utils::timestamp();
                        printf("Query processed (%s)\n", packet->systemAddress.ToString());
                        break;

                        #else

                        Utils::timestamp();
                        printf("Query is disabled (%s)\n", packet->systemAddress.ToString());
                        peer->CloseConnection(packet->systemAddress, true, 0, LOW_PRIORITY);
                        break;

                        #endif
                    }
                    case ID_GAME_INIT:
                    {
                        BitStream query(packet->data, packet->length, false);
                        query.IgnoreBytes(sizeof(MessageID));

                        RakString rname, rpwd;
                        query.Read(rname);
                        query.Read(rpwd);
                        query.Reset();

                        int ret = 1;

                        if (script != NULL)
                        {
                            void* args[2];

                            char name[rname.GetLength()];
                            char pwd[rpwd.GetLength()];
                            strcpy(name, rname.C_String());
                            strcpy(pwd, rpwd.C_String());

                            args[0] = reinterpret_cast<void*>(pwd);
                            args[1] = reinterpret_cast<void*>(name);

                            ret = Script::Call(script, (char*) "OnClientAuthenticate", (char*) "ss", args);
                        }

                        if (ret)
                        {
                            query.Write((MessageID) ID_GAME_INIT);
                            peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);
                        }
                        else
                            peer->CloseConnection(packet->systemAddress, true, 0, HIGH_PRIORITY);
                        break;
                    }
                    case ID_GAME_RUN:
                    {
                        /* Do things - game is running on client side */

                        break;
                    }
                    case ID_GAME_END:
                    {
                        /* Do things - game end for client */

                        peer->CloseConnection(packet->systemAddress, true, 0, HIGH_PRIORITY);
                        break;
                    }
                }
            }

            #ifdef ANNOUNCE

            if ((GetTimeMS() - announcetime) > RAKNET_MASTER_RATE)
                Announce(true);

            #endif

            RakSleep(2);
      }

      peer->Shutdown(300);
      RakPeerInterface::DestroyInstance(peer);

      return ((DWORD) data);
}

HANDLE Dedicated::InitalizeServer(AMX* amx)
{
    HANDLE hDedicatedThread;
    DWORD DedicatedID;

    thread = true;
    script = amx;

    hDedicatedThread = CreateThread(NULL, 0, DedicatedThread, (LPVOID) 0, 0, &DedicatedID);

    return hDedicatedThread;
}
