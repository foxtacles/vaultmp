#include "Interface.h"

using namespace std;

PipeClient* Interface::pipeServer;
PipeServer* Interface::pipeClient;
Interface::ResultHandler Interface::resultHandler;
atomic<bool> Interface::endThread;
atomic<bool> Interface::wakeup;
atomic<bool> Interface::shutdown;
bool Interface::initialized;
Interface::PriorityMap Interface::priorityMap;
Interface::StaticCommandList Interface::static_cmdlist;
Interface::DynamicCommandList Interface::dynamic_cmdlist;
Interface::JobList Interface::job_cmdlist;
Interface::Native Interface::natives;
thread Interface::hCommandThreadReceive;
thread Interface::hCommandThreadSend;
thread Interface::hCommandThreadJob;
CriticalSection Interface::static_cs;
CriticalSection Interface::dynamic_cs;
CriticalSection Interface::job_cs;

#ifdef VAULTMP_DEBUG
DebugInput<Interface> Interface::debug;
#endif

bool Interface::Initialize(ResultHandler resultHandler)
{
	if (!initialized)
	{
		endThread = false;
		wakeup = false;
		shutdown = false;

		Interface::resultHandler = resultHandler;

		pipeClient = new PipeServer();
		pipeServer = new PipeClient();

		hCommandThreadReceive = thread(CommandThreadReceive);
		hCommandThreadSend = thread(CommandThreadSend);
		hCommandThreadJob = thread(CommandThreadJob);

		if (!hCommandThreadReceive.joinable() || !hCommandThreadSend.joinable() || !hCommandThreadJob.joinable())
		{
			delete pipeServer;
			delete pipeClient;
			endThread = true;
			return false;
		}

		initialized = true;

#ifdef VAULTMP_DEBUG
		debug.print("Threads ", CriticalSection::thread_id(hCommandThreadReceive).c_str(), " ", CriticalSection::thread_id(hCommandThreadSend).c_str(), " ", hex, &static_cs, " ", &dynamic_cs);
#endif

		return true;
	}

	return false;
}

void Interface::Terminate()
{
	if (initialized)
	{
		endThread = true;

		if (hCommandThreadReceive.joinable())
			hCommandThreadReceive.join();

		if (hCommandThreadSend.joinable())
			hCommandThreadSend.join();

		if (hCommandThreadJob.joinable())
			hCommandThreadJob.join();

		static_cmdlist.clear();
		dynamic_cmdlist.clear();

		while (!job_cmdlist.empty())
			job_cmdlist.pop();

		priorityMap.clear();
		natives.clear();

		delete pipeServer;
		delete pipeClient;

		initialized = false;
	}
}

void Interface::SignalEnd()
{
	endThread = true;
	shutdown = true;
}

bool Interface::IsAvailable()
{
	return (wakeup && !endThread && hCommandThreadReceive.joinable() && hCommandThreadSend.joinable() && hCommandThreadJob.joinable());
}

bool Interface::HasShutdown()
{
	return shutdown;
}

void Interface::StartSetup()
{
	static_cs.StartSession();

	static_cmdlist.clear();
	priorityMap.clear();
}

void Interface::EndSetup()
{
	vector<unsigned int> priorities;
	priorities.reserve(priorityMap.size());

	for (const auto& priority : priorityMap)
		priorities.emplace_back(priority.first);

	auto it = unique(priorities.begin(), priorities.end());
	priorities.resize(it - priorities.begin());

	auto gcd = [](unsigned int x, unsigned int y)
	{
		for (;;)
		{
			if (x == 0)
				return y;

			y %= x;

			if (y == 0)
				return x;

			x %= y;
		}
	};

	auto lcm = [=](unsigned int x, unsigned int y)
	{
		unsigned int temp = gcd(x, y);
		return temp ? (x / temp * y) : 0;
	};

	unsigned int result = accumulate(priorities.begin(), priorities.end(), 1, lcm);

	for (unsigned int i = 0; i < result; ++i)
	{
		vector<Native::iterator> content;

		for (const auto& priority : priorityMap)
			if (((i + 1) % priority.first) == 0)
				content.emplace_back(priority.second);

		static_cmdlist.emplace_back(move(content));
	}

	static_cs.EndSession();
}

void Interface::StartDynamic()
{
	dynamic_cs.StartSession();
}

void Interface::EndDynamic()
{
	dynamic_cs.EndSession();
}

void Interface::SetupCommand(const string& name, ParamContainer&& param, unsigned int priority)
{
	priorityMap.insert(make_pair(priority, natives.emplace(name, move(param))));
}

void Interface::ExecuteCommand(const string& name, ParamContainer&& param, unsigned int key)
{
	dynamic_cmdlist.emplace_back(natives.emplace(make_pair(name, move(param))), key);
}

void Interface::PushJob(chrono::steady_clock::time_point&& T, function<void()>&& F)
{
	job_cs.StartSession();
	job_cmdlist.emplace(move(T), move(F));
	job_cs.EndSession();
}

vector<string> Interface::Evaluate(Native::iterator _it)
{
	const string& name = _it->first;
	ParamContainer& param = _it->second;

	unsigned int i = 0;
	unsigned int rsize = 1;
	unsigned int lsize = param.size();

	vector<unsigned int> mult;
	vector<string> result;
	mult.reserve(lsize);
	result.reserve(lsize);

	for (i = lsize; i != 0; --i)
	{
		param[i - 1].reset(); // reset to initial state (important for functors)
		const vector<string>& params = param[i - 1].get();

		if (params.empty())
			return result;

		mult.emplace(mult.begin(), rsize);
		rsize *= params.size();
	}

	for (i = 0; i < rsize; ++i)
	{
		string cmd = name;

		for (unsigned int j = 0; j < lsize; ++j)
		{
			unsigned int idx = static_cast<unsigned int>(i / mult[j]) % param[j].get().size();
			cmd += " " + Utils::str_replace(param[j].get()[idx], " ", "|");
		}

		result.emplace_back(move(cmd));
	}

	return result;
}

void Interface::CommandThreadReceive()
{
	try
	{
		pipeClient->SetPipeAttributes("BethesdaClient", PIPE_LENGTH);
		pipeClient->CreateServer();
		pipeClient->ConnectToServer();

		pipeServer->SetPipeAttributes("BethesdaServer", PIPE_LENGTH);

		while (!pipeServer->ConnectToServer() && !endThread);

		unsigned char buffer[PIPE_LENGTH];

		if (!endThread)
		{
			unsigned char code;

			while (!endThread)
			{
				ZeroMemory(buffer, sizeof(buffer));

				pipeClient->Receive(buffer);
				code = buffer[0];

				if (code == PIPE_OP_RETURN || code == PIPE_OP_RETURN_BIG || code == PIPE_OP_RETURN_RAW)
				{
					vector<CommandResult> result = API::Translate(buffer);

					for (CommandResult& _result : result)
					{
						resultHandler(get<0>(_result), get<1>(_result), get<2>(_result), get<3>(_result));

						if (endThread)
							break;
					}
				}
				else if (code == PIPE_SYS_WAKEUP)
				{
					wakeup = true;

#ifdef VAULTMP_DEBUG
					debug.print("vaultmp process waked up (game patched)");
#endif
				}
				else if (code == PIPE_ERROR_CLOSE)
				{
					if (!endThread)
						throw VaultException("Error in vaultmp.dll").stacktrace();
				}
				else if (code)
					throw VaultException("Unknown pipe code identifier %02X", code).stacktrace();
				else
					endThread = true;
			}
		}
	}
	catch (exception& e)
	{
		try
		{
			VaultException& vaulterror = dynamic_cast<VaultException&>(e);
			vaulterror.Message();
		}
		catch (bad_cast&)
		{
			VaultException vaulterror(e.what());
			vaulterror.Message();
		}

#ifdef VAULTMP_DEBUG
		debug.print("Receive thread is going to terminate (ERROR)");
#endif
	}

	endThread = true;
}

void Interface::CommandThreadSend()
{
	try
	{
		while (!wakeup && !endThread)
			this_thread::sleep_for(chrono::milliseconds(10));

		while (!endThread)
		{
			static_cs.StartSession();

			for (auto it = static_cmdlist.begin(); (it != static_cmdlist.end() || !dynamic_cmdlist.empty()) && !endThread;)
			{
				if (it != static_cmdlist.end())
				{
					const auto& next_list = *it;

					for (auto it = next_list.begin(); it != next_list.end() && !endThread; ++it)
					{
						vector<string> cmd = Interface::Evaluate(*it);

						if (!cmd.empty())
						{
							CommandParsed stream = API::Translate(cmd);

							for (auto it = stream.begin(); it != stream.end() && !endThread; ++it)
								pipeServer->Send(it->get());
						}
					}

					++it;
				}

				dynamic_cs.StartSession();

				for (; !dynamic_cmdlist.empty() && !endThread; natives.erase(dynamic_cmdlist.front().first), dynamic_cmdlist.pop_front())
				{
					dynamic_cs.EndSession();

					const auto& dynamic = dynamic_cmdlist.front();

					vector<string> cmd = Interface::Evaluate(dynamic.first);

					if (!cmd.empty())
					{
						CommandParsed stream = API::Translate(cmd, dynamic.second);

						for (auto it = stream.begin(); it != stream.end() && !endThread; ++it)
							pipeServer->Send(it->get());
					}

					dynamic_cs.StartSession();
				}

				dynamic_cs.EndSession();

				this_thread::sleep_for(chrono::milliseconds(1));
			}

			static_cs.EndSession();
		}
	}
	catch (exception& e)
	{
		try
		{
			VaultException& vaulterror = dynamic_cast<VaultException&>(e);
			vaulterror.Message();
		}
		catch (bad_cast&)
		{
			VaultException vaulterror(e.what());
			vaulterror.Message();
		}

#ifdef VAULTMP_DEBUG
		debug.print("Send thread is going to terminate (ERROR)");
#endif
	}

	if (wakeup)
	{
		unsigned char buffer[PIPE_LENGTH];
		buffer[0] = PIPE_ERROR_CLOSE;
		pipeServer->Send(buffer);
	}

	endThread = true;
}

void Interface::CommandThreadJob()
{
	try
	{
		while (!endThread)
		{
			function<void()> F;

			job_cs.StartSession();

			if (!job_cmdlist.empty())
			{
				const Job& job = job_cmdlist.top();

				if (job.T <= chrono::steady_clock::now())
				{
					F = job.F;
					job_cmdlist.pop();
				}
			}

			job_cs.EndSession();

			if (F)
				F();

			this_thread::sleep_for(chrono::milliseconds(1));
		}
	}
	catch (exception& e)
	{
		try
		{
			VaultException& vaulterror = dynamic_cast<VaultException&>(e);
			vaulterror.Message();
		}
		catch (bad_cast&)
		{
			VaultException vaulterror(e.what());
			vaulterror.Message();
		}

#ifdef VAULTMP_DEBUG
		debug.print("Job thread is going to terminate (ERROR)");
#endif
	}

	endThread = true;
}
