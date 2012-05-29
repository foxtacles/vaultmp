#ifndef INTERFACE_H
#define INTERFACE_H

#include "vaultmp.h"

#include <winsock2.h>
#include <tlhelp32.h>
#include <string>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>
#include <thread>
#include <chrono>
#include <numeric>
#include <algorithm>

#include "API.h"
#include "CriticalSection.h"
#include "Data.h"
#include "Pipe.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;
using namespace Data;

typedef unordered_multimap<string, ParamContainer> Native;
typedef multimap<unsigned int, Native::iterator> PriorityMap;
typedef vector<vector<Native::iterator>> StaticCommandList;
typedef deque<pair<Native::iterator, unsigned int>> DynamicCommandList;

/**
 * \brief Provides facilities to execute engine commands, connects with the game process and is responsible for sending / retrieving game data
 *
 * Uses the API to translate commands and command results
 */

class Interface : public API
{

	private:
		static bool endThread;
		static bool wakeup;
		static bool initialized;
		static thread hCommandThreadReceive;
		static thread hCommandThreadSend;
		static PriorityMap priorityMap;
		static StaticCommandList static_cmdlist;
		static DynamicCommandList dynamic_cmdlist;
		static PipeClient* pipeServer;
		static PipeServer* pipeClient;
		static ResultHandler resultHandler;
		static CriticalSection static_cs;
		static CriticalSection dynamic_cs;
		static Native natives;

		static vector<string> Evaluate(Native::iterator _it);

		static void CommandThreadReceive();
		static void CommandThreadSend();

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		Interface() = delete;

	public:

		/**
		 * \brief Initializes the Interface
		 *
		 * Takes a ResultHandler function pointer and the game code
		 */
		static bool Initialize(ResultHandler);
		/**
		 * \brief Terminates the Interface
		 */
		static void Terminate();
		/**
		 * \brief Checks if the Interface is up and running
		 */
		static bool IsAvailable();

		/**
		 * \brief Starts a setup Interface session
		 *
		 * Used to start input static commands
		 */
		static void StartSetup();
		/**
		 * \brief Ends a setup Interface session
		 *
		 * Must be called when finished inputting static commands
		 */
		static void EndSetup();

		/**
		 * \brief Starts an Interface session
		 *
		 * Used to start input dynamic commands
		 */
		static void StartDynamic();
		/**
		 * \brief Ends an Interface session
		 *
		 * Must be called when finished inputting dynamic commands
		 */
		static void EndDynamic();

		/**
		 * \brief Setups a command
		 *
		 * name refers to the API command
		 * param is a ParamContainer which is a STL list of Parameter's
		 * priority (optional) - the lower this variable, the higher is the priority
		 */
		static void SetupCommand(string name, ParamContainer&& param, unsigned int priority = 1);
		/**
		 * \brief Executes a command once
		 *
		 * name refers to an existing command
		 * see DefineNative for a short explanation of ParamContainer
		 * key (optional) - a signed key (usually from the Lockable class) which is to later identify this command
		 */
		static void ExecuteCommand(string name, ParamContainer&&, unsigned int key = 0);

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif
};

#endif
