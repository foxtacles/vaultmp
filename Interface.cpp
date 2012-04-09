#include "Interface.h"

PipeClient* Interface::pipeServer;
PipeServer* Interface::pipeClient;
ResultHandler Interface::resultHandler;
bool Interface::endThread = false;
bool Interface::wakeup = false;
bool Interface::initialized = false;
PriorityMap Interface::priorityMap;
StaticCommandList Interface::static_cmdlist;
DynamicCommandList Interface::dynamic_cmdlist;
unordered_map<string, string> Interface::defs;
unordered_map<string, string> Interface::alias;
Native Interface::natives;
thread Interface::hCommandThreadReceive;
thread Interface::hCommandThreadSend;
CriticalSection Interface::static_cs;
CriticalSection Interface::dynamic_cs;

#ifdef VAULTMP_DEBUG
Debug* Interface::debug;
#endif

bool Interface::Initialize( ResultHandler resultHandler, unsigned char game )
{
	if ( !initialized )
	{
		endThread = false;
		wakeup = false;

		if ( pipeServer != NULL )
			delete pipeServer;

		if ( pipeClient != NULL )
			delete pipeClient;

		Interface::resultHandler = resultHandler;

		pipeClient = new PipeServer();
		pipeServer = new PipeClient();

		hCommandThreadReceive = thread(CommandThreadReceive);
		hCommandThreadSend = thread(CommandThreadSend);

		if ( !hCommandThreadReceive.joinable() || !hCommandThreadSend.joinable() )
		{
			endThread = true;
			return false;
		}

		initialized = true;

#ifdef VAULTMP_DEBUG
        //static_cs.SetDebugHandler(debug);
        //dynamic_cs.SetDebugHandler(debug);
        //debug->PrintFormat("Threads %s %s %p %p", true, CriticalSection::thread_id(hCommandThreadReceive).c_str(), CriticalSection::thread_id(hCommandThreadSend).c_str(), &static_cs, &dynamic_cs);
#endif

		return true;
	}

	return false;
}

void Interface::Terminate()
{
	if ( initialized )
	{
		endThread = true;

        if (hCommandThreadReceive.joinable())
            hCommandThreadReceive.join();

        if (hCommandThreadSend.joinable())
            hCommandThreadSend.join();

        Native::iterator it;

		for ( it = natives.begin(); it != natives.end(); ++it )
			FreeContainer( it->second );

		static_cmdlist.clear();
		dynamic_cmdlist.clear();
		priorityMap.clear();

		natives.clear();
		defs.clear();
		alias.clear();

		initialized = false;
	}
}

#ifdef VAULTMP_DEBUG
void Interface::SetDebugHandler( Debug* debug )
{
	Interface::debug = debug;

	if ( debug != NULL )
		debug->Print( "Attached debug handler to Interface class", true );
}
#endif

bool Interface::IsAvailable()
{
	return ( wakeup && !endThread && hCommandThreadReceive.joinable() && hCommandThreadSend.joinable());
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

    PriorityMap::iterator it;

    for (it = priorityMap.begin(); it != priorityMap.end(); ++it)
        priorities.push_back(it->first);

    vector<unsigned int>::iterator it2 = unique(priorities.begin(), priorities.end());
    priorities.resize(it2 - priorities.begin());

    auto gcd = [](int x, int y) { for (;;) { if (x == 0) return y; y %= x; if (y == 0) return x; x %= y; } };
    auto lcm = [=](int x, int y) { int temp = gcd(x, y); return temp ? (x / temp * y) : 0; };

    unsigned int result = accumulate(priorities.begin(), priorities.end(), 1, lcm);

    for (unsigned int i = 0; i < result; ++i)
    {
        list<Native::iterator> content = list<Native::iterator>();

        for (it = priorityMap.begin(); it != priorityMap.end(); ++it)
            if (((i + 1) % it->first) == 0)
                content.push_back(it->second);

        static_cmdlist.push_back(content);
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

void Interface::DefineCommand( string name, string def, string real )
{
	if ( API::AnnounceFunction( real.empty() ? name : real ) == false )
		throw VaultException( "Function definition %s not found", ( real.empty() ? name : real ).c_str() );

	if ( !real.empty() )
		alias.insert( pair<string, string>( name, real ) );

	defs.insert( pair<string, string>( name, def ) );
}

void Interface::DefineNative( string name, ParamContainer param )
{
	DefineNativeInternal( name, param );
}

Native::iterator Interface::DefineNativeInternal( string name, ParamContainer param )
{
	unordered_map<string, string>::iterator it;
	it = defs.find( name );

	if ( it == defs.end() )
		throw VaultException( "Function definition for %s not found", name.c_str() );

	return natives.insert( pair<string, ParamContainer>( name, param ) );
}

void Interface::ExecuteCommand(Native::iterator it, signed int key)
{
	if ( it == natives.end() )
		throw VaultException( "Native definition not found" );

    dynamic_cmdlist.push_back((pair<Native::iterator, signed int>(it, key)));
}

void Interface::SetupCommand(string name, unsigned int priority)
{
    Native::iterator it = natives.find(name);

	if ( it == natives.end() )
		throw VaultException( "Native definition for %s not found", name.c_str() );

    priorityMap.insert(pair<unsigned int, Native::iterator>(priority, it));
}

void Interface::ExecuteCommand(string name, ParamContainer param, signed int key)
{
    ExecuteCommand(DefineNativeInternal(name, param), key);
}

multimap<string, string> Interface::Evaluate(Native::iterator _it)
{
    unordered_map<string, string>::iterator al = alias.find(_it->first);
    string name = (al != alias.end() ? al->second : _it->first);
    string def = defs.find(name)->second;
    ParamContainer param = _it->second; // copy...maybe change function to operate on reference

	multimap<string, string> result;
	ParamContainer::reverse_iterator it;

    if (!param.empty())
    {
        unsigned int i = 0;
        unsigned int rsize = 1;
        unsigned int lsize = param.size();
        vector<unsigned int> mult;
        vector<ParamContainer::reverse_iterator> lists;
        mult.reserve(lsize);
        lists.reserve(lsize);

        for (it = param.rbegin(); it != param.rend(); ++it, ++i)
        {
            char token[4];
            snprintf(token, sizeof(token), "%%%d", i);

            if ( def.find( token ) == string::npos )
                return result;

            vector<string> params;
            VaultFunctor* getParams = it->second;

            if ( getParams )
                params = ( *getParams )();

            if ( !params.empty() )
                it->first.insert( it->first.end(), params.begin(), params.end() );
            else if (it->first.empty())
                return result;

            mult.insert( mult.begin(), rsize );
            lists.insert( lists.begin(), it );
            rsize *= it->first.size();
        }

        for ( i = 0; i < rsize; ++i )
        {
            string cmd = def;

            for ( int j = 0; j < lsize; j++ )
            {
                unsigned int idx = ( ( int ) ( i / mult[j] ) ) % lists.at( j )->first.size();

                char token[4];
                snprintf( token, sizeof( token ), "%%%d", j );

                cmd.replace( cmd.find( token ), strlen( token ), lists.at( j )->first.at( idx ) );
            }

            result.insert( pair<string, string>( name, cmd ) );
        }
	}

	return result;
}

void Interface::CommandThreadReceive()
{
	try
	{
		pipeClient->SetPipeAttributes( "BethesdaClient", PIPE_LENGTH );
		pipeClient->CreateServer();
		pipeClient->ConnectToServer();

		pipeServer->SetPipeAttributes( "BethesdaServer", PIPE_LENGTH );

		while ( !pipeServer->ConnectToServer() && !endThread );

		unsigned char buffer[PIPE_LENGTH];
		unsigned char code;

		if ( !endThread )
		{
			do
			{
				ZeroMemory( buffer, sizeof( buffer ) );

				pipeClient->Receive( buffer );
				code = buffer[0];
				unsigned char* content = buffer + 1;

				if ( code == PIPE_OP_RETURN || code == PIPE_OP_RETURN_BIG )
				{
					vector<CommandResult> result = API::Translate( buffer );
					vector<CommandResult>::iterator it;

					for ( it = result.begin(); it != result.end(); ++it )
						resultHandler( it->first.first.first, it->first.first.second, it->first.second, it->second );
				}

				else if ( code == PIPE_SYS_WAKEUP )
				{
					wakeup = true;

#ifdef VAULTMP_DEBUG

					if ( debug != NULL )
						debug->Print( "vaultmp process waked up (game patched)", true );

#endif
				}

				else if ( code )
					throw VaultException( "Unknown pipe code identifier %02X", code );
			}
			while ( code && code != PIPE_ERROR_CLOSE && !endThread );
		}
	}

	catch ( std::exception& e )
	{
		try
		{
			VaultException& vaulterror = dynamic_cast<VaultException&>( e );
			vaulterror.Message();
		}

		catch ( std::bad_cast& no_vaulterror )
		{
			VaultException vaulterror( e.what() );
			vaulterror.Message();
		}

#ifdef VAULTMP_DEBUG

		if ( debug != NULL )
			debug->Print( "Receive thread is going to terminate (ERROR)", true );

#endif

		endThread = true;
	}

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->Print( "Receive thread is going to terminate", true );

#endif

	endThread = true;
}

void Interface::CommandThreadSend()
{
    try
    {
        while (!wakeup && !endThread)
            this_thread::sleep_for(chrono::milliseconds(10));

		while ( !endThread )
		{
			StaticCommandList::iterator it;

            static_cs.StartSession();

			for ( it = static_cmdlist.begin(); (it != static_cmdlist.end() || dynamic_cmdlist.size()) && !endThread; ++it)
			{
			    if (it != static_cmdlist.end())
			    {
                    list<Native::iterator>::iterator it2;
                    list<Native::iterator>& next_list = *it;

                    for (it2 = next_list.begin(); it2 != next_list.end() && !endThread; ++it2)
                    {
                        multimap<string, string> cmd = Interface::Evaluate(*it2);

                        if (cmd.size() != 0)
                        {
                            CommandParsed stream = API::Translate(cmd);

                            if (stream.size() != 0)
                            {
                                CommandParsed::iterator it;

                                for (it = stream.begin(); it != stream.end() && !endThread; ++it)
                                {
                                    unsigned char* content = *it;
                                    pipeServer->Send(content);
                                }

                                for (it = stream.begin(); it != stream.end(); ++it)
                                {
                                    unsigned char* content = *it;
                                    delete[] content;
                                }
                            }
                        }
                    }
			    }

                dynamic_cs.StartSession();

                DynamicCommandList::iterator it3;

                for (it3 = dynamic_cmdlist.begin(); it3 != dynamic_cmdlist.end() && !endThread; it3 = dynamic_cmdlist.erase(it3))
                {
                    multimap<string, string> cmd = Interface::Evaluate(it3->first);

                    if (cmd.size() != 0)
                    {
                        CommandParsed stream = API::Translate(cmd, it3->second);

                        if (stream.size() != 0)
                        {
                            CommandParsed::iterator it;

                            for (it = stream.begin(); it != stream.end() && !endThread; ++it)
                            {
                                unsigned char* content = *it;
                                pipeServer->Send(content);
                            }

                            for (it = stream.begin(); it != stream.end(); ++it)
                            {
                                unsigned char* content = *it;
                                delete[] content;
                            }
                        }
                    }

                    FreeContainer(it3->first->second);
                    natives.erase(it3->first);
                }

                dynamic_cs.EndSession();

                this_thread::sleep_for(chrono::milliseconds(1));
            }

            static_cs.EndSession();
        }
    }
    catch (std::exception& e)
    {
        try
        {
            VaultException& vaulterror = dynamic_cast<VaultException&>(e);
            vaulterror.Message();
        }
        catch (std::bad_cast& no_vaulterror)
        {
            VaultException vaulterror(e.what());
            vaulterror.Message();
        }

#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->Print("Send thread is going to terminate (ERROR)", true);
#endif
    }
}
