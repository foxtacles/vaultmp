#ifndef TIMER_H
#define TIMER_H

#include "vaultserver.hpp"
#include "ScriptFunction.hpp"
#include "RakNet.hpp"

#include <unordered_map>

/**
 * \brief Create timers to be used in scripts
 */

class Timer : public ScriptFunction, public RakNet::NetworkIDObject
{
	private:
		~Timer();

		unsigned int ms;
		unsigned int interval;
		std::vector<boost::any> args;
		bool markdelete;

		static std::unordered_map<RakNet::NetworkID, Timer*> timers;
		static RakNet::NetworkID last_timer;
		static unsigned int msecs();

		Timer(ScriptFunc timer, const std::string& def, std::vector<boost::any> args, unsigned int interval);
		Timer(ScriptFuncPAWN timer, AMX* amx, const std::string& def, std::vector<boost::any> args, unsigned int interval);

	public:
		template<typename... Args>
		static Timer* MakeTimer(Args&&... args) { return new Timer(std::forward<Args>(args)...); }

		/**
		 * \brief Called from the dedicated server main thread
		 *
		 * Calls timer functions
		 */
		static void GlobalTick();
		/**
		 * \brief Returns the NetworkID of the latest timer
		 */
		static RakNet::NetworkID LastTimer();
		/**
		 * \brief Terminates a timer
		 */
		static void Terminate(RakNet::NetworkID id);
		/**
		 * \brief Terminates all timers
		 */
		static void TerminateAll();
};

#endif
