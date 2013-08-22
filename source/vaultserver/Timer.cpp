#include "Timer.hpp"
#include "Network.hpp"

using namespace std;
using namespace RakNet;

unordered_map<NetworkID, Timer*> Timer::timers;
NetworkID Timer::last_timer = 0;

Timer::Timer(ScriptFunc timer, const string& def, vector<boost::any> args, unsigned int interval) : ScriptFunction(timer, def), ms(msecs()), interval(interval), args(args), markdelete(false)
{
	this->SetNetworkIDManager(Network::Manager());
	timers.emplace(this->GetNetworkID(), this);
}

Timer::Timer(ScriptFuncPAWN timer, AMX* amx, const string& def, vector<boost::any> args, unsigned int interval) : ScriptFunction(timer, amx, def), ms(msecs()), interval(interval), args(args), markdelete(false)
{
	this->SetNetworkIDManager(Network::Manager());
	timers.emplace(this->GetNetworkID(), this);
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
	for (auto it = timers.begin(); it != timers.end();)
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
	for (auto it = timers.begin(); it != timers.end(); timers.erase(it++))
	{
		Timer* timer = it->second;
		delete timer;
	}
}
