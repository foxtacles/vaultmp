#include "vaultscript.h"
#include <cstdio>

using namespace std;
using namespace vaultmp;

void VAULTSCRIPT exec()
{
	printf( "My first C++ vaultscript <3\n" );
	SetServerName( "vaultmp 0.1a server" );
	SetServerRule( "website", "vaultmp.com" );

	switch ( GetGameCode() )
	{
		case FALLOUT3:
			SetServerMap( "the wasteland" );
			break;

		case NEWVEGAS:
			SetServerMap( "mojave desert" );
			break;
	}
}

bool VAULTSCRIPT OnClientAuthenticate( string name, string pwd )
{
	return true;
}

void VAULTSCRIPT OnPlayerDisconnect( ID player, Reason reason )
{

}

Base VAULTSCRIPT OnPlayerRequestGame( ID player )
{
	Base base = 0x00000000;

	switch ( GetGameCode() )
	{
		case FALLOUT3:
			base = 0x00030D82; // Carter
			break;

		case NEWVEGAS:
			base = 0x0010C0BE; // Jessup
			break;
	}

	return base;
}

void VAULTSCRIPT OnSpawn( ID object )
{
    printf("spawn\n");
}

void VAULTSCRIPT OnCellChange( ID object, Cell cell )
{

}

void VAULTSCRIPT OnContainerItemChange( ID container, Base base, Count count, Value value )
{

}

void VAULTSCRIPT OnActorValueChange( ID actor, Index index, Value value )
{

}

void VAULTSCRIPT OnActorBaseValueChange( ID actor, Index index, Value value )
{

}

void VAULTSCRIPT OnActorAlert( ID actor, State alerted )
{

}

void VAULTSCRIPT OnActorSneak( ID actor, State sneaking )
{

}

void VAULTSCRIPT OnActorDeath( ID actor )
{
    printf("dead\n");
}

void VAULTSCRIPT OnActorEquipItem( ID actor, Base base, Value value )
{

}

void VAULTSCRIPT OnActorUnequipItem( ID actor, Base base, Value value )
{

}
