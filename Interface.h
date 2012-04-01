#ifndef INTERFACE_H
#define INTERFACE_H

#include "vaultmp.h"

#include <winsock2.h>
#include <tlhelp32.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <thread>
#include <chrono>

#include "API.h"
#include "CriticalSection.h"
#include "Data.h"
#include "Pipe.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;
using namespace Data;

typedef multimap<string, ParamContainer> Native;
typedef list<pair<Native::iterator, vector<int> > > CommandList;

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
		static char* module;
        static thread hCommandThreadReceive;
        static thread hCommandThreadSend;
		static CommandList cmdlist;
		static CommandList tmplist;
		static PipeClient* pipeServer;
		static PipeServer* pipeClient;
		static ResultHandler resultHandler;
		static CriticalSection cs;

		static map<string, string> defs;
		static map<string, string> alias;
		static Native natives;

    static Native::iterator DefineNativeInternal(string name, ParamContainer);
    static void ExecuteCommand(Native::iterator it, bool loop, unsigned int priority, signed int key);
    static multimap<string, string> Evaluate(string name, string def, ParamContainer param);

		static void CommandThreadReceive( char* module );
		static void CommandThreadSend();

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		Interface();

	public:

		/**
		 * \brief Initializes the Interface
		 *
		 * Takes the name of the game executable to initialize, a ResultHandler function pointer and the game code
		 */
		static bool Initialize( char* module, ResultHandler, unsigned char game );
		/**
		 * \brief Terminates the Interface
		 */
		static void Terminate();
		/**
		 * \brief Lookup the ID of a given process name
		 */
		static DWORD lookupProgramID( const char process[] );
		/**
		 * \brief Checks if the Interface is up and running
		 */
		static bool IsAvailable();

		/**
		 * \brief Starts an Interface session
		 *
		 * Used to start executing commands
		 */
		static void StartSession();
		/**
		 * \brief Ends an Interface session
		 *
		 * Must be called when finished executing commands
		 */
		static void EndSession();

		/**
		 * \brief Defines an Interface command
		 *
		 * name is the string which will be associated with this command; it must be known to the API
		 * def is the function template definition with n-placeholders
		 * real is an optional string to specifiy that this command is an alias; in this case, real is the name of an already existing command and name is the alias
		 *
		 * Example: DefineCommand("GetPos", "%0.GetPos %1");
		 */
		static void DefineCommand( string name, string def, string real = string() );
		/**
		 * \brief Defines a native for an existing command
		 *
		 * name refers to an existing command.
		 * param is a ParamContainer which has the form pair<ParamList, RetrieveBooleanFlag>
		 * ParamList is a STL list of Parameter's, RetrieveBooleanFlag is a function pointer which takes no arguments and returns a bool
		 */
		static void DefineNative( string name, ParamContainer param );
		/**
		 * \brief Executes a command in an indefinite loop until Terminate gets called
		 *
		 * name refers to an existing command.
		 * priority (optional) - the lower this variable, the higher is the priority
		 *
		 * A native must be defined for the given command.
		 */
		static void ExecuteCommandLoop( string name, unsigned int priority = 0 );
		/**
		 * \brief Executes a command once
		 *
		 * name refers to an existing command
		 * see DefineNative for a short explanation of ParamContainer
		 * priority (optional) - the lower this variable, the higher is the priority
		 * key (optional) - a signed key (usually from the Lockable class) which is to later identify this command
		 */
		static void ExecuteCommandOnce( string name, ParamContainer, unsigned int priority = 0, signed int key = 0 );

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler( Debug* debug );
#endif
};

#endif
