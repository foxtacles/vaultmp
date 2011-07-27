#ifndef COMMAND_H
#define COMMAND_H

#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <list>
#include <vector>

#include "vaultmp.h"
#include "Data.h"
#include "Pipe.h"

#define OPENCMD() while (cmdmutex); cmdmutex = true;
#define CLOSECMD() cmdmutex = false;
#define PUSHCMD(cmd) tmplist.push_back(cmd);
#define STARTSESSION() while (cmdsession); cmdsession = true;
#define ENDSESSION() cmdsession = false;
#define SESSION() cmdsession

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;
using namespace pipe;
using namespace Data;

typedef multimap<string, ParamContainer> Native;
typedef list<pair<Native::iterator, vector<int> > > CommandList;

class Command
{

private:
    static bool endThread;
    static bool wakeup;
    static bool cmdmutex;
    static bool cmdsession;
    static bool initialized;
    static char* module;
    static HANDLE hCommandThreadReceive;
    static HANDLE hCommandThreadSend;
    static CommandList cmdlist;
    static CommandList tmplist;
    static PipeClient* pipeServer;
    static PipeServer* pipeClient;
    static ResultHandler resultHandler;
    static StringHandler stringHandler;

    static map<string, string> defs;
    static Native natives;

    static Native::iterator DefineNativeInternal(string name, ParamContainer);
    static void ExecuteCommand(Native::iterator it, bool loop, int priority = 0, int sleep = 1);
    static list<string> Eval(string name, string def, ParamContainer param);

    static DWORD WINAPI CommandThreadReceive(LPVOID data);
    static DWORD WINAPI CommandThreadSend(LPVOID data);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    Command();

public:

    static HANDLE* Initialize(char* module, ResultHandler, StringHandler);
    static bool Terminate();
    static DWORD lookupProgramID(const char process[]);
    static bool GetReady();

    static void StartSession();
    static void EndSession();

    static void DefineCommand(string name, string def);
    static void DefineNative(string name, ParamContainer param);
    static void ExecuteCommandLoop(string name, int priority = 0, int sleep = 1);
    static void ExecuteCommandOnce(string name, ParamContainer, int priority = 0, int sleep = 1);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif
};

#endif
