#ifndef TIMER_H
#define TIMER_H

#include <map>
#include <string>

typedef void (*TimerFunc)();
typedef std::string TimerPAWN;

#include "PAWN.h"
#include "../vaultmp.h"
#include "../Debug.h"
#include "../Network.h"
#include "../RakNet/gettimeofday.h"
#include "../RakNet/NetworkIDObject.h"

using namespace RakNet;

/**
 * \brief Create timers to be used in scripts
 */

class Timer : public NetworkIDObject
{
private:

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    ~Timer();

    unsigned int ms;
    unsigned int interval;
    bool pawn;
    bool markdelete;
    TimerFunc timer;
    TimerPAWN pawnc;
    AMX* amx;

    static map<NetworkID, Timer*> timers;
    static unsigned int msecs();

public:

    Timer(TimerFunc timer, unsigned int interval);
    Timer(TimerPAWN timer, AMX* amx, unsigned int interval);

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

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif
};

#endif
