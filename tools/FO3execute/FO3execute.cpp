#include <windows.h>
#include <string>
#include <iostream>

HWND FO3_hwnd;
DWORD FO3_id;
HANDLE FO3_process;

unsigned* Fallout3opTrigger;      // 0x00 = trigger disabled. 0x0A = trigger enabled.
char* Fallout3input;        // Command input storage
bool* Fallout3mutex;           // Command queue mutex

void seDebugPrivilege()
{
	TOKEN_PRIVILEGES priv;
	HANDLE hThis, hToken;
	LUID luid;
	hThis = GetCurrentProcess();
	OpenProcessToken( hThis, TOKEN_ADJUST_PRIVILEGES, &hToken );
	LookupPrivilegeValue( 0, "seDebugPrivilege", &luid );
	priv.PrivilegeCount = 1;
	priv.Privileges[0].Luid = luid;
	priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges( hToken, false, &priv, 0, 0, 0 );
	CloseHandle( hToken );
	CloseHandle( hThis );
}

unsigned offset( unsigned src, unsigned dest, unsigned jmpbytes )
{
	unsigned way;
	src += jmpbytes;
	way = dest - src;
	return way;
}

bool getFO3()
{

	FO3_hwnd = FindWindowW( NULL, L"Fallout3" );

	if ( !FO3_hwnd )
		return false;

	else
	{

		GetWindowThreadProcessId( FO3_hwnd, &FO3_id );

		seDebugPrivilege();

		FO3_process = OpenProcess( PROCESS_ALL_ACCESS, false, FO3_id );

		if ( !FO3_process )
			return false;

		DWORD rw = 0;
		unsigned bytes = 0;
		unsigned tmp = 0x0A; // see below
		char bytestream[16];

		HANDLE hProc;
		hProc = FO3_process;

		Fallout3opTrigger = ( unsigned* ) VirtualAllocEx( hProc, 0, 4, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ); // reserve space for trigger (4byte)
		Fallout3input = ( char* ) VirtualAllocEx( hProc, 0, 256, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ); // reserve space for command input (256byte)
		Fallout3mutex = ( bool* ) VirtualAllocEx( hProc, 0, 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ); // reserve space for boolean mutex (1byte)

		WriteProcessMemory( FO3_process, ( LPVOID ) Fallout3opTrigger, &tmp, sizeof( tmp ), NULL ); // set trigger to 0x0A (that is, the FO3 process will start to process inputs from the beginning)
		// with setting Fallout3opTrigger back to 0x00, you can disable command processing

		/* Patching key event and console */

		/* 0062B663   83FB 0A          CMP EBX,A
		 * 0062B666   90               NOP
		 *
		 * 0062B66B   0F85 BE150000    JNZ Fallout3.0062CC2F
		 * 00627A84   EB 6E            JMP SHORT Fallout3.00627AF4
		 * 0062B67E   EB 1B            JMP SHORT Fallout3.0062B69B
		 *
		 * 00627AFF   E9 990D0000      JMP Fallout3.0062889D
		 * 00627B04   90               NOP

		 * 0062B69B   90               NOP
		 * 0062B69C   90               NOP
		 * 0062B69D   90               NOP
		 * 0062B69E   90               NOP
		 * 0062B69F   90               NOP
		 * 0062B6A0   90               NOP
		 * 0062B6A1   90               NOP
		 *
		 * 0062B744   83FB 0A          CMP EBX,A
		 * 0062B747   90               NOP
		 * 0062B748   90               NOP
		 * 0062B749   90               NOP
		 *
		 * 006195E8   90               NOP
		 * 006195E9   90               NOP
		 * 006195EA   90               NOP
		 * 006195EB   90               NOP
		 * 006195EC   90               NOP
		 * 006195ED   90               NOP
		 * 006195EE   90               NOP
		 * 006195EF   90               NOP
		 * 006195F0   90               NOP
		 * 006195F1   90               NOP
		 * 006195F2   90               NOP
		 * 006195F3   90               NOP
		 * 006195F4   90               NOP
		 * 006195F5   90               NOP
		 * 006195F6   90               NOP
		 */

		bytestream[0] = 0x83;
		bytestream[1] = 0xFB;
		bytestream[2] = 0x0A;
		bytestream[3] = 0x90;

		for ( int i = 0; i < 4; i++ ) WriteProcessMemory( hProc, ( LPVOID ) ( 0x0062B663 + i ), &bytestream[i], sizeof( bytestream[i] ), &rw );

		bytestream[0] = 0x0F;
		bytestream[1] = 0x85;
		bytestream[2] = 0xBE;
		bytestream[3] = 0x15;
		bytestream[4] = 0x00;
		bytestream[5] = 0x00;

		for ( int i = 0; i < 6; i++ ) WriteProcessMemory( hProc, ( LPVOID ) ( 0x0062B66B + i ), &bytestream[i], sizeof( bytestream[i] ), &rw );

		bytestream[0] = 0xEB;
		bytestream[1] = 0x6E;

		for ( int i = 0; i < 2; i++ ) WriteProcessMemory( hProc, ( LPVOID ) ( 0x00627A84 + i ), &bytestream[i], sizeof( bytestream[i] ), &rw );

		bytestream[0] = 0xEB;
		bytestream[1] = 0x1B;

		for ( int i = 0; i < 2; i++ ) WriteProcessMemory( hProc, ( LPVOID ) ( 0x0062B67E + i ), &bytestream[i], sizeof( bytestream[i] ), &rw );

		bytestream[0] = 0xE9;
		bytestream[1] = 0x99;
		bytestream[2] = 0x0D;
		bytestream[3] = 0x00;
		bytestream[4] = 0x00;
		bytestream[5] = 0x90;

		for ( int i = 0; i < 6; i++ ) WriteProcessMemory( hProc, ( LPVOID ) ( 0x00627AFF + i ), &bytestream[i], sizeof( bytestream[i] ), &rw );

		bytestream[0] = 0x90;
		bytestream[1] = 0x90;
		bytestream[2] = 0x90;
		bytestream[3] = 0x90;
		bytestream[4] = 0x90;
		bytestream[5] = 0x90;
		bytestream[6] = 0x90;

		for ( int i = 0; i < 7; i++ ) WriteProcessMemory( hProc, ( LPVOID ) ( 0x0062B69B + i ), &bytestream[i], sizeof( bytestream[i] ), &rw );

		bytestream[0] = 0x83;
		bytestream[1] = 0xFB;
		bytestream[2] = 0x0A;
		bytestream[3] = 0x90;
		bytestream[4] = 0x90;
		bytestream[5] = 0x90;

		for ( int i = 0; i < 6; i++ ) WriteProcessMemory( hProc, ( LPVOID ) ( 0x0062B744 + i ), &bytestream[i], sizeof( bytestream[i] ), &rw );

		bytestream[0] = 0x90;
		bytestream[1] = 0x90;
		bytestream[2] = 0x90;
		bytestream[3] = 0x90;
		bytestream[4] = 0x90;
		bytestream[5] = 0x90;
		bytestream[6] = 0x90;
		bytestream[7] = 0x90;
		bytestream[8] = 0x90;
		bytestream[9] = 0x90;
		bytestream[10] = 0x90;
		bytestream[11] = 0x90;
		bytestream[12] = 0x90;
		bytestream[13] = 0x90;
		bytestream[14] = 0x90;

		for ( int i = 0; i < 15; i++ ) WriteProcessMemory( hProc, ( LPVOID ) ( 0x006195E8 + i ), &bytestream[i], sizeof( bytestream[i] ), &rw );

		/* Writing Fallout3 command trigger TOTAL BYTES TO RESERVE: 20 */

		/* XXXXXXXX   8B1D XXXXXXXX    MOV EBX,[XXXXXXXX]
		 * XXXXXXXX   83FB 0A          CMP EBX,A
		 * XXXXXXXX  -0F84 XXXXXXXX    JE Fallout3.006288CA
		 * XXXXXXXX  -E9 XXXXXXXX      JMP Fallout3.0062897A
		 *
		 * 006288C4  -E9 XXXXXXXX      JMP XXXXXXXX
		 * 006288C9   90               NOP
		 */

		bytes = 0;
		rw = 0;

		LPVOID Fallout3triggerASM = VirtualAllocEx( hProc, 0, 20, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

		tmp = ( unsigned ) Fallout3opTrigger;
		bytestream[0] = 0x8B;
		bytestream[1] = 0x1D;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3triggerASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3triggerASM ) + bytes ), &tmp, sizeof( tmp ), &rw );
		bytes += rw;

		bytestream[0] = 0x83;
		bytestream[1] = 0xFB;
		bytestream[2] = 0x0A;

		for ( int i = 0; i < 3; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3triggerASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		tmp = offset( ( ( ( unsigned ) Fallout3triggerASM ) + bytes ), ( unsigned ) 0x006288CA, 6 );
		bytestream[0] = 0x0F;
		bytestream[1] = 0x84;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3triggerASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3triggerASM ) + bytes ), &tmp, sizeof( tmp ), &rw );
		bytes += rw;

		tmp = offset( ( ( ( unsigned ) Fallout3triggerASM ) + bytes ), ( unsigned ) 0x0062897A, 5 );
		bytestream[0] = 0xE9;
		WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3triggerASM ) + bytes ), &bytestream[0], sizeof( bytestream[0] ), &rw );
		bytes += rw;
		WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3triggerASM ) + bytes ), &tmp, sizeof( tmp ), &rw );
		bytes += rw;

		tmp = offset( ( unsigned ) 0x006288C4, ( unsigned ) Fallout3triggerASM, 5 );
		bytestream[0] = 0xE9;
		bytes = 0;
		WriteProcessMemory( hProc, ( LPVOID ) ( 0x006288C4 + bytes ), &bytestream[0], sizeof( bytestream[0] ), &rw );
		bytes += rw;
		WriteProcessMemory( hProc, ( LPVOID ) ( 0x006288C4 + bytes ), &tmp, sizeof( tmp ), &rw );
		bytes += rw;
		bytestream[0] = 0x90;
		WriteProcessMemory( hProc, ( LPVOID ) ( 0x006288C4 + bytes ), &bytestream[0], sizeof( bytestream[0] ), &rw );
		bytes += rw;

		/* Writing Fallout3 command INPUT detour TOTAL BYTES TO RESERVE: 64 */

		/* XXXXXXXX   50               PUSH EAX
		 * XXXXXXXX   51               PUSH ECX
		 * XXXXXXXX   56               PUSH ESI
		 * XXXXXXXX   8A06             MOV AL,BYTE PTR DS:[ESI]
		 * XXXXXXXX   3C 00            CMP AL,0
		 * XXXXXXXX   74 0D            JE SHORT XXXXXXXX
		 * XXXXXXXX   5E               POP ESI
		 * XXXXXXXX   59               POP ECX
		 * XXXXXXXX   58               POP EAX
		 * XXXXXXXX   BA FFFEFE7E      MOV EDX,7EFEFEFF
		 * XXXXXXXX  -E9 XXXXXXXX      JMP Fallout3.00C075B4
		 * XXXXXXXX   B9 XXXXXXXX      MOV ECX,XXXXXXXX
		 * XXXXXXXX   8A01             MOV AL,BYTE PTR DS:[ECX]
		 * XXXXXXXX   3C 00            CMP AL,0
		 * XXXXXXXX  ^74 E8            JE SHORT XXXXXXXX
		 * XXXXXXXX   8806             MOV BYTE PTR DS:[ESI],AL
		 * XXXXXXXX   C601 00          MOV BYTE PTR DS:[ECX],0
		 * XXXXXXXX   83C1 01          ADD ECX,1
		 * XXXXXXXX   83C6 01          ADD ESI,1
		 * XXXXXXXX   8A01             MOV AL,BYTE PTR DS:[ECX]
		 * XXXXXXXX   3C 00            CMP AL,0
		 * XXXXXXXX   74 02            JE SHORT XXXXXXXX
		 * XXXXXXXX  ^EB ED            JMP SHORT XXXXXXXX
		 * XXXXXXXX   C605 XXXXXXXX 00 MOV BYTE PTR DS:[XXXXXXXX],0
		 * XXXXXXXX   C606 00          MOV BYTE PTR DS:[ESI],0
		 * XXXXXXXX  ^EB C9            JMP SHORT XXXXXXXX
		 *
		 * 00C075AF   -E9 XXXXXXXX     JMP XXXXXXXX
		 */

		bytes = 0;
		rw = 0;

		LPVOID Fallout3inputASM = VirtualAllocEx( hProc, 0, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

		bytestream[0] = 0x50;
		bytestream[1] = 0x51;
		bytestream[2] = 0x56;

		for ( int i = 0; i < 3; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x8A;
		bytestream[1] = 0x06;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x3C;
		bytestream[1] = 0x00;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x74;
		bytestream[1] = 0x0D;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x5E;
		bytestream[1] = 0x59;
		bytestream[2] = 0x58;

		for ( int i = 0; i < 3; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0xBA;
		bytestream[1] = 0xFF;
		bytestream[2] = 0xFE;
		bytestream[3] = 0xFE;
		bytestream[4] = 0x7E;

		for ( int i = 0; i < 5; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		tmp = offset( ( ( ( unsigned ) Fallout3inputASM ) + bytes ), ( unsigned ) 0x00C075B4, 5 );
		bytestream[0] = 0xE9;
		WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[0], sizeof( bytestream[0] ), &rw );
		bytes += rw;
		WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &tmp, sizeof( tmp ), &rw );
		bytes += rw;

		tmp = ( unsigned ) Fallout3input;
		bytestream[0] = 0xB9;
		WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[0], sizeof( bytestream[0] ), &rw );
		bytes += rw;
		WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &tmp, sizeof( tmp ), &rw );
		bytes += rw;

		bytestream[0] = 0x8A;
		bytestream[1] = 0x01;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x3C;
		bytestream[1] = 0x00;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x74;
		bytestream[1] = 0xE8;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x88;
		bytestream[1] = 0x06;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0xC6;
		bytestream[1] = 0x01;
		bytestream[2] = 0x00;

		for ( int i = 0; i < 3; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x83;
		bytestream[1] = 0xC1;
		bytestream[2] = 0x01;

		for ( int i = 0; i < 3; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x83;
		bytestream[1] = 0xC6;
		bytestream[2] = 0x01;

		for ( int i = 0; i < 3; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x8A;
		bytestream[1] = 0x01;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x3C;
		bytestream[1] = 0x00;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0x74;
		bytestream[1] = 0x02;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0xEB;
		bytestream[1] = 0xED;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		tmp = ( unsigned ) Fallout3mutex;
		bytestream[0] = 0xC6;
		bytestream[1] = 0x05;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &tmp, sizeof( tmp ), &rw );
		bytes += rw;
		bytestream[0] = 0x00;
		WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[0], sizeof( bytestream[0] ), &rw );
		bytes += rw;

		bytestream[0] = 0xC6;
		bytestream[1] = 0x06;
		bytestream[2] = 0x00;

		for ( int i = 0; i < 3; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		bytestream[0] = 0xEB;
		bytestream[1] = 0xC9;

		for ( int i = 0; i < 2; i++ )
		{
			WriteProcessMemory( hProc, ( LPVOID ) ( ( ( unsigned ) Fallout3inputASM ) + bytes ), &bytestream[i], sizeof( bytestream[i] ), &rw );
			bytes += rw;
		}

		tmp = offset( ( unsigned ) 0x00C075AF, ( ( unsigned ) Fallout3inputASM ), 5 );
		bytestream[0] = 0xE9;
		bytes = 0;
		WriteProcessMemory( hProc, ( LPVOID ) ( ( unsigned ) 0x00C075AF + bytes ), &bytestream[0], sizeof( bytestream[0] ), &rw );
		bytes += rw;
		WriteProcessMemory( hProc, ( LPVOID ) ( ( unsigned ) 0x00C075AF + bytes ), &tmp, sizeof( tmp ), &rw );
	}

	return true;
}

int main( int argc, char *argv[] )
{
	std::cout << "Run Fallout3.exe...\n";

	while ( !getFO3() ) Sleep( 100 ); // wait for Fallout3.exe

	std::string input;

	while ( input.compare( "exit" ) != 0 )
	{
		std::getline( std::cin, input, '\n' ); // get input from command line

		char tmp[256]; // we need a C string version
		strcpy( tmp, input.c_str() );

		// the Fallout3mutex check is missing here; we would have to read the memory (Fallout3mutex) and wait until the boolean is set to false

		WriteProcessMemory( FO3_process, ( LPVOID ) Fallout3input, &tmp, sizeof( tmp ), NULL ); // copy the input to Fallout3input (the memspace we specifically reserved for that purpose in the FO3 process)

		// in case the trigger is set to 0x0A (default), the input will now be processed
	}

	std::cout << "Thanks for using FO3execute tool.\n";

	system( "PAUSE" );

	return 0;

}
