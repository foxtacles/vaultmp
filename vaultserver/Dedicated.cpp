#include "Dedicated.h"

using namespace RakNet;
using namespace std;

RakPeerInterface* Dedicated::peer;
SocketDescriptor* Dedicated::sockdescr;
int Dedicated::port;
int Dedicated::connections;
AMX* Dedicated::amx;
char* Dedicated::announce;
bool Dedicated::query;

SystemAddress Dedicated::master;
TimeMS Dedicated::announcetime;

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
        peer->Connect(master.ToString(false), master.port, 0, 0, 0, 0, 3, 100, 0);
    }

    announcetime = GetTimeMS();
}

DWORD WINAPI Dedicated::DedicatedThread(LPVOID data)
{
      sockdescr = new SocketDescriptor(port, 0);
      peer = RakPeerInterface::GetInstance();

      if (announce)
      {
            peer->Startup(connections + 1, sockdescr, 1, THREAD_PRIORITY_NORMAL);
            peer->SetMaximumIncomingConnections(connections);
            master.SetBinaryAddress(strtok(announce, ":"));
            char* cport = strtok(NULL, ":");
            master.port = cport != NULL ? atoi(cport) : RAKNET_MASTER_STANDARD_PORT;
            peer->Connect(master.ToString(false), master.port, 0, 0, 0, 0, 3, 500, 0);
            announcetime = GetTimeMS();
      }
      else
      {
            peer->Startup(connections, sockdescr, 1, THREAD_PRIORITY_NORMAL);
            peer->SetMaximumIncomingConnections(connections);
      }

      Packet* packet;

      self.SetServerName("testserver");
      self.SetServerMap("asdf");
      self.SetServerPlayers(pair<int, int>(0, connections));
      self.SetServerPing(123);
      self.SetServerRule("asdf", "kokolores");
      self.SetServerRule("blub", "moep");

      Client::SetMaximumClients(connections);

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
                    {
                        Utils::timestamp();
                        printf("Client disconnected (%s)\n", packet->systemAddress.ToString());

                        Client* client = Client::GetClientFromGUID(packet->guid);
                        if (client != NULL)
                        {
                            delete client;
                            self.SetServerPlayers(pair<int, int>(Client::GetClientCount(), connections));
                        }
                        break;
                    }
                    case ID_CONNECTION_LOST:
                    {
                        Utils::timestamp();
                        printf("Lost connection (%s)\n", packet->systemAddress.ToString());

                        Client* client = Client::GetClientFromGUID(packet->guid);
                        if (client != NULL)
                        {
                            delete client;
                            self.SetServerPlayers(pair<int, int>(Client::GetClientCount(), connections));
                        }
                        break;
                    }
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
                        if (query)
                        {
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
                        }
                        else
                        {
                            Utils::timestamp();
                            printf("Query is disabled (%s)\n", packet->systemAddress.ToString());
                            peer->CloseConnection(packet->systemAddress, true, 0, LOW_PRIORITY);
                            break;
                        }
                    }
                    case ID_GAME_INIT:
                    {
                        BitStream query(packet->data, packet->length, false);
                        query.IgnoreBytes(sizeof(MessageID));

                        RakString rname, rpwd;
                        query.Read(rname);
                        query.Read(rpwd);
                        query.Reset();

                        char name[rname.GetLength()];
                        char pwd[rpwd.GetLength()];
                        strcpy(name, rname.C_String());
                        strcpy(pwd, rpwd.C_String());

                        Client* client = new Client(packet->guid, string(name), string(pwd));
                        self.SetServerPlayers(pair<int, int>(Client::GetClientCount(), connections));

                        int ret = 1;

                        if (amx != NULL)
                        {
                            void* args[3];

                            int id = client->GetClientID();

                            args[0] = reinterpret_cast<void*>(pwd);
                            args[1] = reinterpret_cast<void*>(name);
                            args[2] = reinterpret_cast<void*>(&id);

                            ret = Script::Call(amx, (char*) "OnClientAuthenticate", (char*) "ssi", args);
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

            if (announce)
            {
                if ((GetTimeMS() - announcetime) > RAKNET_MASTER_RATE)
                    Announce(true);
            }

            RakSleep(2);
      }

      peer->Shutdown(300);
      RakPeerInterface::DestroyInstance(peer);

      return ((DWORD) data);
}

HANDLE Dedicated::InitalizeServer(int port, int connections, AMX* amx, char* announce, bool query)
{
    HANDLE hDedicatedThread;
    DWORD DedicatedID;

    thread = true;
    Dedicated::port = port ? port : RAKNET_STANDARD_PORT;
    Dedicated::connections = connections ? connections : RAKNET_STANDARD_CONNECTIONS;
    Dedicated::amx = amx;
    Dedicated::announce = announce;
    Dedicated::query = query;

    hDedicatedThread = CreateThread(NULL, 0, DedicatedThread, (LPVOID) 0, 0, &DedicatedID);

    return hDedicatedThread;
}

/* void Dedicated::SetServerConnections(int connections)
{

} */
