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
typedef bool (*fOnClientAuthenticate)(int, string, string);
typedef void (*fOnPlayerConnect)(int);
typedef void (*fOnPlayerDisconnect)(int, unsigned char);
typedef int  (*fOnPlayerRequestGame)(int);
typedef void (*fOnPlayerSpawn)(int);
typedef void (*fOnPlayerDeath)(int);
typedef void (*fOnPlayerCellChange)(int, int);

class Script {

private:

    Script(char* path);
    ~Script();

    static vector<Script*> scripts;

    void* handle;
    bool cpp_script;

    fexec exec;
    fOnClientAuthenticate OnClientAuthenticate;
    fOnPlayerConnect OnPlayerConnect;
    fOnPlayerDisconnect OnPlayerDisconnect;
    fOnPlayerRequestGame OnPlayerRequestGame;
    fOnPlayerSpawn OnPlayerSpawn;
    fOnPlayerDeath OnPlayerDeath;
    fOnPlayerCellChange OnPlayerCellChange;

public:

    static void LoadScripts(char* scripts);
    static void UnloadScripts();

    static bool Authenticate(int client, string name, string pwd);
    static void Disconnect(int player, unsigned char reason);

};

#endif
