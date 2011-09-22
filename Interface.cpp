#include "Interface.h"

PipeClient* Interface::pipeServer;
PipeServer* Interface::pipeClient;
ResultHandler Interface::resultHandler;
bool Interface::endThread = false;
bool Interface::wakeup = false;
bool Interface::initialized = false;
char* Interface::module;
CommandList Interface::cmdlist;
CommandList Interface::tmplist;
map<string, string> Interface::defs;
map<string, string> Interface::alias;
Native Interface::natives;
HANDLE Interface::hCommandThreadReceive;
HANDLE Interface::hCommandThreadSend;
CriticalSection Interface::cs;

#ifdef VAULTMP_DEBUG
Debug* Interface::debug;
#endif

bool Interface::Initialize(char* module, ResultHandler resultHandler, unsigned char game)
{
    if (!initialized)
    {
        if (!strlen(module))
            return NULL;

        endThread = false;
        wakeup = false;

        if (pipeServer != NULL)
            delete pipeServer;
        if (pipeClient != NULL)
            delete pipeClient;

        Interface::module = new char[strlen(module)];
        strcpy(Interface::module, module);

        Interface::resultHandler = resultHandler;

        pipeClient = new PipeServer();
        pipeServer = new PipeClient();

        hCommandThreadReceive = CreateThread(NULL, 0, CommandThreadReceive, (LPVOID) Interface::module, 0, NULL);
        hCommandThreadSend = CreateThread(NULL, 0, CommandThreadSend, (LPVOID) 0, 0, NULL);

        if (hCommandThreadReceive == NULL || hCommandThreadSend == NULL)
        {
            endThread = true;
            return false;
        }

        initialized = true;

        return true;
    }

    return false;
}

void Interface::Terminate()
{
    if (initialized)
    {
        endThread = true;

        HANDLE threads[2];
        threads[0] = hCommandThreadReceive;
        threads[1] = hCommandThreadSend;

        WaitForMultipleObjects(2, threads, TRUE, INFINITE);

        CloseHandle(hCommandThreadReceive);
        CloseHandle(hCommandThreadSend);

        CommandList::iterator it;
        cmdlist.splice(cmdlist.begin(), tmplist);

        for (it = cmdlist.begin(); it != cmdlist.end(); ++it)
            FreeContainer(it->first->second);

        cmdlist.clear();
        tmplist.clear();
        natives.clear();
        defs.clear();
        alias.clear();

        initialized = false;

        delete[] module;
    }
}

#ifdef VAULTMP_DEBUG
void Interface::SetDebugHandler(Debug* debug)
{
    Interface::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Interface class", true);
}
#endif

DWORD Interface::lookupProgramID(const char process[])
{
    HANDLE hSnapshot;
    PROCESSENTRY32 ProcessEntry;
    ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(hSnapshot, &ProcessEntry))
        do
        {
            if (!strcmp(ProcessEntry.szExeFile, process))
            {
                CloseHandle(hSnapshot);
                return ProcessEntry.th32ProcessID;
            }
        }
        while(Process32Next(hSnapshot, &ProcessEntry));

    CloseHandle(hSnapshot);

    return 0;
}

bool Interface::IsAvailable()
{
    return (wakeup && !endThread && (WaitForSingleObject(hCommandThreadReceive, 0) == WAIT_TIMEOUT) && (WaitForSingleObject(hCommandThreadSend, 0) == WAIT_TIMEOUT));
}

void Interface::StartSession()
{
    cs.StartSession();
}

void Interface::EndSession()
{
    cs.EndSession();
}

void Interface::DefineCommand(string name, string def, string real)
{
    if (API::AnnounceFunction(real.empty() ? name : real) == false)
        throw VaultException("Function definition %s not found", (real.empty() ? name : real).c_str());

    if (!real.empty())
        alias.insert(pair<string, string>(name, real));

    defs.insert(pair<string, string>(name, def));
}

void Interface::DefineNative(string name, ParamContainer param)
{
    DefineNativeInternal(name, param);
}

Native::iterator Interface::DefineNativeInternal(string name, ParamContainer param)
{
    map<string, string>::iterator it;
    it = defs.find(name);

    if (it == defs.end())
        throw VaultException("Function definition %s not found", name.c_str());

    return natives.insert(pair<string, ParamContainer>(name, param));
}

void Interface::ExecuteCommand(Native::iterator it, bool loop, int priority, int sleep, signed int key)
{
    if (it == natives.end())
        throw VaultException("Native definition not found");

    vector<int> data;
    data.push_back((int) loop);
    data.push_back(sleep);
    data.push_back(priority);
    data.push_back(key);

    PUSHCMD((pair<Native::iterator, vector<int> >(it, data))); // Critical section
}

void Interface::ExecuteCommandLoop(string name, int priority, int sleep)
{
    map<string, string>::iterator it;
    it = defs.find(name);

    if (it == defs.end())
        throw VaultException("Command definition %s not found", name.c_str());

    ExecuteCommand(natives.find(name), true, priority, sleep, 0);
}

void Interface::ExecuteCommandOnce(string name, ParamContainer param, int priority, int sleep, signed int key)
{
    ExecuteCommand(DefineNativeInternal(name, param), false, priority, sleep, key);
}

multimap<string, string> Interface::Evaluate(string name, string def, ParamContainer param)
{
    multimap<string, string> result;
    ParamList::reverse_iterator it;
    RetrieveBooleanFlag performCheck = param.second;

    if (!param.first.empty())
    {
        if (performCheck())
        {
            int i = 0;
            int rsize = 1;
            int lsize = param.first.size();
            vector<int> mult;
            vector<ParamList::reverse_iterator> lists;
            mult.reserve(lsize);
            lists.reserve(lsize);

            for (it = param.first.rbegin(); it != param.first.rend(); ++it, i++)
            {
                char token[4];
                snprintf(token, sizeof(token), "%%%d", i);
                if (def.find(token) == string::npos)
                    return result;

                vector<string> params;
                VaultFunctor* getParams = it->second;
                if (getParams)
                    params = (*getParams)();

                if (!params.empty())
                    it->first.insert(it->first.end(), params.begin(), params.end());
                else if (it->first.empty())
                    return result;

                mult.insert(mult.begin(), rsize);
                lists.insert(lists.begin(), it);
                rsize *= it->first.size();
            }

            for (i = 0; i < rsize; i++)
            {
                string cmd = def;

                for (int j = 0; j < lsize; j++)
                {
                    int idx = ((int) (i / mult[j])) % lists.at(j)->first.size();

                    char token[4];
                    snprintf(token, sizeof(token), "%%%d", j);

                    cmd.replace(cmd.find(token), strlen(token), lists.at(j)->first.at(idx));
                }

                result.insert(pair<string, string>(name, cmd));
            }
        }
    }

    return result;
}

DWORD WINAPI Interface::CommandThreadReceive(LPVOID data)
{
    try
    {
        pipeClient->SetPipeAttributes("BethesdaClient", PIPE_LENGTH);
        pipeClient->CreateServer();
        pipeClient->ConnectToServer();

        pipeServer->SetPipeAttributes("BethesdaServer", PIPE_LENGTH);
        while (!pipeServer->ConnectToServer() && !endThread);

        char buffer[PIPE_LENGTH];
        char code;

        if (!endThread)
        {
            do
            {
                ZeroMemory(buffer, sizeof(buffer));

                pipeClient->Receive(buffer);
                code = buffer[0];
                char* content = buffer + 1;

                if (code == PIPE_OP_RETURN || code == PIPE_OP_RETURN_BIG)
                {
                    vector<CommandResult> result = API::Translate(buffer);
                    vector<CommandResult>::iterator it;

                    for (it = result.begin(); it != result.end(); ++it)
                        resultHandler(it->first.first.first, it->first.first.second, it->first.second, it->second);
                }
                else if (code == PIPE_SYS_WAKEUP)
                {
                    wakeup = true;

#ifdef VAULTMP_DEBUG
                    if (debug != NULL)
                        debug->Print("vaultmp process waked up (game patched)", true);
#endif
                }
                else if (code)
                    throw VaultException("Unknown pipe code identifier %02X", code);

                if (lookupProgramID((char*) data) == 0)
                {
                    endThread = true;

#ifdef VAULTMP_DEBUG
                    if (debug != NULL)
                        debug->Print("Game process missing, shutting down", true);
#endif
                }
            }
            while (code != PIPE_ERROR_CLOSE && !endThread);
        }

        // kill game process if running
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
            debug->Print("Receive thread is going to terminate (ERROR)", true);
#endif

        endThread = true;

        return ((DWORD) data);
    }

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->Print("Receive thread is going to terminate", true);
#endif

    endThread = true;

    return ((DWORD) data);
}

DWORD WINAPI Interface::CommandThreadSend(LPVOID data)
{
    try
    {
        while (!wakeup && !endThread);

        while (!endThread)
        {
            CommandList::iterator it;
            CommandList::iterator insertAt = cmdlist.end();

            if (!tmplist.empty())
            {
                cs.StartSession();

                cmdlist.splice(cmdlist.begin(), tmplist);

                cs.EndSession();
            }

            for (it = cmdlist.begin(); it != cmdlist.end() && !endThread;)
            {
                if (!tmplist.empty())
                {
                    cs.StartSession();

                    int count = tmplist.size();

                    if (insertAt != cmdlist.end())
                    {
                        CommandList::iterator insertAt_tmp = insertAt;
                        ++insertAt_tmp;
                        cmdlist.splice(insertAt_tmp, tmplist);
                    }
                    else
                    {
                        CommandList::iterator it_tmp = it;
                        ++it_tmp;
                        cmdlist.splice(it_tmp, tmplist);
                    }

                    CommandList::iterator it_tmp = it;
                    advance(it_tmp, count);

                    insertAt = it_tmp;

                    cs.EndSession();
                }

                if (insertAt == it)
                    insertAt = cmdlist.end();

                map<string, string>::iterator al = alias.find(it->first->first);
                string name = (al != alias.end() ? al->second : it->first->first);
                string def = defs.find(name)->second;
                ParamContainer& param = it->first->second;
                vector<int>& data = it->second;

                if (data.size() != 5)
                    data.push_back(data.at(2));

                if (data.at(4) != 0)
                {
                    data.at(4)--;
                    ++it;
                    continue;
                }
                else
                    data.at(4) = data.at(2);

                multimap<string, string> cmd = Interface::Evaluate(name, def, param);

                if (cmd.size() != 0)
                {
                    signed int key = data.at(3);

                    CommandParsed stream = API::Translate(cmd, key);

                    if (stream.size() != 0)
                    {
                        CommandParsed::iterator it2;

                        for (it2 = stream.begin(); it2 != stream.end() && !endThread; ++it2)
                        {
                            char* content = *it2;
                            pipeServer->Send(content);
                            Sleep(data.at(1));
                        }

                        for (it2 = stream.begin(); it2 != stream.end(); ++it2)
                        {
                            char* content = *it2;
                            delete[] content;
                        }
                    }
                }

                if (data.at(0) == 0)
                {
                    FreeContainer(param);
                    natives.erase(it->first);
                    it = cmdlist.erase(it);
                }
                else
                    ++it;
            }
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

        return ((DWORD) data);
    }

    return ((DWORD) data);
}
