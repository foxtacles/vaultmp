#include "Bethesda.h"

using namespace std;
using namespace RakNet;

typedef HINSTANCE ( __stdcall *fLoadLibrary )( char* );

unsigned char Bethesda::game = 0x00;
bool Bethesda::initialized = false;
string Bethesda::password = "";
Savegame Bethesda::savegame;
ModList Bethesda::modfiles;
char Bethesda::module[32];

#ifdef VAULTMP_DEBUG
Debug* Bethesda::debug;
#endif

void Bethesda::CommandHandler( signed int key, vector<double>& info, double result, bool error )
{
	using namespace Values;
	unsigned short opcode = getFrom<double, unsigned short>(info.at( 0 ));

	if ( !error )
	{
#ifdef VAULTMP_DEBUG
		//debug->PrintFormat("Executing command %04hX on reference %08X", true, opcode, info.size() > 1 ? getFrom<double, unsigned int>(info.at(1)) : 0);
#endif

		Lockable* data = NULL;

		if ( key != 0x00000000 )
			data = Lockable::BlindUnlock( key );

		FactoryObject reference, self;

		switch ( opcode )
		{
			case Functions::Func_PlaceAtMe:
				Game::PlaceAtMe( data, getFrom<double, unsigned int>(result) );
				break;

			case Functions::Func_GetPos:
				reference = GameFactory::GetObject( getFrom<double, unsigned int>(info.at(1)) );
				Game::GetPos( reference, getFrom<double, unsigned char>( info.at( 2 ) ), result);
				break;

			case Functions::Func_SetPos:
				break;

			case Functions::Func_GetAngle:
				reference = GameFactory::GetObject( getFrom<double, unsigned int>(info.at(1)) );
				Game::GetAngle( reference, getFrom<double, unsigned char>( info.at( 2 ) ), result);
				break;

			case Functions::Func_SetAngle:
				break;

			case Functions::Func_GetActorValue:
				reference = GameFactory::GetObject( getFrom<double, unsigned int>(info.at(1)) );
				Game::GetActorValue( reference, false, getFrom<double, unsigned char>( info.at( 2 ) ), result);
				break;

			case Functions::Func_ForceActorValue:
				break;

			case Functions::Func_GetBaseActorValue:
				reference = GameFactory::GetObject( getFrom<double, unsigned int>(info.at(1)) );
				Game::GetActorValue( reference, true, getFrom<double, unsigned char>( info.at( 2 ) ), result);
				break;

			case Functions::Func_SetActorValue:
				break;

			case Functions::Func_GetActorState:
				{
					reference = GameFactory::GetObject( getFrom<double, unsigned int>(info.at(1)) );
					Game::GetActorState( reference,
										 *reinterpret_cast<unsigned char*>(((unsigned) &result) + 4 ),
										 *reinterpret_cast<unsigned char*>(((unsigned) &result) + 5 ),
										 *reinterpret_cast<bool*>(&result),
										 *reinterpret_cast<bool*>(((unsigned) &result) + 1));
					break;
				}

			case Functions::Func_PlayGroup:
				break;

			case Functions::Func_GetDead:
				break;

			case Functions::Func_MoveTo:
				break;

			case Functions::Func_Enable:
				break;

			case Functions::Func_Disable:
				break;

			case Functions::Func_SetRestrained:
				break;

			case Functions::Func_SetAlert:
				break;

			case Functions::Func_SetForceSneak:
				break;

			case Fallout::Functions::Func_ScanContainer:
			{
				reference = GameFactory::GetObject( getFrom<double, unsigned int>(info.at(1)) );
				vector<unsigned char>* data = getFrom<double, vector<unsigned char>*>(result);
				Game::ScanContainer( reference, *data);
				delete data;
				break;
			}

			case Fallout::Functions::Func_MarkForDelete:
				break;

			case Fallout3::Functions::Func_GetParentCell:
			case FalloutNV::Functions::Func_GetParentCell:
				{
					reference = GameFactory::GetObject( getFrom<double, unsigned int>(info.at(1)) );
					self = GameFactory::GetObject( PLAYER_REFERENCE );
					Game::GetParentCell( vector<FactoryObject> {reference, self}, getFrom<double, unsigned int>(result) );
					break;
				}

			case Fallout3::Functions::Func_GetControl:
			case FalloutNV::Functions::Func_GetControl:
				self = GameFactory::GetObject( PLAYER_REFERENCE );
				Game::GetControl( self, getFrom<double, int>(info.at(1)), result);
				break;

			case Fallout3::Functions::Func_Load:
			case FalloutNV::Functions::Func_Load:
                Game::LoadEnvironment();
				break;

			case Fallout3::Functions::Func_SetName:
			case FalloutNV::Functions::Func_SetName:
				break;

			default:
				throw VaultException( "Unhandled function %04hX", opcode );
		}
	}

	else
	{
#ifdef VAULTMP_DEBUG
		debug->PrintFormat( "Command %04hX failed", true, opcode);
#endif

		switch ( opcode )
		{
			case Functions::Func_PlaceAtMe:
				Game::Failure_PlaceAtMe( getFrom<double, unsigned int>(info.at(1)), getFrom<double, unsigned int>(info.at(2)), getFrom<double, unsigned int>(info.at(3)), key );
				break;

			default:
				break;
		}
	}
}

DWORD Bethesda::lookupProgramID( const char process[] )
{
	HANDLE hSnapshot;
	PROCESSENTRY32 ProcessEntry;
	ProcessEntry.dwSize = sizeof( PROCESSENTRY32 );
	hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	if ( Process32First( hSnapshot, &ProcessEntry ) )
		do
		{
			if ( !strcmp( ProcessEntry.szExeFile, process ) )
			{
				CloseHandle( hSnapshot );
				return ProcessEntry.th32ProcessID;
			}
		}
		while( Process32Next( hSnapshot, &ProcessEntry ) );

	CloseHandle( hSnapshot );

	return 0;
}

void Bethesda::Initialize()
{
	switch ( Bethesda::game = game )
	{
		case FALLOUT3:
			strcpy( module, "Fallout3.exe" );
			break;

		case NEWVEGAS:
			SetEnvironmentVariable( "SteamAppID", "22380" );
			strcpy( module, "FalloutNV.exe" );
			break;

		default:
			throw VaultException( "Bad game ID %08X", Bethesda::game );
	}

	TCHAR savefile[MAX_PATH];
	ZeroMemory( savefile, sizeof( savefile ) );
	SHGetFolderPath( NULL, CSIDL_PERSONAL, NULL, 0, savefile ); // SHGFP_TYPE_CURRENT

	switch ( Bethesda::game )
	{
		case FALLOUT3:
			strcat( savefile, "\\My Games\\Fallout3\\Saves\\" );
			break;

		case NEWVEGAS:
			strcat( savefile, "\\My Games\\FalloutNV\\Saves\\" );
			break;
	}

	strcat( savefile, Utils::FileOnly( Bethesda::savegame.first.c_str() ) );
	unsigned int crc;

	if ( !Utils::crc32file( savefile, &crc ) )
		throw VaultException( "Could not find savegame file:\n\n%s\n\nAsk the server owner to send you the file or try to Synchronize with the server", savefile );

	if ( crc != Bethesda::savegame.second )
		throw VaultException( "Savegame differs from the server version:\n\n%s\n\nAsk the server owner to send you the file or try to Synchronize with the server", savefile );

	TCHAR curdir[MAX_PATH];
	ZeroMemory( curdir, sizeof( curdir ) );
	GetModuleFileName( GetModuleHandle( NULL ), ( LPTSTR ) curdir, MAX_PATH );
	PathRemoveFileSpec( curdir );

	strcat( curdir, "\\Data\\" );

	for ( ModList::iterator it = modfiles.begin(); it != modfiles.end(); ++it )
	{
		TCHAR modfile[MAX_PATH];
		ZeroMemory( modfile, sizeof( modfile ) );
		strcat( modfile, curdir );
		strcat( modfile, it->first.c_str() );

		if ( !Utils::crc32file( modfile, &crc ) )
			throw VaultException( "Could not find modification file:\n\n%s\n\nAsk the server owner to send you the file or try to Synchronize with the server", modfile );

		if ( crc != it->second )
			throw VaultException( "Modfile differs from the server version:\n\n%s\n\nAsk the server owner to send you the file or try to Synchronize with the server", modfile );
	}

	ZeroMemory( savefile, sizeof( savefile ) );
	SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, savefile ); // SHGFP_TYPE_CURRENT

	switch ( Bethesda::game )
	{
		case FALLOUT3:
			strcat( savefile, "\\Fallout3\\plugins.txt" );
			break;

		case NEWVEGAS:
			strcat( savefile, "\\FalloutNV\\plugins.txt" );
			break;
	}

	FILE* plugins = fopen( savefile, "w" );

	switch ( Bethesda::game )
	{
		case FALLOUT3:
			{
				char esm[] = "Fallout3.esm\n";
				fwrite( esm, sizeof( char ), strlen( esm ), plugins );
				break;
			}

		case NEWVEGAS:
			{
				char esm[] = "FalloutNV.esm\n";
				fwrite( esm, sizeof( char ), strlen( esm ), plugins );
				break;
			}
	}

	for ( ModList::iterator it = modfiles.begin(); it != modfiles.end(); ++it )
	{
		fwrite( it->first.c_str(), sizeof( char ), it->first.length(), plugins );
		fwrite( "\n", sizeof( char ), 1, plugins );
	}

	fclose( plugins );

	if ( lookupProgramID( module ) == 0 )
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		HANDLE hRemoteThread;
		HINSTANCE hDll;
		fLoadLibrary pLoadLibrary;
		LPVOID remote;

		ZeroMemory( &si, sizeof( si ) );
		ZeroMemory( &pi, sizeof( pi ) );
		si.cb = sizeof( si );

		if ( CreateProcess( module, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi ) )
		{
			CloseHandle( si.hStdInput );
			CloseHandle( si.hStdOutput );
			CloseHandle( si.hStdError );

			GetModuleFileName( GetModuleHandle( NULL ), ( LPTSTR ) curdir, MAX_PATH );
			PathRemoveFileSpec( curdir );
			strcat( curdir, "\\vaultmp.dll" );
			unsigned int size = strlen( curdir ) + 1;

			hDll = LoadLibrary( "kernel32.dll" );
			pLoadLibrary = ( fLoadLibrary ) GetProcAddress( hDll, "LoadLibraryA" ); // TODO: GetRemoteProcAddress

			if( ( remote = VirtualAllocEx( pi.hProcess, 0, size, MEM_COMMIT, PAGE_READWRITE ) ) == NULL )
			{
				VirtualFreeEx( pi.hProcess, remote, size, MEM_RELEASE );
				CloseHandle( pi.hThread );
				CloseHandle( pi.hProcess );
				throw VaultException( "Couldn't allocate memory in remote process" );
			}

			if( WriteProcessMemory( pi.hProcess, remote, curdir, size, NULL ) == false )
			{
				VirtualFreeEx( pi.hProcess, remote, size, MEM_RELEASE );
				CloseHandle( pi.hThread );
				CloseHandle( pi.hProcess );
				throw VaultException( "Couldn't write memory in remote process" );
			}

			if( ( hRemoteThread = CreateRemoteThread( pi.hProcess, NULL, 0, ( LPTHREAD_START_ROUTINE ) pLoadLibrary, remote, 0, 0 ) ) == NULL )
			{
				VirtualFreeEx( pi.hProcess, remote, size, MEM_RELEASE );
				CloseHandle( pi.hThread );
				CloseHandle( pi.hProcess );
				throw VaultException( "Couldn't create remote thread" );
			}

			if( WaitForSingleObject( hRemoteThread, 5000 ) != WAIT_OBJECT_0 )
			{
				VirtualFreeEx( pi.hProcess, remote, size, MEM_RELEASE );
				throw VaultException( "Remote thread timed out" );
			}

			VirtualFreeEx( pi.hProcess, remote, size, MEM_RELEASE );

			try
			{
				Interface::Initialize( &CommandHandler, Bethesda::game );

                chrono::steady_clock::time_point till = chrono::steady_clock::now() + chrono::milliseconds(5000);

                while (chrono::steady_clock::now() < till && !Interface::IsAvailable())
                    this_thread::sleep_for(chrono::milliseconds(100));

				if ( !Interface::IsAvailable() )
					throw VaultException( "Failed connecting to vaultmp interface" );
			}

			catch ( ... )
			{
				CloseHandle( pi.hThread );
				CloseHandle( pi.hProcess );
				throw;
			}

			ResumeThread( pi.hThread );
			CloseHandle( pi.hThread );
			CloseHandle( pi.hProcess );

			this_thread::sleep_for(chrono::milliseconds(5000));

			initialized = true;
		}

		else
			throw VaultException( "Failed creating the game process" );
	}

	else
		throw VaultException( "Either Fallout 3 or Fallout: New Vegas is already runnning" );
}

void Bethesda::Terminate()
{
    DWORD id;

    if (*module && (id = Bethesda::lookupProgramID(module)))
    {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, id);
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
    }
}

void Bethesda::InitializeVaultMP( RakPeerInterface* peer, SystemAddress server, string name, string pwd, unsigned char game )
{
	Bethesda::game = game;
	Bethesda::password = pwd;
	Bethesda::savegame = Savegame();
	Bethesda::modfiles.clear();
	ZeroMemory(module, sizeof(module));
	Game::game = game;
	initialized = false;

#ifdef VAULTMP_DEBUG
	debug = new Debug( ( char* ) "vaultmp" );
	debug->PrintFormat( "Vault-Tec Multiplayer Mod client debug log (%s)", false, CLIENT_VERSION );
	debug->PrintFormat( "Connecting to server: %s (name: %s, password: %s, game: %s)", false, server.ToString(), name.c_str(), pwd.c_str(), game == FALLOUT3 ? ( char* ) "Fallout 3" : ( char* ) "Fallout New Vegas" );
	debug->Print( "Visit www.vaultmp.com for help and upload this log if you experience problems with the mod.", false );
	debug->Print( "-----------------------------------------------------------------------------------------------------", false );
	//debug->PrintSystem();
	API::SetDebugHandler( debug );
	VaultException::SetDebugHandler( debug );
	NetworkClient::SetDebugHandler( debug );
	Interface::SetDebugHandler( debug );
	Lockable::SetDebugHandler( debug );
	Object::SetDebugHandler( debug );
	Item::SetDebugHandler( debug );
	Container::SetDebugHandler( debug );
	Actor::SetDebugHandler( debug );
	Player::SetDebugHandler( debug );
	Game::SetDebugHandler( debug );
	GameFactory::SetDebugHandler( debug );
#endif

	GameFactory::Initialize( game );
	API::Initialize( game );

	NetworkID id = GameFactory::CreateInstance( ID_PLAYER, PLAYER_REFERENCE, PLAYER_BASE );
	FactoryObject reference = GameFactory::GetObject( id );
	Player* self = vaultcast<Player>( reference );
	self->SetEnabled( true );
	self->SetName( name );
	GameFactory::LeaveReference( reference );
	self = NULL; // lets make sure that we dont use this by accident somewhere (old version code did so)

	Network::Flush();

	try
	{
		if ( peer->Connect( server.ToString( false ), server.GetPort(), DEDICATED_VERSION, sizeof( DEDICATED_VERSION ), 0, 0, 3, 500, 0 ) == CONNECTION_ATTEMPT_STARTED )
		{
			bool query = true;
			Packet* packet;

			while ( query )
			{
				NetworkResponse response;

				while ( ( response = Network::Next() ).size() )
					Network::Dispatch( peer, response );

				for ( packet = peer->Receive(); packet; peer->DeallocatePacket( packet ), packet = peer->Receive() )
				{
					if ( packet->data[0] == ID_DISCONNECTION_NOTIFICATION )
						query = false;

					else if ( packet->data[0] == ID_CONNECTION_REQUEST_ACCEPTED )
						Game::server = peer->GetGuidFromSystemAddress( server );

					try
					{
						response = NetworkClient::ProcessPacket( packet );
						Network::Dispatch( peer, response );
					}

					catch ( ... )
					{
						peer->DeallocatePacket( packet );
						response = NetworkClient::ProcessEvent( ID_EVENT_CLIENT_ERROR );
						Network::Dispatch( peer, response );
						peer->CloseConnection( server, true, CHANNEL_SYSTEM, HIGH_PRIORITY );
						throw;
					}
				}

				if ( initialized && !Interface::IsAvailable() )
				{
					NetworkResponse response = NetworkClient::ProcessEvent( ID_EVENT_INTERFACE_LOST );
					Network::Dispatch( peer, response );
					peer->CloseConnection( server, true, CHANNEL_SYSTEM, HIGH_PRIORITY );
					throw VaultException( "Lost connection to interface" );
				}

				this_thread::sleep_for(chrono::milliseconds(2));
			}
		}

		else
			throw VaultException( "Could not establish connection to server" );
	}

	catch ( ... )
	{
		this_thread::sleep_for(chrono::milliseconds(200));
		Packet* packet = NULL;

		while ( packet = peer->Receive() ) peer->DeallocatePacket( packet ); // disconnection notification might still arrive

		Interface::Terminate();
		GameFactory::DestroyAllInstances();
		API::Terminate();
		Bethesda::Terminate();

#ifdef VAULTMP_DEBUG
		debug->Print( "Network thread is going to terminate (ERROR)", true );
#endif
		throw;
	}

	Interface::Terminate();
	GameFactory::DestroyAllInstances();
	API::Terminate();
	Bethesda::Terminate();

#ifdef VAULTMP_DEBUG
	debug->Print( "Network thread is going to terminate (no error occured)", true );
#endif
}
