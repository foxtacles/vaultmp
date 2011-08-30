#ifndef SCRIPT_H
#define SCRIPT_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <vector>
#include <string>

#include "PAWN.h"
#include "Dedicated.h"
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

typedef void (*fexec)();
typedef bool (*fOnClientAuthenticate)(string, string);
typedef void (*fOnPlayerDisconnect)(unsigned int, unsigned char);
typedef unsigned int (*fOnPlayerRequestGame)(unsigned int);
typedef void (*fOnPlayerSpawn)(unsigned int);
typedef void (*fOnPlayerDeath)(unsigned int);
typedef void (*fOnPlayerCellChange)(unsigned int, unsigned int);
typedef void (*fOnPlayerValueChange)(unsigned int, bool, unsigned char, double);

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

public:

    static void LoadScripts(char* scripts);
    static void UnloadScripts();

    static bool Authenticate(string name, string pwd);
    static void Disconnect(unsigned int player, unsigned char reason);
    static unsigned int RequestGame(unsigned int player);
    static void CellChange(unsigned int player, unsigned int cell);
    static void ValueChange(unsigned int player, bool base, unsigned char index, double value);

};

#endif
