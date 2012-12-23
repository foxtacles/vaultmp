#ifndef TIMER_H
#define TIMER_H

#include <unordered_map>
#include <string>

#include "../RakNet.h"

#include "boost/any.hpp"

#include "ScriptFunction.h"
#include "Script.h"
#include "PAWN.h"
#include "../vaultmp.h"
#include "../Debug.h"
#include "../Network.h"

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

	public:
		Timer(ScriptFunc timer, const std::string& def, std::vector<boost::any> args, unsigned int interval);
		Timer(ScriptFuncPAWN timer, AMX* amx, const std::string& def, std::vector<boost::any> args, unsigned int interval);

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
