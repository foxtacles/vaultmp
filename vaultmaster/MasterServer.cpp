#include "MasterServer.h"

using namespace RakNet;
using namespace Data;
using namespace std;

RakPeerInterface* MasterServer::peer;
SocketDescriptor* MasterServer::sockdescr;

typedef map<SystemAddress, ServerEntry> ServerMap;
ServerMap MasterServer::serverList;

bool MasterServer::thread;

void MasterServer::TerminateThread()
{
      thread = false;
}

void MasterServer::timestamp()
{
      time_t ltime;
      ltime = time(NULL);
      char t[32];
      sprintf(t, "[%s", asctime(localtime(&ltime)));
      char* newline = strchr(t, '\n');
      *newline = ']';
      strcat(t, " ");
      printf(t);
}

void MasterServer::RemoveServer(SystemAddress addr)
{
    map<SystemAddress, ServerEntry>::iterator i;
    i = serverList.find(addr);

    if (i != serverList.end())
        serverList.erase(i);
}

DWORD WINAPI MasterServer::MasterThread(LPVOID data)
{
      sockdescr = new SocketDescriptor(RAKNET_PORT, 0);
      peer = RakPeerInterface::GetInstance();
      peer->Startup(RAKNET_CONNECTIONS, sockdescr, 1, THREAD_PRIORITY_NORMAL);
      peer->SetMaximumIncomingConnections(RAKNET_CONNECTIONS);
      peer->SetIncomingPassword(MASTER_VERSION, sizeof(MASTER_VERSION));

      Packet* packet;

      while (thread)
      {
            for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
            {
                switch (packet->data[0])
                {
                    case ID_NEW_INCOMING_CONNECTION:
                        timestamp();
                        printf("New incoming connection from %s\n", packet->systemAddress.ToString());
                        break;
                    case ID_DISCONNECTION_NOTIFICATION:
                        timestamp();
                        printf("Client disconnected (%s)\n", packet->systemAddress.ToString());
                        RemoveServer(packet->systemAddress);
                        break;
                    case ID_CONNECTION_LOST:
                        timestamp();
                        printf("Lost connection (%s)\n", packet->systemAddress.ToString());
                        RemoveServer(packet->systemAddress);
                        break;
                    case ID_MASTER_QUERY:
                    {
                        BitStream query;
                        query.Write((MessageID) ID_MASTER_QUERY);
                        query.Write((unsigned int) serverList.size());

                        for (map<SystemAddress, ServerEntry>::const_iterator i = serverList.begin(); i != serverList.end(); ++i)
                        {
                            SystemAddress addr = i->first;
                            ServerEntry entry = i->second;
                            RakString name(entry.GetServerName().c_str()); RakString map(entry.GetServerMap().c_str());
                            int players = entry.GetServerPlayers().first; int playersMax = entry.GetServerPlayers().second;
                            bool NewVegas = entry.IsNewVegas();
                            std::map<string, string> rules = entry.GetServerRules();

                            query.Write(addr);
                            query.Write(name);
                            query.Write(map);
                            query.Write(players);
                            query.Write(playersMax);
                            query.Write(NewVegas);
                            query.Write((int) rules.size());

                            for (std::map<string, string>::const_iterator k = rules.begin(); k != rules.end(); ++k)
                            {
                                RakString key(k->first.c_str());
                                RakString value(k->second.c_str());
                                query.Write(key);
                                query.Write(value);
                            }
                        }

                        peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);

                        timestamp();
                        printf("Query processed (%s)\n", packet->systemAddress.ToString());
                        break;
                    }
                    case ID_MASTER_UPDATE:
                    {
                        BitStream query(packet->data, packet->length, false);
                        query.IgnoreBytes(sizeof(MessageID));

                        SystemAddress addr;
                        query.Read(addr);
                        query.Reset();

                        map<SystemAddress, ServerEntry>::iterator i;
                        i = serverList.find(addr);

                        query.Write((MessageID) ID_MASTER_UPDATE);
                        query.Write(addr);

                        if (i != serverList.end())
                        {
                            ServerEntry entry = i->second;
                            RakString name(entry.GetServerName().c_str()); RakString map(entry.GetServerMap().c_str());
                            int players = entry.GetServerPlayers().first; int playersMax = entry.GetServerPlayers().second;
                            bool NewVegas = entry.IsNewVegas();
                            std::map<string, string> rules = entry.GetServerRules();

                            query.Write(name);
                            query.Write(map);
                            query.Write(players);
                            query.Write(playersMax);
                            query.Write(NewVegas);
                            query.Write((int) rules.size());

                            for (std::map<string, string>::const_iterator k = rules.begin(); k != rules.end(); ++k)
                            {
                                RakString key(k->first.c_str());
                                RakString value(k->second.c_str());
                                query.Write(key);
                                query.Write(value);
                            }
                        }

                        peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);

                        timestamp();
                        printf("Update processed (%s)\n", packet->systemAddress.ToString());
                        break;
                    }
                    case ID_MASTER_ANNOUNCE:
                    {
                        BitStream query(packet->data, packet->length, false);
                        query.IgnoreBytes(sizeof(MessageID));

                        bool announce;
                        query.Read(announce);

                        map<SystemAddress, ServerEntry>::iterator i;
                        i = serverList.find(packet->systemAddress);

                        if (announce)
                        {
                            RakString name, map;
                            int players, playersMax, rsize;
                            bool NewVegas;

                            query.Read(name);
                            query.Read(map);
                            query.Read(players);
                            query.Read(playersMax);
                            query.Read(NewVegas);
                            query.Read(rsize);

                            ServerEntry* entry;

                            if (i == serverList.end())
                            {
                                std::pair<std::map<SystemAddress, ServerEntry>::iterator, bool> k;
                                k = serverList.insert(pair<SystemAddress, ServerEntry>(packet->systemAddress, ServerEntry(name.C_String(), map.C_String(), pair<int, int>(players, playersMax), 999, NewVegas)));
                                entry = &(k.first)->second;
                            }
                            else
                            {
                                entry = &i->second;
                                entry->SetServerName(name.C_String());
                                entry->SetServerMap(map.C_String());
                                entry->SetServerPlayers(pair<int, int>(players, playersMax));
                            }

                            for (int j = 0; j < rsize; j++)
                            {
                                RakString key, value;
                                query.Read(key);
                                query.Read(value);
                                entry->SetServerRule(key.C_String(), value.C_String());
                            }
                        }
                        else
                            if (i != serverList.end())
                                serverList.erase(i);

                        timestamp();
                        printf("Announce processed (%s)\n", packet->systemAddress.ToString());
                        break;
                    }
                }
            }

            RakSleep(2);
      }

      peer->Shutdown(300);
      RakPeerInterface::DestroyInstance(peer);

      return ((DWORD) data);
}

HANDLE MasterServer::InitalizeRakNet()
{
    HANDLE hMasterThread;
    DWORD MasterID;

    thread = true;

    hMasterThread = CreateThread(NULL, 0, MasterThread, (LPVOID) 0, 0, &MasterID);

    return hMasterThread;
}
