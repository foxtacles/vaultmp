#include "vaultscript.h"
#include <cstdio>

using namespace std;
using namespace vaultmp;

void MyTimer()
{
    printf("C++ foo\n");
}

// It is not safe to call ANY vaultmp function before the execution of exec()
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

    CreateTimer(&MyTimer, 5000);
}

bool VAULTSCRIPT OnClientAuthenticate(string name, string pwd)
{
    printf("C++: client auth %s, %s\n", name.c_str(), pwd.c_str());
    return true;
}

void VAULTSCRIPT OnPlayerDisconnect(Player player, Reason reason)
{
    printf("C++: player disconnect %d, %d\n", player, reason);
}

Base VAULTSCRIPT OnPlayerRequestGame(Player player)
{
    printf("C++: player game %d\n", player);
    return 0x00030D82;
}

void VAULTSCRIPT OnPlayerSpawn(Player player)
{

}

void VAULTSCRIPT OnPlayerDeath(Player player)
{

}

void VAULTSCRIPT OnPlayerCellChange(Player player, Cell cell)
{
    printf("C++: player cell %d,%08X\n", player, cell);
}

void VAULTSCRIPT OnPlayerValueChange(Player player, Index index, Value value)
{
    printf("C++: player value %s -> %f\n", ValueToString(index).c_str(), (float)value);
}

void VAULTSCRIPT OnPlayerBaseValueChange(Player player, Index index, Value value)
{
    printf("C++: player base value %s -> %f\n", ValueToString(index).c_str(), (float)value);
}

void VAULTSCRIPT OnPlayerStateChange(Player player, Index index, State alerted)
{
    printf("C++: player running animation %s, alerted %d\n", AnimToString(index).c_str(), (int)alerted);
}
