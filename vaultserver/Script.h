#ifndef SCRIPT_H
#define SCRIPT_H

#ifdef __WIN32__
#include <windows.h>
#else
#include <dlfcn.h>
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
typedef void (*fOnPlayerDisconnect)(NetworkID, unsigned char);
typedef unsigned int (*fOnPlayerRequestGame)(NetworkID);
typedef void (*fOnSpawn)(NetworkID);
typedef void (*fOnCellChange)(NetworkID, unsigned int);
typedef void (*fOnActorValueChange)(NetworkID, unsigned char, double);
typedef void (*fOnActorBaseValueChange)(NetworkID, unsigned char, double);
typedef void (*fOnActorAlert)(NetworkID, bool);
typedef void (*fOnActorSneak)(NetworkID, bool);
typedef void (*fOnActorDeath)(NetworkID);

/**
 * \brief Maintains communication with a script
 *
 * A script can be either a C++ or PAWN script
 */

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
    fOnSpawn OnSpawn;
    fOnCellChange OnCellChange;
    fOnActorValueChange OnActorValueChange;
    fOnActorBaseValueChange OnActorBaseValueChange;
    fOnActorAlert OnActorAlert;
    fOnActorSneak OnActorSneak;
    fOnActorDeath OnActorDeath;

    Script(const Script&);
    Script& operator=(const Script&);

public:

    /**
     * \brief Loads a comma separated list of scripts
     *
     * base is the absolute path to the scripts
     */
    static void LoadScripts(char* scripts, char* base);
    /**
     * \brief Unloads all scripts
     */
    static void UnloadScripts();
    /**
     * \brief Creates a C++ timer
     */
    static NetworkID CreateTimer(TimerFunc timer, unsigned int interval);
    /**
     * \brief Creates a PAWN timer
     */
    static NetworkID CreateTimerPAWN(TimerPAWN timer, AMX* amx, unsigned int interval);
    /**
     * \brief Kills a timer
     */
    static void KillTimer(NetworkID id);

    static bool Authenticate(string name, string pwd);
    static void Disconnect(FactoryObject reference, unsigned char reason);
    static unsigned int RequestGame(FactoryObject reference);
    static void CellChange(FactoryObject reference, unsigned int cell);
    static void ValueChange(FactoryObject reference, unsigned char index, bool base, double value);
    static void Alert(FactoryObject reference, bool alerted);
    static void Sneak(FactoryObject reference, bool sneaking);

    static string GetName(NetworkID id);
    static void GetPos(NetworkID id, double& X, double& Y, double& Z);
    static void GetAngle(NetworkID id, double& X, double& Y, double& Z);
    static unsigned int GetCell(NetworkID id);

    static double GetActorValue(NetworkID id, unsigned char index);
    static double GetActorBaseValue(NetworkID id, unsigned char index);
    static unsigned char GetActorMovingAnimation(NetworkID id);
    static bool GetActorAlerted(NetworkID id);
    static bool GetActorSneaking(NetworkID id);
    static bool GetActorDead(NetworkID id);
    static bool IsActorJumping(NetworkID id);

};

#endif
