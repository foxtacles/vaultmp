#ifndef INTERFACE_H
#define INTERFACE_H

#include "vaultmp.hpp"
#include "API.hpp"
#include "VaultFunctor.hpp"
#include "CriticalSection.hpp"
#include "Utils.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

#include <map>
#include <queue>
#include <thread>
#include <atomic>

class PipeClient;
class PipeServer;

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

				static std::vector<std::string> make(signed int str) { return std::vector<std::string>{Utils::toString(str)}; }
				static std::vector<std::string> make(unsigned int str) { return std::vector<std::string>{Utils::toString(str)}; }
				static std::vector<std::string> make(unsigned long long str) { return std::vector<std::string>{Utils::toString(str)}; }
				static std::vector<std::string> make(double str) { return std::vector<std::string>{Utils::toString(str)}; }
				static std::vector<std::string> make(bool str) { return std::vector<std::string>{str ? "1" : "0"}; }

			public:
				RawParameter(const std::string& str) : data({str}) {}
				RawParameter(const std::vector<std::string>& str) : data(str) {}
				RawParameter(const std::vector<unsigned char>& str) : data(make(str)) {}
				RawParameter(const std::vector<unsigned int>& str) : data(make(str)) {}
				RawParameter(signed int str) : data(make(str)) {}
				RawParameter(unsigned int str) : data(make(str)) {}
				RawParameter(unsigned long long str) : data(make(str)) {}
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

				const FuncParameter& connect(FuncParameter&& param) const
				{
					func.get()->connect(param.func.release());
					return *this;
				}

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

		struct Job {
			std::chrono::steady_clock::time_point T;
			std::function<void()> F;

			Job(std::chrono::steady_clock::time_point&& T, std::function<void()>&& F) : T(move(T)), F(move(F)) {}
			bool operator<(const Job& job) const { return T < job.T; }
		};

		typedef std::vector<Parameter> ParamContainer;
		typedef void (*ResultHandler)(unsigned int, const std::vector<double>&, double, bool);

	private:
		typedef std::unordered_multimap<Values::Func, ParamContainer, _hash_Func> Native;
		typedef std::multimap<unsigned int, Native::iterator> PriorityMap;
		typedef std::vector<std::vector<Native::iterator>> StaticCommandList;
		typedef std::deque<std::pair<Native::iterator, unsigned int>> DynamicCommandList;
		typedef std::priority_queue<Job> JobList;

		static std::atomic<bool> endThread;
		static std::atomic<bool> wakeup;
		static std::atomic<bool> shutdown;
		static bool initialized;
		static std::thread hCommandThreadReceive;
		static std::thread hCommandThreadSend;
		static std::thread hCommandThreadJob;
		static PriorityMap priorityMap;
		static StaticCommandList static_cmdlist;
		static DynamicCommandList dynamic_cmdlist;
		static JobList job_cmdlist;
		static PipeClient* pipeServer;
		static PipeServer* pipeClient;
		static ResultHandler resultHandler;
		static CriticalSection static_cs;
		static CriticalSection dynamic_cs;
		static CriticalSection job_cs;
		static Native natives;

		static void StartSetup_();
		static void EndSetup_();
		static API::CommandInput Evaluate(Native::iterator _it);

		static void CommandThreadReceive();
		static void CommandThreadSend();
		static void CommandThreadJob();

#ifdef VAULTMP_DEBUG
		static DebugInput<Interface> debug;
#endif

		Interface() = delete;

	public:
		/**
		 * \brief Initializes the Interface
		 *
		 * Takes a ResultHandler function pointer
		 */
		static bool Initialize(ResultHandler);
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
		 * \brief Executes Interface setup
		 *
		 * Used to start input static commands
		 */
		template<typename F>
		static void Setup(F function) {
			CriticalLock lock(static_cs);
			StartSetup_();
			function();
			EndSetup_();
		}

		/**
		 * \brief Executes Interface commands
		 *
		 * Used to start input dynamic commands
		 */
		template<typename F>
		static void Dynamic(F function) {
			CriticalLock lock(dynamic_cs);
			function();
		}

		/**
		 * \brief Setups a command
		 */
		static void SetupCommand(Values::Func opcode, ParamContainer&&, unsigned int priority = 1);
		/**
		 * \brief Executes a command once
		 */
		static void ExecuteCommand(Values::Func opcode, ParamContainer&&, unsigned int key = 0);
		/**
		 * \brief Pushes a job
		 */
		static void PushJob(std::chrono::steady_clock::time_point&& T, std::function<void()>&& F);
};

using RawParameter = Interface::RawParameter;
using FuncParameter = Interface::FuncParameter;
using ParamContainer = Interface::ParamContainer;

#endif
