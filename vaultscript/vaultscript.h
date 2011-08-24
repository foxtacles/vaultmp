/*
 *  vaultscript.h
 *  Don't change anything here unless you REALLY know what you're doing!
 */

#ifdef __WIN32__
#include <windows.h>
#endif
#include <string>

#ifndef __WIN32__
  #define VAULTSCRIPT __attribute__ ((__visibility__("default")))
#else
    #define VAULTSCRIPT __declspec(dllexport)
#endif

#define FALLOUT3                0x01
#define NEWVEGAS                0x02
#define OBLIVION                0x04

using namespace std;
extern "C" {
    VAULTSCRIPT void exec();
    VAULTSCRIPT bool OnClientAuthenticate(int client, string name, string pwd);
    VAULTSCRIPT void OnPlayerConnect(int player);
    VAULTSCRIPT void OnPlayerDisconnect(int player, unsigned char reason);
    VAULTSCRIPT int OnPlayerRequestGame(int player);
    VAULTSCRIPT void OnPlayerSpawn(int player);
    VAULTSCRIPT void OnPlayerDeath(int player);
    VAULTSCRIPT void OnPlayerCellChange(int player, int cell);

    VAULTSCRIPT void (*timestamp)();
    VAULTSCRIPT void (*SetServerName)(string);
    VAULTSCRIPT void (*SetServerMap)(string);
    VAULTSCRIPT void (*SetServerRule)(string, string);
    VAULTSCRIPT int (*GetGameCode)();
}
