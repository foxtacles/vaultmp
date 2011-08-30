#include "vaultscript.h"
#include <stdio.h>

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
}

bool VAULTSCRIPT OnClientAuthenticate(string name, string pwd)
{
    printf("C++: client auth %s, %s\n", name.c_str(), pwd.c_str());
    return true;
}

void VAULTSCRIPT OnPlayerDisconnect(unsigned int player, unsigned char reason)
{
    printf("C++: player disconnect %d, %d\n", player, (unsigned int) reason);
}

unsigned int VAULTSCRIPT OnPlayerRequestGame(unsigned int player)
{
    printf("C++: player game %d\n", player);
    return 0x00030D82;
}

void VAULTSCRIPT OnPlayerSpawn(unsigned int player)
{

}

void VAULTSCRIPT OnPlayerDeath(unsigned int player)
{

}

void VAULTSCRIPT OnPlayerCellChange(unsigned int player, unsigned int cell)
{
    printf("C++: player cell %d,%08X\n", player, cell);
}

void VAULTSCRIPT OnPlayerValueChange(unsigned int player, bool base, unsigned char index, double value)
{
    printf("C++: player value %s -> %f\n", ValueToString(index).c_str(), (float)value);
}

#ifdef __WIN32__
BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    // do not use
    return TRUE;
}
#endif
