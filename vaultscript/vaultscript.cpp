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

bool VAULTSCRIPT OnClientAuthenticate(int client, string name, string pwd)
{
    printf("C++: client auth %d, %s, %s\n", client, name.c_str(), pwd.c_str());
    return true;
}

void VAULTSCRIPT OnPlayerConnect(int player)
{

}

void VAULTSCRIPT OnPlayerDisconnect(int player, unsigned char reason)
{
    printf("C++: player disconnect %d, %d\n", player, (int) reason);
}

int VAULTSCRIPT OnPlayerRequestGame(int player)
{
    return 0;
}

void VAULTSCRIPT OnPlayerSpawn(int player)
{

}

void VAULTSCRIPT OnPlayerDeath(int player)
{

}

void VAULTSCRIPT OnPlayerCellChange(int player, int cell)
{

}

#ifdef __WIN32__
BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    // do not use
    return TRUE;
}
#endif
