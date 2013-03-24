#include "MasterServer.h"

using namespace RakNet;
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

void MasterServer::RemoveServer(SystemAddress addr)
{
	map<SystemAddress, ServerEntry>::iterator i;
	i = serverList.find(addr);

	if (i != serverList.end())
		serverList.erase(i);
}

void MasterServer::MasterThread()
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
					Utils::timestamp();
					printf("New incoming connection from %s\n", packet->systemAddress.ToString());
					break;

				case ID_DISCONNECTION_NOTIFICATION:
					Utils::timestamp();
					printf("Client disconnected (%s)\n", packet->systemAddress.ToString());
					RemoveServer(packet->systemAddress);
					break;

				case ID_CONNECTION_LOST:
					Utils::timestamp();
					printf("Lost connection (%s)\n", packet->systemAddress.ToString());
					RemoveServer(packet->systemAddress);
					break;

				case ID_MASTER_QUERY:
				{
					BitStream query;
					query.Write((MessageID) ID_MASTER_QUERY);
					query.Write((unsigned int) serverList.size());

					for (map<SystemAddress, ServerEntry>::iterator i = serverList.begin(); i != serverList.end(); ++i)
					{
						SystemAddress addr = i->first;
						ServerEntry entry = i->second;
						RakString name(entry.GetServerName().c_str());
						RakString map(entry.GetServerMap().c_str());
						unsigned int players = entry.GetServerPlayers().first;
						unsigned int playersMax = entry.GetServerPlayers().second;
						const std::map<string, string>& rules = entry.GetServerRules();
						const std::vector<string>& modfiles = entry.GetServerModFiles();

						query.Write(addr);
						query.Write(name);
						query.Write(map);
						query.Write(players);
						query.Write(playersMax);
						query.Write(rules.size());

						for (const auto& k : rules)
						{
							RakString key(k.first.c_str());
							RakString value(k.second.c_str());
							query.Write(key);
							query.Write(value);
						}

						query.Write(modfiles.size());

						for (const auto& k : modfiles)
						{
						    RakString mod_name(k.c_str());
						    query.Write(mod_name);
						}
					}

					peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);

					Utils::timestamp();
					printf("Query processed (%s)\n", packet->systemAddress.ToString());
					break;
				}

				case ID_MASTER_UPDATE:
				{
					BitStream queryy(packet->data, packet->length, false);
					queryy.IgnoreBytes(sizeof(MessageID));

					SystemAddress addr;
					queryy.Read(addr);

					BitStream query;

					map<SystemAddress, ServerEntry>::iterator i;
					i = serverList.find(addr);

					query.Write((MessageID) ID_MASTER_UPDATE);
					query.Write(addr);

					if (i != serverList.end())
					{
						ServerEntry entry = i->second;
						RakString name(entry.GetServerName().c_str());
						RakString map(entry.GetServerMap().c_str());
						unsigned int players = entry.GetServerPlayers().first;
						unsigned int playersMax = entry.GetServerPlayers().second;
						const std::map<string, string>& rules = entry.GetServerRules();
						const std::vector<string>& modfiles = entry.GetServerModFiles();

						query.Write(name);
						query.Write(map);
						query.Write(players);
						query.Write(playersMax);
						query.Write(rules.size());

						for (const auto& k : rules)
						{
							RakString key(k.first.c_str());
							RakString value(k.second.c_str());
							query.Write(key);
							query.Write(value);
						}

						query.Write(modfiles.size());

						for (const auto& k : modfiles)
						{
						    RakString mod_name(k.c_str());
						    query.Write(mod_name);
						}
					}

					peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);

					Utils::timestamp();
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
						unsigned int players, playersMax, rsize, msize;

						query.Read(name);
						query.Read(map);
						query.Read(players);
						query.Read(playersMax);
						query.Read(rsize);

						ServerEntry* entry;

						if (i == serverList.end())
						{
							std::pair<std::map<SystemAddress, ServerEntry>::iterator, bool> k;
							k = serverList.insert(make_pair(packet->systemAddress, ServerEntry(name.C_String(), map.C_String(), make_pair(players, playersMax), 999)));
							entry = &(k.first)->second;
						}
						else
						{
							entry = &i->second;
							entry->SetServerName(name.C_String());
							entry->SetServerMap(map.C_String());
							entry->SetServerPlayers(make_pair(players, playersMax));
						}

						for (unsigned int j = 0; j < rsize; j++)
						{
							RakString key, value;
							query.Read(key);
							query.Read(value);
							entry->SetServerRule(key.C_String(), value.C_String());
						}

						entry->ClearModFiles();
						query.Read(msize);

                        for (unsigned int j = 0; j < msize; j++)
                        {
                            RakString mod_name;
                            query.Read(mod_name);
                            entry->SetModFiles(mod_name.C_String());
                        }
					}
					else if (i != serverList.end())
						serverList.erase(i);

					Utils::timestamp();
					printf("Announce processed (%s)\n", packet->systemAddress.ToString());
					break;
				}
			}
		}

		this_thread::sleep_for(chrono::milliseconds(1));
	}

	peer->Shutdown(300);
	RakPeerInterface::DestroyInstance(peer);
}

std::thread MasterServer::InitalizeRakNet()
{
	thread = true;

	return std::thread(MasterThread);
}
