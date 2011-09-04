#ifndef SCRIPT_H
#define SCRIPT_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <vector>
#include <string>

#include "PAWN.h"
#include "Dedicated.h"
#include "Timer.h"
#include "../API.h"
#include "../Utils.h"
#include "../vaultmp.h"
#include "../VaultException.h"

#ifdef __WIN32__
#define GetScriptCallback(a,b,c) c = (a) GetProcAddress((HINSTANCE)this->handle,b); \
    if (!c) throw VaultException("Script callback not found: %s", b);
#else
#define GetScriptCallback(a,b,c) c = (a) dlsym(this->handle,b); \
    if (!c) throw VaultException("Script callback not found: %s", b);
#endif

#ifdef __WIN32__
#define SetScriptFunction(a,b) *((unsigned int*)(GetProcAddress((HINSTANCE)this->handle,a)?GetProcAddress((HINSTANCE)this->handle,a):throw VaultException("Script function pointer not found: %s", a)))=(unsigned int)b;
#else
#define SetScriptFunction(a,b) *((unsigned int*)(dlsym(this->handle,a)?dlsym(this->handle,a):throw VaultException("Script function pointer not found: %s", a)))=(unsigned int)b;
#endif

using namespace std;
using namespace Values;

typedef void (*fexec)();
typedef bool (*fOnClientAuthenticate)(string, string);
typedef void (*fOnPlayerDisconnect)(unsigned int, unsigned char);
typedef unsigned int (*fOnPlayerRequestGame)(unsigned int);
typedef void (*fOnPlayerSpawn)(unsigned int);
typedef void (*fOnPlayerDeath)(unsigned int);
typedef void (*fOnPlayerCellChange)(unsigned int, unsigned int);
typedef void (*fOnPlayerValueChange)(unsigned int, unsigned char, double);
typedef void (*fOnPlayerBaseValueChange)(unsigned int, unsigned char, double);
typedef void (*fOnPlayerStateChange)(unsigned int, unsigned char, bool);

class Script {

private:

    Script(char* path);
    ~Script();

    static vector<Script*> scripts;

    void* handle;
    bool cpp_script;

    fexec exec;
    fOnClientAuthenticate OnClientAuthenticate;
    fOnPlayerDisconnect OnPlayerDisconnect;
    fOnPlayerRequestGame OnPlayerRequestGame;
    fOnPlayerSpawn OnPlayerSpawn;
    fOnPlayerDeath OnPlayerDeath;
    fOnPlayerCellChange OnPlayerCellChange;
    fOnPlayerValueChange OnPlayerValueChange;
    fOnPlayerBaseValueChange OnPlayerBaseValueChange;
    fOnPlayerStateChange OnPlayerStateChange;

    Script(const Script&);
    Script& operator=(const Script&);

public:

    static void LoadScripts(char* scripts);
    static void UnloadScripts();
    static NetworkID CreateTimer(TimerFunc timer, unsigned int interval);
    static NetworkID CreateTimerPAWN(TimerPAWN timer, AMX* amx, unsigned int interval);
    static void KillTimer(NetworkID id);

    static bool Authenticate(string name, string pwd);
    static void Disconnect(unsigned int player, unsigned char reason);
    static unsigned int RequestGame(unsigned int player);
    static void CellChange(unsigned int player, unsigned int cell);
    static void ValueChange(unsigned int player, unsigned char index, bool base, double value);
    static void StateChange(unsigned int player, unsigned char index, bool alerted);

    static double GetPlayerPos(unsigned int player, unsigned char index);
    static void GetPlayerPosXYZ(unsigned int player, double& X, double& Y, double& Z);
    static double GetPlayerAngle(unsigned int player, unsigned char index);
    static void GetPlayerAngleXYZ(unsigned int player, double& X, double& Y, double& Z);
    static double GetPlayerValue(unsigned int player, unsigned char index);
    static double GetPlayerBaseValue(unsigned int player, unsigned char index);
    static unsigned int GetPlayerCell(unsigned int player);

};

#endif
