#include "Dedicated.h"

using namespace std;
using namespace RakNet;
using namespace Values;

RakPeerInterface* Dedicated::peer;
SocketDescriptor* Dedicated::sockdescr;
unsigned int Dedicated::port;
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

unsigned char Dedicated::GetGameCode()
{
	return self->GetGame();
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
			unsigned char game = self->GetGame();
			const map<string, string>& rules = self->GetServerRules();

			query.Write(name);
			query.Write(_map);
			query.Write(players);
			query.Write(playersMax);
			query.Write(game);
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
		unsigned char game = self->GetGame();
		const map<string, string>& rules = self->GetServerRules();

		query.Write(name);
		query.Write(_map);
		query.Write(players);
		query.Write(playersMax);
		query.Write(game);
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
		files.AddFile(it->first.c_str(), file, 0, len, len, FileListNodeContext(FILE_MODFILE, i), true);
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
	sockdescr = new SocketDescriptor(port, 0);
	peer = RakPeerInterface::GetInstance();
	peer->SetIncomingPassword(DEDICATED_VERSION, sizeof(DEDICATED_VERSION));

	if (announce)
	{
		vector<char> buf(announce, announce + strlen(announce) + 1);
		peer->Startup(connections + 1, sockdescr, 1, THREAD_PRIORITY_NORMAL);
		peer->SetMaximumIncomingConnections(connections);
		master.SetBinaryAddress(strtok(&buf[0], ":"));
		char* cport = strtok(nullptr, ":");
		master.SetPort(cport != nullptr ? atoi(cport) : RAKNET_MASTER_STANDARD_PORT);
		peer->Connect(master.ToString(false), master.GetPort(), MASTER_VERSION, sizeof(MASTER_VERSION), 0, 0, 3, 500, 0);
		announcetime = GetTimeMS();
	}
	else
	{
		peer->Startup(connections, sockdescr, 1, THREAD_PRIORITY_NORMAL);
		peer->SetMaximumIncomingConnections(connections);
	}

#ifdef VAULTMP_DEBUG
	Debug::SetDebugHandler("vaultserver");
	debug.note("Vault-Tec Multiplayer Mod dedicated server debug log (", DEDICATED_VERSION, ")");
	debug.note("Local host: ", peer->GetMyBoundAddress().ToString(), " (", self->GetGame() == FALLOUT3 ? "Fallout 3" : "Fallout New Vegas", ")");
	debug.note("Visit www.vaultmp.com for help and upload this log if you experience problems with the mod.");
	debug.note("-----------------------------------------------------------------------------------------------------");
#endif

	try
	{
		Packet* packet;

		GameFactory::Initialize(self->GetGame());
		API::Initialize(self->GetGame());
		Client::SetMaximumClients(connections);
		Network::Flush();

		Player::SetSpawnCell(cell);

		static_assert(sizeof(chrono::system_clock::rep) == sizeof(Time64_T), "Underlying representation of chrono::system_clock should be 64bit integral");

		Script::gameTime.first = chrono::system_clock::now();
		Script::gameTime.second = 1.0;
		Script::CreateTimer(&Script::Timer_GameTime, 1000);

		Script::gameWeather = DEFAULT_WEATHER;

		auto containers = DB::Reference::Lookup("CONT");

		for (const auto* reference : containers)
		{
			// FIXME dlc support
			if (reference->GetReference() & 0xFF000000)
				continue;

			auto container = GameFactory::GetObject<Container>(GameFactory::CreateInstance(ID_CONTAINER, reference->GetReference(), reference->GetBase())).get();
			const auto& pos = reference->GetPos();
			const auto& angle = reference->GetAngle();
			auto cell = reference->GetCell();
			auto lock = reference->GetLock();
			container->SetNetworkPos(Axis_X, get<0>(pos));
			container->SetNetworkPos(Axis_Y, get<1>(pos));
			container->SetNetworkPos(Axis_Z, get<2>(pos));
			container->SetGamePos(Axis_X, get<0>(pos));
			container->SetGamePos(Axis_Y, get<1>(pos));
			container->SetGamePos(Axis_Z, get<2>(pos));
			container->SetAngle(Axis_X, get<0>(angle));
			container->SetAngle(Axis_Y, get<1>(angle));
			container->SetAngle(Axis_Z, get<2>(angle));
			container->SetNetworkCell(cell);
			container->SetGameCell(cell);
			container->SetLockLevel(lock);
/*
			const auto& items = DB::BaseContainer::Lookup(container->GetBase());

			for (const auto* item : items)
			{
				if (item->GetItem() & 0xFF000000)
					continue;

				auto diff = container->AddItem(item->GetItem(), item->GetCount(), item->GetCondition(), true);
				container->ApplyDiff(diff);
			}
*/
		}

		Utils::timestamp();
		printf("Dedicated server initialized, running scripts now\n");

		Script::Run();
		Script::OnServerInit();

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
			Script::OnServerExit();
			throw;
		}

		Script::OnServerExit();
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

std::thread Dedicated::InitializeServer(unsigned int port, unsigned int connections, const char* announce, bool query, bool fileserve, unsigned int fileslots)
{
	std::thread hDedicatedThread;

	thread = true;
	Dedicated::port = port ? port : RAKNET_STANDARD_PORT;
	Dedicated::connections = connections ? connections : RAKNET_STANDARD_CONNECTIONS;
	Dedicated::announce = strlen(announce) ? announce : nullptr;
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
