#include "Timer.h"

map<NetworkID, Timer*> Timer::timers;
NetworkID Timer::last_timer = 0;

Timer::Timer(ScriptFunc timer, string def, vector<boost::any> args, unsigned int interval) : markdelete(false), ms(msecs()), interval(interval), args(args), ScriptFunction(timer, def)
{
	this->SetNetworkIDManager(Network::Manager());
	timers.insert(pair<NetworkID, Timer*>(this->GetNetworkID(), this));
}

Timer::Timer(ScriptFuncPAWN timer, AMX* amx, string def, vector<boost::any> args, unsigned int interval) : markdelete(false), ms(msecs()), interval(interval), args(args), ScriptFunction(timer, amx, def)
{
	this->SetNetworkIDManager(Network::Manager());
	timers.insert(pair<NetworkID, Timer*>(this->GetNetworkID(), this));
}

Timer::~Timer()
{

}

unsigned int Timer::msecs()
{
	timeval t;
	gettimeofday(&t, nullptr);
	return (unsigned int)((t.tv_sec * 1000) + (t.tv_usec / 1000));
}

void Timer::GlobalTick()
{
	map<NetworkID, Timer*>::iterator it;

	for (it = timers.begin(); it != timers.end();)
	{
		Timer* timer = it->second;

		if (timer->markdelete)
		{
			timers.erase(it++);
			delete timer;
			continue;
		}

		if ((msecs() - timer->ms) > timer->interval)
		{
			last_timer = it->first;
			timer->Call(timer->args);
			timer->ms = msecs();
		}

		++it;
	}
}

NetworkID Timer::LastTimer()
{
	return last_timer;
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

	for (it = timers.begin(); it != timers.end(); timers.erase(it++))
	{
		Timer* timer = it->second;
		delete timer;
	}
}
