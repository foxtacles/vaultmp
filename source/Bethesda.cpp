#include "Bethesda.h"

using namespace std;
using namespace RakNet;

typedef HINSTANCE(__stdcall* fLoadLibrary)(char*);

unsigned char Bethesda::game = 0x00;
bool Bethesda::initialized = false;
string Bethesda::password = "";
bool Bethesda::multiinst = false;
bool Bethesda::steam = false;
DWORD Bethesda::process = 0;
ModList Bethesda::modfiles;
char Bethesda::module[32];

#ifdef VAULTMP_DEBUG
Debug* Bethesda::debug;
#endif

DWORD Bethesda::lookupProgramID(const char process[])
{
	HANDLE hSnapshot;
	PROCESSENTRY32 ProcessEntry;
	ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (Process32First(hSnapshot, &ProcessEntry))
		do
		{
			if (!strcmp(ProcessEntry.szExeFile, process))
			{
				CloseHandle(hSnapshot);
				return ProcessEntry.th32ProcessID;
			}
		}
		while (Process32Next(hSnapshot, &ProcessEntry));

	CloseHandle(hSnapshot);

	return 0;
}

void Bethesda::Initialize()
{
	switch (Bethesda::game = game)
	{
		case FALLOUT3:
			strcpy(module, "Fallout3.exe");
			break;

		case NEWVEGAS:
			SetEnvironmentVariable("SteamAppID", "22380");
			strcpy(module, "FalloutNV.exe");
			break;

		default:
			throw VaultException("Bad game ID %08X", Bethesda::game);
	}

	TCHAR curdir[MAX_PATH+1];
	unsigned int crc;
	ZeroMemory(curdir, sizeof(curdir));
	GetModuleFileName(GetModuleHandle(nullptr), (LPTSTR) curdir, MAX_PATH);
	PathRemoveFileSpec(curdir);

	strcat(curdir, "\\Data\\");

	for (ModList::iterator it = modfiles.begin(); it != modfiles.end(); ++it)
	{
		TCHAR modfile[MAX_PATH+1];
		ZeroMemory(modfile, sizeof(modfile));
		strcat(modfile, curdir);
		strcat(modfile, it->first.c_str());

		if (!Utils::crc32file(modfile, &crc))
			throw VaultException("Could not find modification file:\n\n%s\n\nAsk the server owner to send you the file or try to Synchronize with the server", modfile);

		if (crc != it->second)
			throw VaultException("Modfile differs from the server version:\n\n%s\n\nAsk the server owner to send you the file or try to Synchronize with the server", modfile);
	}

	TCHAR pluginsdir[MAX_PATH+1];
	ZeroMemory(pluginsdir, sizeof(pluginsdir));
	SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, pluginsdir);   // SHGFP_TYPE_CURRENT

	switch (Bethesda::game)
	{
		case FALLOUT3:
			strcat(pluginsdir, "\\Fallout3\\plugins.vmp");
			break;

		case NEWVEGAS:
			strcat(pluginsdir, "\\FalloutNV\\plugins.vmp");
			break;
	}

	FILE* plugins = fopen(pluginsdir, "w");

	switch (Bethesda::game)
	{
		case FALLOUT3:
		{
			char esm[] = "Fallout3.esm\n";
			fwrite(esm, sizeof(char), strlen(esm), plugins);
			break;
		}

		case NEWVEGAS:
		{
			char esm[] = "FalloutNV.esm\n";
			fwrite(esm, sizeof(char), strlen(esm), plugins);
			break;
		}
	}

	for (ModList::iterator it = modfiles.begin(); it != modfiles.end(); ++it)
	{
		fwrite(it->first.c_str(), sizeof(char), it->first.length(), plugins);
		fwrite("\n", sizeof(char), 1, plugins);
	}

	fclose(plugins);

	if (Bethesda::multiinst || lookupProgramID(module) == 0)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		si.cb = sizeof(si);

		if (CreateProcess(module, nullptr, nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi))
		{
			HANDLE hRemoteThread;
			HINSTANCE hDll;
			fLoadLibrary pLoadLibrary;
			LPVOID remote;

			Bethesda::process = pi.dwProcessId;

			CloseHandle(si.hStdInput);
			CloseHandle(si.hStdOutput);
			CloseHandle(si.hStdError);

			GetModuleFileName(GetModuleHandle(nullptr), (LPTSTR) curdir, MAX_PATH);
			PathRemoveFileSpec(curdir);
			strcat(curdir, "\\vaultmp.dll");
			unsigned int size = strlen(curdir) + 1;

			hDll = LoadLibrary("kernel32.dll");
			pLoadLibrary = (fLoadLibrary) GetProcAddress(hDll, "LoadLibraryA");     // TODO: GetRemoteProcAddress

			if ((remote = VirtualAllocEx(pi.hProcess, 0, size, MEM_COMMIT, PAGE_READWRITE)) == nullptr)
			{
				VirtualFreeEx(pi.hProcess, remote, size, MEM_RELEASE);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
				throw VaultException("Couldn't allocate memory in remote process");
			}

			if (WriteProcessMemory(pi.hProcess, remote, curdir, size, nullptr) == false)
			{
				VirtualFreeEx(pi.hProcess, remote, size, MEM_RELEASE);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
				throw VaultException("Couldn't write memory in remote process");
			}

			if ((hRemoteThread = CreateRemoteThread(pi.hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE) pLoadLibrary, remote, 0, 0)) == nullptr)
			{
				VirtualFreeEx(pi.hProcess, remote, size, MEM_RELEASE);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
				throw VaultException("Couldn't create remote thread");
			}

			if (WaitForSingleObject(hRemoteThread, 5000) != WAIT_OBJECT_0)
			{
				VirtualFreeEx(pi.hProcess, remote, size, MEM_RELEASE);
				throw VaultException("Remote thread timed out");
			}

			VirtualFreeEx(pi.hProcess, remote, size, MEM_RELEASE);

			try
			{
				Interface::Initialize(&Game::CommandHandler, Bethesda::steam);

				chrono::steady_clock::time_point till = chrono::steady_clock::now() + chrono::milliseconds(5000);

				while (chrono::steady_clock::now() < till && !Interface::IsAvailable())
					this_thread::sleep_for(chrono::milliseconds(100));

				if (!Interface::IsAvailable())
					throw VaultException("Failed connecting to vaultmp interface");
			}

			catch (...)
			{
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
				throw;
			}

			ResumeThread(pi.hThread);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);

			// proper game ready signal here
			this_thread::sleep_for(chrono::seconds(6));

			initialized = true;
		}
		else
			throw VaultException("Failed creating the game process");
	}
	else
		throw VaultException("Either Fallout 3 or Fallout: New Vegas is already runnning");
}

void Bethesda::Terminate(RakPeerInterface* peer)
{
#ifdef VAULTMP_DEBUG
	if (debug)
		debug->Print("Terminate called...", true);
#endif

	this_thread::sleep_for(chrono::milliseconds(200));
	Packet* packet = nullptr;

	while ((packet = peer->Receive()))
		peer->DeallocatePacket(packet); // disconnection notification might still arrive

	Interface::Terminate();
	GameFactory::DestroyAllInstances();
	API::Terminate();

	if (initialized)
	{
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, Bethesda::process);

		if (hProcess)
		{
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
		}
	}
}

void Bethesda::InitializeVaultMP(RakPeerInterface* peer, SystemAddress server, string name, string pwd, unsigned char game, bool multiinst, bool steam)
{
	Bethesda::game = game;
	Bethesda::password = pwd;
	Bethesda::multiinst = multiinst;
	Bethesda::steam = steam;
	Bethesda::modfiles.clear();
	ZeroMemory(module, sizeof(module));
	Game::game = game;
	initialized = false;

#ifdef VAULTMP_DEBUG
	debug = new Debug((char*) "vaultmp");
	debug->PrintFormat("Vault-Tec Multiplayer Mod client debug log (%s)", false, CLIENT_VERSION);
	debug->PrintFormat("Connecting to server: %s (name: %s, password: %s, game: %s)", false, server.ToString(), name.c_str(), pwd.c_str(), game == FALLOUT3 ? (char*) "Fallout 3" : (char*) "Fallout New Vegas");
	debug->Print("Visit www.vaultmp.com for help and upload this log if you experience problems with the mod.", false);
	debug->Print("-----------------------------------------------------------------------------------------------------", false);
	//debug->PrintSystem();
	API::SetDebugHandler(debug);
	VaultException::SetDebugHandler(debug);
	NetworkClient::SetDebugHandler(debug);
	Interface::SetDebugHandler(debug);
	Lockable::SetDebugHandler(debug);
	Reference::SetDebugHandler(debug);
	Object::SetDebugHandler(debug);
	Item::SetDebugHandler(debug);
	Container::SetDebugHandler(debug);
	Actor::SetDebugHandler(debug);
	Player::SetDebugHandler(debug);
	Game::SetDebugHandler(debug);
	GameFactory::SetDebugHandler(debug);
	//Network::SetDebugHandler(debug);
#endif

	GameFactory::Initialize(game);
	API::Initialize(game);

	NetworkID id = GameFactory::CreateInstance(ID_PLAYER, PLAYER_REFERENCE, PLAYER_BASE);
	FactoryObject reference = GameFactory::GetObject(id);
	Player* self = vaultcast<Player>(reference);
	self->SetEnabled(true);
	self->SetName(name);
	GameFactory::LeaveReference(reference);

	Network::Flush();
	Network::ToggleDequeue(true);

	try
	{
		if (peer->Connect(server.ToString(false), server.GetPort(), DEDICATED_VERSION, sizeof(DEDICATED_VERSION), 0, 0, 3, 500, 0) == CONNECTION_ATTEMPT_STARTED)
		{
			bool query = true;
			Packet* packet;

			while (query)
			{
				while (Network::Dispatch(peer));

				for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
				{
					if (packet->data[0] == ID_DISCONNECTION_NOTIFICATION)
						query = false;
					else if (packet->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
						Game::server = peer->GetGuidFromSystemAddress(server);

					try
					{
						Network::Dispatch(peer, NetworkClient::ProcessPacket(packet));
					}
					catch (...)
					{
						peer->DeallocatePacket(packet);
						Network::Dispatch(peer, NetworkClient::ProcessEvent(ID_EVENT_CLIENT_ERROR));
						peer->CloseConnection(server, true, CHANNEL_SYSTEM, HIGH_PRIORITY);
						throw;
					}
				}

				if (!query)
					throw VaultException("Server closed connection");

				if (initialized && !Interface::IsAvailable())
				{
					Network::Dispatch(peer, NetworkClient::ProcessEvent(ID_EVENT_INTERFACE_LOST));
					peer->CloseConnection(server, true, CHANNEL_SYSTEM, HIGH_PRIORITY);
					query = false;

					if (!Interface::HasShutdown())
						throw VaultException("Interface lost, game closed unexpectedly");
				}

				this_thread::sleep_for(chrono::milliseconds(1));
			}
		}
		else
			throw VaultException("Could not establish connection to server");
	}
	catch (...)
	{
		Bethesda::Terminate(peer);

#ifdef VAULTMP_DEBUG
		debug->Print("Network thread is going to terminate (ERROR)", true);
#endif
		throw;
	}

	Bethesda::Terminate(peer);

#ifdef VAULTMP_DEBUG
	debug->Print("Network thread is going to terminate (no error occured)", true);
#endif
}
