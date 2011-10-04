#ifndef TIMER_H
#define TIMER_H

#include <map>
#include <string>

#include "boost/any.hpp"

#include "ScriptFunction.h"
#include "Script.h"
#include "PAWN.h"
#include "../vaultmp.h"
#include "../Debug.h"
#include "../Network.h"
#include "../RakNet/gettimeofday.h"
#include "../RakNet/NetworkIDObject.h"

using namespace std;
using namespace RakNet;

/**
 * \brief Create timers to be used in scripts
 */

class Timer : public ScriptFunction, public NetworkIDObject
{
private:
    ~Timer();

    unsigned int ms;
    unsigned int interval;
    vector<boost::any> args;
    bool markdelete;

    static map<NetworkID, Timer*> timers;
    static unsigned int msecs();

public:

    Timer(ScriptFunc timer, string def, vector<boost::any> args, unsigned int interval);
    Timer(ScriptFuncPAWN timer, AMX* amx, string def, vector<boost::any> args, unsigned int interval);

    /**
     * \brief Called from the dedicated server main thread
     *
     * Calls timer functions
     */
    static void GlobalTick();
    /**
     * \brief Terminates a timer
     */
    static void Terminate(NetworkID id);
    /**
     * \brief Terminates all timers
     */
    static void TerminateAll();
};

#endif
