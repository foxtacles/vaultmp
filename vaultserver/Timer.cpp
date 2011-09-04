#include "Timer.h"

map<NetworkID, Timer*> Timer::timers;

#ifdef VAULTMP_DEBUG
Debug* Timer::debug = NULL;
#endif

#ifdef VAULTMP_DEBUG
void Timer::SetDebugHandler(Debug* debug)
{
    Timer::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Timer class", true);
}
#endif

Timer::Timer(TimerFunc timer, unsigned int interval) : timer(timer), interval(interval)
{
    this->SetNetworkIDManager(Network::Manager());
    this->ms = msecs();
    this->markdelete = false;
    this->pawn = false;
    timers.insert(pair<NetworkID, Timer*>(this->GetNetworkID(), this));
}

Timer::Timer(TimerPAWN timer, AMX* amx, unsigned int interval) : pawnc(timer), amx(amx), interval(interval)
{
    if (!amx)
        throw VaultException("Timer: AMX pointer is NULL");

    this->SetNetworkIDManager(Network::Manager());
    this->ms = msecs();
    this->markdelete = false;
    this->pawn = true;
    timers.insert(pair<NetworkID, Timer*>(this->GetNetworkID(), this));
}

Timer::~Timer()
{

}

unsigned int Timer::msecs()
{
    timeval t;
    gettimeofday(&t, NULL);
    return (unsigned int) ((t.tv_sec * 1000) + (t.tv_usec / 1000));
}

void Timer::GlobalTick()
{
    map<NetworkID, Timer*>::iterator it;

    for (it = timers.begin(); it != timers.end();)
    {
        Timer* timer = it->second;

        if (timer->markdelete)
        {
            it = timers.erase(it);
            delete timer;
            continue;
        }

        if ((msecs() - timer->ms) > timer->interval)
        {
            if (timer->pawn)
                PAWN::Call(timer->amx, timer->pawnc.c_str(), "", 0);
            else
                timer->timer();

            timer->ms = msecs();
        }

        ++it;
    }
}

void Timer::Terminate(NetworkID id)
{
    Timer* timer = Network::Manager()->GET_OBJECT_FROM_ID<Timer*>(id);

    if (timer)
        timer->markdelete = true;
}

void Timer::TerminateAll()
{
    map<NetworkID, Timer*>::iterator it;

    for (it = timers.begin(); it != timers.end(); it = timers.erase(it))
    {
        Timer* timer = it->second;
        delete timer;
    }
}
