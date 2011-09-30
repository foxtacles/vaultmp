#include "vaultscript.h"
#include <cstdio>

using namespace std;
using namespace vaultmp;

void MyTimer()
{

}

void VAULTSCRIPT exec()
{
    printf("My first C++ vaultscript <3\n");
    SetServerName("vaultmp 0.1a server");
    SetServerRule("website", "vaultmp.com");

    switch (GetGameCode())
    {
    case FALLOUT3:
        SetServerMap("the wasteland");
        break;
    case NEWVEGAS:
        SetServerMap("mojave desert");
        break;
    case OBLIVION:
        SetServerMap("cyrodiil");
        break;
    }

    //CreateTimer(&MyTimer, 5000);
}

bool VAULTSCRIPT OnClientAuthenticate(string name, string pwd)
{
    return true;
}

void VAULTSCRIPT OnPlayerDisconnect(ID player, Reason reason)
{
}

Base VAULTSCRIPT OnPlayerRequestGame(ID player)
{
    Base base = 0x00000000;

    switch (GetGameCode())
    {
    case FALLOUT3:
        base = 0x00030D82; // Carter
        break;
    case NEWVEGAS:
        base = 0x0010C0BE; // Jessup
        break;
    case OBLIVION:
        base = 0x000A3166; // Achille
        break;
    }

    return base;
}

void VAULTSCRIPT OnSpawn(ID object)
{

}

void VAULTSCRIPT OnCellChange(ID object, Cell cell)
{

}

void VAULTSCRIPT OnActorValueChange(ID actor, Index index, Value value)
{

}

void VAULTSCRIPT OnActorBaseValueChange(ID actor, Index index, Value value)
{

}

void VAULTSCRIPT OnActorAlert(ID actor, State alerted)
{

}

void VAULTSCRIPT OnActorSneak(ID actor, State sneaking)
{

}

void VAULTSCRIPT OnActorDeath(ID actor)
{

}
