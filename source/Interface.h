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

/**
 * \brief Provides facilities to execute engine commands, connects with the game process and is responsible for sending / retrieving game data
 *
 * Uses the API to translate commands and command results
 */

class Interface : public API
{
	private:
		class Parameter_ {

			protected:
				Parameter_() = default;
				Parameter_(const Parameter_&) = default;
				Parameter_(Parameter_&&) = default;
				Parameter_& operator= (Parameter_&&) = default;

			public:
				virtual ~Parameter_() {}

				virtual const std::vector<std::string>& get() const = 0;
				virtual void reset() const = 0;
		};

	public:
		class RawParameter : public Parameter_ {

			private:
				std::vector<std::string> data;

				static std::vector<std::string> make(const std::vector<unsigned char>& str)
				{
					std::vector<std::string> convert;

					for (unsigned char param : str)
						convert.emplace_back(Utils::toString(param));

					return convert;
				}

				static std::vector<std::string> make(const std::vector<unsigned int>& str)
				{
					std::vector<std::string> convert;

					for (unsigned int param : str)
						convert.emplace_back(Utils::toString(param));

					return convert;
				}

				static std::vector<std::string> make(signed int str)
				{
					return std::vector<std::string>{Utils::toString(str)};
				}

				static std::vector<std::string> make(unsigned int str)
				{
					return std::vector<std::string>{Utils::toString(str)};
				}

				static std::vector<std::string> make(double str)
				{
					return std::vector<std::string>{Utils::toString(str)};
				}

				static std::vector<std::string> make(bool str)
				{
					return std::vector<std::string>{str ? "1" : "0"};
				}

			public:
				RawParameter(const std::string& str) : data({str}) {}
				RawParameter(const std::vector<std::string>& str) : data(str) {}
				RawParameter(const std::vector<unsigned char>& str) : data(make(str)) {}
				RawParameter(const std::vector<unsigned int>& str) : data(make(str)) {}
				RawParameter(signed int str) : data(make(str)) {}
				RawParameter(unsigned int str) : data(make(str)) {}
				RawParameter(double str) : data(make(str)) {}
				RawParameter(bool str) : data(make(str)) {}
				RawParameter(const RawParameter&) = default;
				RawParameter(RawParameter&&) = default;
				RawParameter& operator= (RawParameter&&) = default;
				virtual ~RawParameter() {}

				virtual const std::vector<std::string>& get() const { return data; }
				virtual void reset() const {}
		};

		class FuncParameter : public Parameter_ {

			private:
				std::unique_ptr<VaultFunctor> func;
				mutable std::vector<std::string> data;
				mutable bool initialized;

			public:
				FuncParameter(std::unique_ptr<VaultFunctor>&& func) : func(move(func)), initialized(false) {}
				FuncParameter(FuncParameter&&) = default;
				FuncParameter& operator= (FuncParameter&&) = default;
				virtual ~FuncParameter() {}

				virtual const std::vector<std::string>& get() const
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
				std::unique_ptr<const Parameter_, void(*)(const Parameter_*)> param;

				static void no_delete(const Parameter_*) {}
				static void def_delete(const Parameter_* param) { delete param; }

			public:
				Parameter(RawParameter& param) : param(new RawParameter(param), def_delete) {}
				Parameter(RawParameter&& param) : param(new RawParameter(std::move(param)), def_delete) {}
				Parameter(FuncParameter&& param) : param(new FuncParameter(std::move(param)), def_delete) {}
				Parameter(const RawParameter& param) : param(&param, no_delete) {}
				Parameter(const FuncParameter& param) : param(&param, no_delete) {}

				Parameter(Parameter&&) = default;
				Parameter& operator= (Parameter&&) = default;

				// hack: initializer lists reference static memory... this ctor enables moving the unique_ptr
				// NOT A COPY CTOR
				Parameter(const Parameter& param) : Parameter(std::move(const_cast<Parameter&>(param))) {}

				~Parameter() = default;

				const std::vector<std::string>& get() const { return param->get(); }
				void reset() { param->reset(); }
		};

		typedef std::vector<Parameter> ParamContainer;
		typedef void (*ResultHandler)(unsigned int, const std::vector<double>&, double, bool);

	private:
		typedef std::unordered_multimap<std::string, ParamContainer> Native;
		typedef std::multimap<unsigned int, Native::iterator> PriorityMap;
		typedef std::vector<std::vector<Native::iterator>> StaticCommandList;
		typedef std::deque<std::pair<Native::iterator, unsigned int>> DynamicCommandList;

		static std::atomic<bool> endThread;
		static std::atomic<bool> wakeup;
		static std::atomic<bool> shutdown;
		static bool initialized;
		static std::thread hCommandThreadReceive;
		static std::thread hCommandThreadSend;
		static PriorityMap priorityMap;
		static StaticCommandList static_cmdlist;
		static DynamicCommandList dynamic_cmdlist;
		static PipeClient* pipeServer;
		static PipeServer* pipeClient;
		static ResultHandler resultHandler;
		static CriticalSection static_cs;
		static CriticalSection dynamic_cs;
		static Native natives;

		static std::vector<std::string> Evaluate(Native::iterator _it);

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
		 * \brief Signals the interface to shutdown
		 */
		static void SignalEnd();
		/**
		 * \brief Checks if the Interface is up and running
		 */
		static bool IsAvailable();
		/**
		 * \brief Returns true if the interface has been shutdown properly
		 */
		static bool HasShutdown();

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
		static void SetupCommand(const std::string& name, ParamContainer&& param, unsigned int priority = 1);
		/**
		 * \brief Executes a command once
		 *
		 * name refers to an existing command
		 * key (optional) - a key (usually from the Lockable class) which is to later identify this command
		 */
		static void ExecuteCommand(const std::string& name, ParamContainer&&, unsigned int key = 0);

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif
};

using RawParameter = Interface::RawParameter;
using FuncParameter = Interface::FuncParameter;
using ParamContainer = Interface::ParamContainer;

#endif
