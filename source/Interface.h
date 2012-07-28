#ifndef INTERFACE_H
#define INTERFACE_H

#include "vaultmp.h"

#ifndef VAULTSERVER
#include <winsock2.h>
#include <tlhelp32.h>
#endif

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

#ifndef VAULTSERVER
#include "Pipe.h"
#else
class PipeServer;
class PipeClient;
#endif

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;
using namespace Data;

/**
 * \brief Provides facilities to execute engine commands, connects with the game process and is responsible for sending / retrieving game data
 *
 * Uses the API to translate commands and command results
 */

class Interface : public API
{
	private:
		class _Parameter {

			protected:
				_Parameter() = default;
				_Parameter(const _Parameter&) = default;
				_Parameter(_Parameter&&) = default;
				_Parameter& operator= (_Parameter&&) = default;

			public:
				virtual ~_Parameter() {}

				virtual const vector<string>& get() const = 0;
				virtual void reset() const = 0;
		};

	public:
		class RawParameter : public _Parameter {

			private:
				vector<string> data;

				static vector<string> make(const vector<unsigned char>& str)
				{
					vector<string> convert;

					for (unsigned char param : str)
						convert.emplace_back(Utils::toString(param));

					return convert;
				}

				static vector<string> make(signed int str)
				{
					return vector<string>{Utils::toString(str)};
				}

				static vector<string> make(unsigned int str)
				{
					return vector<string>{Utils::toString(str)};
				}

				static vector<string> make(double str)
				{
					return vector<string>{Utils::toString(str)};
				}

				static vector<string> make(bool str)
				{
					return vector<string>{str ? "1" : "0"};
				}

			public:
				RawParameter(string str) : data({str}) {}
				RawParameter(const vector<string>& str) : data(str) {}
				RawParameter(const vector<unsigned char>& str) : data(make(str)) {}
				RawParameter(signed int str) : data(make(str)) {}
				RawParameter(unsigned int str) : data(make(str)) {}
				RawParameter(double str) : data(make(str)) {}
				RawParameter(bool str) : data(make(str)) {}
				RawParameter(const RawParameter&) = default;
				RawParameter(RawParameter&&) = default;
				RawParameter& operator= (RawParameter&&) = default;
				virtual ~RawParameter() {}

				virtual const vector<string>& get() const { return data; }
				virtual void reset() const {}
		};

		class FuncParameter : public _Parameter {

			private:
				unique_ptr<VaultFunctor> func;
				mutable vector<string> data;
				mutable bool initialized;

			public:
				FuncParameter(unique_ptr<VaultFunctor>&& func) : func(move(func)), initialized(false) {}
				FuncParameter(FuncParameter&&) = default;
				FuncParameter& operator= (FuncParameter&&) = default;
				virtual ~FuncParameter() {}

				virtual const vector<string>& get() const
				{
					if (!initialized)
					{
						data = (*func.get())();
						initialized = true;
					}

					return data;
				}

				virtual void reset() const { initialized = false; }
		};

		class Parameter {

			private:
				unique_ptr<const _Parameter, void(*)(const _Parameter*)> param;

				static void no_delete(const _Parameter* param) {}
				static void def_delete(const _Parameter* param) { delete param; }

			public:
				Parameter(RawParameter& param) : param(new RawParameter(param), def_delete) {}
				Parameter(RawParameter&& param) : param(new RawParameter(move(param)), def_delete) {}
				Parameter(FuncParameter&& param) : param(new FuncParameter(move(param)), def_delete) {}
				Parameter(const RawParameter& param) : param(&param, no_delete) {}
				Parameter(const FuncParameter& param) : param(&param, no_delete) {}

				Parameter(Parameter&&) = default;
				Parameter& operator= (Parameter&&) = default;

				// hack: initializer lists reference static memory... this ctor enables moving the unique_ptr
				// NOT A COPY CTOR
				Parameter(const Parameter& param) : Parameter(move(const_cast<Parameter&>(param))) {}

				~Parameter() = default;

				const vector<string>& get() const { return param->get(); }
				void reset() { param->reset(); }
		};

		typedef vector<Parameter> ParamContainer;
		typedef void (*ResultHandler)(unsigned int, const vector<double>&, double, bool);

	private:
		typedef unordered_multimap<string, ParamContainer> Native;
		typedef multimap<unsigned int, Native::iterator> PriorityMap;
		typedef vector<vector<Native::iterator>> StaticCommandList;
		typedef deque<pair<Native::iterator, unsigned int>> DynamicCommandList;

		static atomic<bool> endThread;
		static atomic<bool> wakeup;
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

		static void CommandThreadReceive(bool steam);
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
		static bool Initialize(ResultHandler, bool steam);
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
		 * key (optional) - a key (usually from the Lockable class) which is to later identify this command
		 */
		static void ExecuteCommand(string name, ParamContainer&&, unsigned int key = 0);

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif
};

using RawParameter = Interface::RawParameter;
using FuncParameter = Interface::FuncParameter;
using ParamContainer = Interface::ParamContainer;

#endif
