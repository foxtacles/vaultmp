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
    ID_GAME_START,
    ID_GAME_END,
    ID_NEW_PLAYER,
    ID_PLAYER_LEFT,
    ID_POS_UPDATE
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
                        Player* player = Player::GetPlayerFromGUID(packet->guid);

                        if (player != NULL)
                        {
                            delete player;

                            map<RakNetGUID, string> players = Player::GetPlayerList();
                            map<RakNetGUID, string>::iterator it;

                            BitStream query(packet->data, packet->length, false);
                            query.IgnoreBytes(sizeof(MessageID));
                            query.Reset();

                            query.Write((MessageID) ID_PLAYER_LEFT);
                            query.Write(packet->guid);

                            for (it = players.begin(); it != players.end(); it++)
                            {
                                RakNetGUID guid = it->first;
                                peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, guid, false, 0);
                            }
                        }

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
                        BitStream query;

                        char savegame[128];
                        strcpy(savegame, "default");

                        int ret = 1;

                        if (amx != NULL)
                        {
                            void* args[3];

                            int id = Client::GetClientFromGUID(packet->guid)->GetClientID();
                            int len = sizeof(savegame);

                            args[0] = reinterpret_cast<void*>(&len);
                            args[1] = reinterpret_cast<void*>(savegame);
                            args[2] = reinterpret_cast<void*>(&id);

                            ret = Script::Call(amx, (char*) "OnClientRequestGame", (char*) "isi", args, len);
                        }

                        if (ret)
                        {
                            query.Write((MessageID) ID_GAME_RUN);

                            RakString save(savegame);
                            query.Write(save);

                            peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);
                        }
                        else
                            peer->CloseConnection(packet->systemAddress, true, 0, HIGH_PRIORITY);
                        break;
                    }
                    case ID_GAME_START:
                    {
                        map<RakNetGUID, string> players = Player::GetPlayerList();
                        map<RakNetGUID, string>::iterator it;

                        Client* client = Client::GetClientFromGUID(packet->guid);
                        string name = client->GetAuthName();
                        RakString pname(name.c_str());

                        BitStream query(packet->data, packet->length, false);
                        query.IgnoreBytes(sizeof(MessageID));
                        query.Reset();

                        query.Write((MessageID) ID_NEW_PLAYER);
                        query.Write(packet->guid);
                        query.Write(pname);

                        for (it = players.begin(); it != players.end(); it++)
                        {
                            RakNetGUID guid = it->first;
                            peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, guid, false, 0);
                        }

                        query.Reset();

                        for (it = players.begin(); it != players.end(); it++)
                        {
                            RakNetGUID guid = it->first;
                            Player* player = Player::GetPlayerFromGUID(guid);
                            string name = player->GetPlayerName();
                            RakString pname(name.c_str());

                            query.Write((MessageID) ID_NEW_PLAYER);
                            query.Write(guid);
                            query.Write(pname);
                            peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->guid, false, 0);
                            query.Reset();
                        }

                        Player* player = new Player(packet->guid);
                        player->SetPlayerName(name);
                        break;
                    }
                    case ID_POS_UPDATE:
                    {
                        BitStream query(packet->data, packet->length, false);
                        query.IgnoreBytes(sizeof(MessageID));

                        float X, Y, Z, A, health, baseHealth, cond0, cond1, cond2, cond3, cond4, cond5;
                        bool dead;
                        int moving;
                        query.Read(X);
                        query.Read(Y);
                        query.Read(Z);
                        query.Read(A);
                        query.Read(health);
                        query.Read(baseHealth);
                        query.Read(cond0);
                        query.Read(cond1);
                        query.Read(cond2);
                        query.Read(cond3);
                        query.Read(cond4);
                        query.Read(cond5);
                        query.Read(dead);
                        query.Read(moving);
                        query.Reset();

                        query.Write((MessageID) ID_POS_UPDATE);
                        query.Write(packet->guid);
                        query.Write(X);
                        query.Write(Y);
                        query.Write(Z);
                        query.Write(A);
                        query.Write(health);
                        query.Write(baseHealth);
                        query.Write(cond0);
                        query.Write(cond1);
                        query.Write(cond2);
                        query.Write(cond3);
                        query.Write(cond4);
                        query.Write(cond5);
                        query.Write(dead);
                        query.Write(moving);

                        map<RakNetGUID, string> players = Player::GetPlayerList();
                        map<RakNetGUID, string>::iterator it;

                        for (it = players.begin(); it != players.end(); it++)
                        {
                            RakNetGUID guid = it->first;
                            if (guid != packet->guid) peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, guid, false, 0);
                        }

                        Player* player = Player::GetPlayerFromGUID(packet->guid);
                        player->SetPlayerPos(0, X);
                        player->SetPlayerPos(1, Y);
                        player->SetPlayerPos(2, Z);
                        player->SetPlayerAngle(A);
                        player->SetPlayerHealth(health);
                        player->SetPlayerBaseHealth(baseHealth);
                        player->SetPlayerCondition(0, cond0);
                        player->SetPlayerCondition(1, cond1);
                        player->SetPlayerCondition(2, cond2);
                        player->SetPlayerCondition(3, cond3);
                        player->SetPlayerCondition(4, cond4);
                        player->SetPlayerCondition(5, cond5);
                        player->SetPlayerDead(dead);
                        player->SetPlayerMoving(moving);
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
