#include "Dedicated.h"
#include "API.h"
#include "ServerEntry.h"
#include "Data.h"
#include "Utils.h"
#include "GameFactory.h"
#include "Client.h"
#include "Network.h"
#include "NetworkServer.h"
#include "Timer.h"
#include "Script.h"

using namespace std;
using namespace RakNet;
using namespace Values;

RakPeerInterface* Dedicated::peer;
unsigned int Dedicated::port;
const char* Dedicated::host;
unsigned int Dedicated::fileslots;
unsigned int Dedicated::connections;
const char* Dedicated::announce;
bool Dedicated::query;
bool Dedicated::fileserve;
SystemAddress Dedicated::master;
TimeMS Dedicated::announcetime;
ServerEntry* Dedicated::self = nullptr;
unsigned int Dedicated::cell;
ModList Dedicated::modfiles;

#ifdef VAULTMP_DEBUG
DebugInput<Dedicated> Dedicated::debug;
#endif

bool Dedicated::thread;

void Dedicated::TerminateThread()
{
	thread = false;
}

void Dedicated::SetServerName(const char* name)
{
	self->SetServerName(name);
}

void Dedicated::SetServerMap(const char* map)
{
	self->SetServerMap(map);
}

void Dedicated::SetServerRule(const char* rule, const char* value)
{
	self->SetServerRule(rule, value);
}

unsigned int Dedicated::GetCurrentPlayers()
{
	return self->GetServerPlayers().first;
}

unsigned int Dedicated::GetMaximumPlayers()
{
	return self->GetServerPlayers().second;
}

void Dedicated::Announce(bool announce)
{
	if (peer->GetConnectionState(master) == IS_CONNECTED)
	{
		BitStream query;

		if (announce)
		{
			query.Write((MessageID) ID_MASTER_ANNOUNCE);
			query.Write(true);

			RakString name(self->GetServerName().c_str());
			RakString _map(self->GetServerMap().c_str());
			unsigned int players = self->GetServerPlayers().first;
			unsigned int playersMax = self->GetServerPlayers().second;
			const map<string, string>& rules = self->GetServerRules();

			query.Write(name);
			query.Write(_map);
			query.Write(players);
			query.Write(playersMax);
			query.Write(rules.size());

			for (const auto& i : rules)
			{
				RakString key(i.first.c_str());
				RakString value(i.second.c_str());
				query.Write(key);
				query.Write(value);
			}

			query.Write(modfiles.size());

			for (const auto& j : modfiles)
			{
			    RakString name(j.first.c_str());
			    query.Write(name);
			}
		}
		else
		{
			query.Write((MessageID) ID_MASTER_ANNOUNCE);
			query.Write(false);
		}

		peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, master, false, 0);
	}
	else
	{
		Utils::timestamp();
		printf("Lost connection to MasterServer (%s)\n", master.ToString());
		peer->Connect(master.ToString(false), master.GetPort(), MASTER_VERSION, sizeof(MASTER_VERSION), 0, 0, 3, 100, 0);
	}

	announcetime = GetTimeMS();
}

void Dedicated::Query(Packet* packet)
{
	if (Dedicated::query)
	{
		BitStream queryy(packet->data, packet->length, false);
		queryy.IgnoreBytes(sizeof(MessageID));

		SystemAddress addr;
		queryy.Read(addr);

		BitStream query;

		query.Write((MessageID) ID_MASTER_UPDATE);
		query.Write(addr);

		RakString name(self->GetServerName().c_str());
		RakString _map(self->GetServerMap().c_str());
		unsigned int players = self->GetServerPlayers().first;
		unsigned int playersMax = self->GetServerPlayers().second;
		const map<string, string>& rules = self->GetServerRules();

		query.Write(name);
		query.Write(_map);
		query.Write(players);
		query.Write(playersMax);
		query.Write(rules.size());

		for (const auto& i : rules)
		{
			RakString key(i.first.c_str());
			RakString value(i.second.c_str());
			query.Write(key);
			query.Write(value);
		}

		query.Write(modfiles.size());

		for (const auto& k : modfiles)
        {
            RakString mod_name(k.first.c_str());
            query.Write(mod_name);
        }

		peer->Send(&query, LOW_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, packet->systemAddress, false, 0);

		Utils::timestamp();
		printf("Query processed (%s)\n", packet->systemAddress.ToString());
	}
	else
	{
		Utils::timestamp();
		printf("Query is disabled (%s)\n", packet->systemAddress.ToString());
		peer->CloseConnection(packet->systemAddress, true, 0, LOW_PRIORITY);
	}
}

class FileProgress : public FileListProgress
{
		virtual void OnFilePush(const char* fileName, unsigned int, unsigned int, unsigned int bytesBeingSent, bool, SystemAddress targetSystem)
		{
			Utils::timestamp();
			printf("Sending %s (%d bytes) to %s\n", fileName, bytesBeingSent, targetSystem.ToString(false));
		}

		virtual void OnFilePushesComplete(SystemAddress systemAddress)
		{
			Utils::timestamp();
			printf("Transfer complete (%s)\n", systemAddress.ToString(false));
		}

		virtual void OnSendAborted(SystemAddress systemAddress)
		{
			Utils::timestamp();
			printf("Transfer aborted (%s)\n", systemAddress.ToString(false));
		}

} fileProgress;

void Dedicated::FileThread()
{
	PacketizedTCP tcp;
	FileList files;
	FileListTransfer flt;
	IncrementalReadInterface incInterface;
	tcp.Start(Dedicated::port, Dedicated::fileslots);
	tcp.AttachPlugin(&flt);
	flt.AddCallback(&fileProgress);
	flt.StartIncrementalReadThreads(1);

	char dir[MAX_PATH];
	char file[MAX_PATH];
	getcwd(dir, sizeof(dir));

	ModList::iterator it;
	unsigned int len;
	unsigned int i = 1;

	for (it = modfiles.begin(); it != modfiles.end(); ++it, i++)
	{
		snprintf(file, sizeof(file), "%s/%s/%s", dir, MODFILES_PATH, it->first.c_str());
		len = Utils::FileLength(file);
		files.AddFile(it->first.c_str(), file, 0, len, len, FileListNodeContext(FILE_MODFILE, i, 0, 0), true);
	}

	Packet* packet;
	char rdy = RAKNET_FILE_RDY;

	while (thread)
	{
		packet = tcp.Receive();
		SystemAddress addr = tcp.HasNewIncomingConnection();

		if (addr != UNASSIGNED_SYSTEM_ADDRESS)
			tcp.Send(&rdy, sizeof(char), addr, false);

		if (packet && packet->data[0] == RAKNET_FILE_RDY && packet->length == 3)
			flt.Send(&files, 0, packet->systemAddress, *((unsigned short*)(packet->data + 1)), MEDIUM_PRIORITY, CHANNEL_SYSTEM, &incInterface, 2000000);

		tcp.DeallocatePacket(packet);
		this_thread::sleep_for(chrono::milliseconds(5));
	}
}

void Dedicated::DedicatedThread()
{
	auto sockdescr = SocketDescriptor(port, host);
	peer = RakPeerInterface::GetInstance();
	peer->SetIncomingPassword(DEDICATED_VERSION, sizeof(DEDICATED_VERSION));

	if (announce)
	{
		vector<char> buf(announce, announce + strlen(announce) + 1);
		peer->Startup(connections + 1, &sockdescr, 1, THREAD_PRIORITY_NORMAL);
		peer->SetMaximumIncomingConnections(connections);
		master.SetBinaryAddress(strtok(&buf[0], ":"));
		char* cport = strtok(nullptr, ":");
		master.SetPortHostOrder(cport != nullptr ? atoi(cport) : RAKNET_MASTER_STANDARD_PORT);
		peer->Connect(master.ToString(false), master.GetPort(), MASTER_VERSION, sizeof(MASTER_VERSION), 0, 0, 3, 500, 0);
		announcetime = GetTimeMS();
	}
	else
	{
		peer->Startup(connections, &sockdescr, 1, THREAD_PRIORITY_NORMAL);
		peer->SetMaximumIncomingConnections(connections);
	}

#ifdef VAULTMP_DEBUG
	Debug::SetDebugHandler("vaultserver");
	debug.note("Vault-Tec Multiplayer Mod dedicated server debug log (", DEDICATED_VERSION, ")");
	debug.note("Local host: ", peer->GetMyBoundAddress().ToString());
	debug.note("Visit www.vaultmp.com for help and upload this log if you experience problems with the mod.");
	debug.note("-----------------------------------------------------------------------------------------------------");
#endif

	try
	{
		Packet* packet;

		GameFactory::Initialize();
		API::Initialize();
		Client::SetMaximumClients(connections);
		Network::Flush();

		Player::SetSpawnCell(cell);

		Script::Initialize();

		Utils::timestamp();
		printf("Dedicated server initialized, running scripts now\n");

		Script::Call<Script::CBI("OnServerInit")>();

		try
		{
			while (thread)
			{
				while (Network::Dispatch(peer));

				for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
				{
					if (packet->data[0] == ID_MASTER_UPDATE)
						Query(packet);
					else
					{
						try
						{
							NetworkResponse response = NetworkServer::ProcessPacket(packet);

							vector<RakNetGUID> closures;

							for (const SingleResponse& _response : response)
								if (static_cast<pTypes>(*_response.get_packet()->get()) == pTypes::ID_GAME_END)
									closures.insert(closures.end(), _response.get_targets().begin(), _response.get_targets().end());

							Network::Dispatch(peer, move(response));

							while (Network::Dispatch(peer));

							for (RakNetGUID& guid : closures)
								peer->CloseConnection(guid, true, CHANNEL_SYSTEM, HIGH_PRIORITY);
						}
						catch (...)
						{
							peer->DeallocatePacket(packet);
							Network::Dispatch(peer, NetworkServer::ProcessEvent(ID_EVENT_SERVER_ERROR));
							throw;
						}
					}
				}

				Timer::GlobalTick();

				this_thread::sleep_for(chrono::milliseconds(1));

				if (announce)
				{
					if ((GetTimeMS() - announcetime) > RAKNET_MASTER_RATE)
						Announce(true);
				}
			}
		}
		catch (...)
		{
			Script::Call<Script::CBI("OnServerExit")>();
			throw;
		}

		Script::Call<Script::CBI("OnServerExit")>();
	}
	catch (exception& e)
	{
		try
		{
			VaultException& vaulterror = dynamic_cast<VaultException&>(e);
			vaulterror.Console();
		}
		catch (bad_cast&)
		{
			VaultException vaulterror(e.what());
			vaulterror.Console();
		}
	}

	Script::UnloadScripts();

	thread = false;

	peer->Shutdown(300);
	RakPeerInterface::DestroyInstance(peer);

	GameFactory::DestroyAllInstances();
	API::Terminate();

#ifdef VAULTMP_DEBUG
	debug.print("Network thread is going to terminate");
	Debug::SetDebugHandler(nullptr);
#endif
}

std::thread Dedicated::InitializeServer(unsigned int port, const char* host, unsigned int connections, const char* announce, bool query, bool fileserve, unsigned int fileslots)
{
	std::thread hDedicatedThread;

	thread = true;
	Dedicated::port = port ? port : RAKNET_STANDARD_PORT;
	Dedicated::host = host;
	Dedicated::connections = connections ? connections : RAKNET_STANDARD_CONNECTIONS;
	Dedicated::announce = (announce && *announce) ? announce : nullptr;
	Dedicated::query = query;
	Dedicated::fileserve = fileserve;
	Dedicated::fileslots = fileslots;
	Dedicated::self->SetServerPlayers(make_pair(0u, Dedicated::connections));

	hDedicatedThread = std::thread(DedicatedThread);

	if (fileserve)
		std::thread(FileThread).detach();

	return hDedicatedThread;
}

void Dedicated::SetServerEntry(ServerEntry* self)
{
	Dedicated::self = self;
}

void Dedicated::SetSpawnCell(unsigned int cell)
{
	Dedicated::cell = cell;
}

void Dedicated::SetModfiles(ModList modfiles)
{
	Dedicated::modfiles = modfiles;
}

/* void Dedicated::SetServerConnections(int connections)
{

} */
