#ifndef INTERFACE_H
#define INTERFACE_H

#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <list>
#include <map>
#include <vector>

#include "vaultmp.h"
#include "API.h"
#include "CriticalSection.h"
#include "Data.h"
#include "Pipe.h"

#define PUSHCMD(cmd) tmplist.push_back(cmd);

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;
using namespace Data;

typedef multimap<string, ParamContainer> Native;
typedef list<pair<Native::iterator, vector<int> > > CommandList;

class Interface : public API
{

private:
    static bool endThread;
    static bool wakeup;
    static bool initialized;
    static char* module;
    static HANDLE hCommandThreadReceive;
    static HANDLE hCommandThreadSend;
    static CommandList cmdlist;
    static CommandList tmplist;
    static PipeClient* pipeServer;
    static PipeServer* pipeClient;
    static ResultHandler resultHandler;
    static CriticalSection cs;

    static map<string, string> defs;
    static Native natives;

    static Native::iterator DefineNativeInternal(string name, ParamContainer);
    static void ExecuteCommand(Native::iterator it, bool loop, int priority, int sleep, signed int key);
    static multimap<string, string> Evaluate(string name, string def, ParamContainer param);

    static DWORD WINAPI CommandThreadReceive(LPVOID data);
    static DWORD WINAPI CommandThreadSend(LPVOID data);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    Interface();

public:

    static bool Initialize(char* module, ResultHandler, int game);
    static void Terminate();
    static DWORD lookupProgramID(const char process[]);
    static bool IsAvailable();

    static void StartSession();
    static void EndSession();

    static void DefineCommand(string name, string def, string real = string());
    static void DefineNative(string name, ParamContainer param);
    static void ExecuteCommandLoop(string name, int priority = 0, int sleep = 2);
    static void ExecuteCommandOnce(string name, ParamContainer, int priority = 0, int sleep = 2, signed int key = 0);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif
};

#endif
