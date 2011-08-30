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
    VAULTSCRIPT bool OnClientAuthenticate(string name, string pwd);
    VAULTSCRIPT void OnPlayerDisconnect(unsigned int player, unsigned char reason);
    VAULTSCRIPT unsigned int OnPlayerRequestGame(unsigned int player);
    VAULTSCRIPT void OnPlayerSpawn(unsigned int player);
    VAULTSCRIPT void OnPlayerDeath(unsigned int player);
    VAULTSCRIPT void OnPlayerCellChange(unsigned int player, unsigned int cell);
    VAULTSCRIPT void OnPlayerValueChange(unsigned int player, bool base, unsigned char index, double value);

    VAULTSCRIPT void (*timestamp)();
    VAULTSCRIPT void (*SetServerName)(string);
    VAULTSCRIPT void (*SetServerMap)(string);
    VAULTSCRIPT void (*SetServerRule)(string, string);
    VAULTSCRIPT unsigned int (*GetGameCode)();

    VAULTSCRIPT string (*ValueToString)(unsigned char);
    VAULTSCRIPT string (*AxisToString)(unsigned char);
    VAULTSCRIPT string (*AnimToString)(unsigned char);

}
