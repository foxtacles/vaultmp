#include "Command.h"

using namespace std;
using namespace pipe;

PipeClient* Command::pipeServer;
PipeServer* Command::pipeClient;
ResultHandler Command::resultHandler;
StringHandler Command::stringHandler;
bool Command::endThread = false;
bool Command::wakeup = false;
bool Command::cmdmutex = false;
bool Command::cmdsession = false;
bool Command::initialized = false;
char* Command::module;
CommandList Command::cmdlist;
CommandList Command::tmplist;
map<string, string> Command::defs;
Native Command::natives;
HANDLE Command::hCommandThreadReceive;
HANDLE Command::hCommandThreadSend;
#ifdef VAULTMP_DEBUG
Debug* Command::debug = NULL;
#endif

Command::Command()
{

}

HANDLE* Command::Initialize(char* module, ResultHandler resultHandler, StringHandler stringHandler)
{
    if (!initialized)
    {
        if (!strlen(module))
            return NULL;

        endThread = false;
        wakeup = false;
        cmdmutex = false;
        cmdsession = false;

        if (pipeServer != NULL)
            delete pipeServer;
        if (pipeClient != NULL)
            delete pipeClient;

        Command::module = new char[strlen(module)];
        strcpy(Command::module, module);

        Command::resultHandler = resultHandler;
        Command::stringHandler = stringHandler;

        pipeClient = new PipeServer();
        pipeServer = new PipeClient();

        HANDLE* threads = new HANDLE[2];

        threads[0] = CreateThread(NULL, 0, CommandThreadReceive, (LPVOID) Command::module, 0, NULL);
        threads[1] = CreateThread(NULL, 0, CommandThreadSend, (LPVOID) 0, 0, NULL);

        if (threads[0] == NULL || threads[1] == NULL)
        {
            endThread = true;
            return NULL;
        }

        initialized = true;

        return threads;
    }

    return NULL;
}

bool Command::Terminate()
{
    if (initialized)
    {
        endThread = true;
        initialized = false;

        cmdlist.clear();
        tmplist.clear();
        natives.clear();
        defs.clear();

        delete[] module;
    }
}

#ifdef VAULTMP_DEBUG
void Command::SetDebugHandler(Debug* debug)
{
    Command::debug = debug;

    if (debug != NULL)
        debug->Print((char*) "Attached debug handler to Command class", true);
}
#endif

DWORD Command::lookupProgramID(const char process[])
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

bool Command::GetReady()
{
    return wakeup;
}

void Command::StartSession()
{
    STARTSESSION();
    OPENCMD();
}

void Command::EndSession()
{
    ENDSESSION();
    CLOSECMD();
}

void Command::DefineCommand(string name, string def)
{
    defs.insert(pair<string, string>(name, def));
}

void Command::DefineNative(string name, ParamContainer param)
{
    DefineNativeInternal(name, param);
}

Native::iterator Command::DefineNativeInternal(string name, ParamContainer param)
{
    map<string, string>::iterator it;
    it = defs.find(name);

    if (it == defs.end())
        return natives.end();

    return natives.insert(pair<string, ParamContainer>(name, param));
}

void Command::ExecuteCommand(Native::iterator it, bool loop, int priority, int sleep)
{
    if (it == natives.end())
        return;

    vector<int> data;
    data.push_back((int) loop);
    data.push_back(sleep);
    data.push_back(priority);

    while (!SESSION());

    PUSHCMD((pair<Native::iterator, vector<int> >(it, data)));
}

void Command::ExecuteCommandLoop(string name, int priority, int sleep)
{
    map<string, string>::iterator it;
    it = defs.find(name);

    if (it == defs.end())
        return;

    ExecuteCommand(natives.find(name), true, priority, sleep);
}

void Command::ExecuteCommandOnce(string name, ParamContainer param, int priority, int sleep)
{
    ExecuteCommand(DefineNativeInternal(name, param), false, priority, sleep);
}

list<string> Command::Eval(string name, string def, ParamContainer param)
{
    list<string> result;
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
                sprintf(token, "%%%d", i);
                if (def.find(token) == string::npos)
                    return result;

                RetrieveParamVector getParams = it->second;
                vector<string> params = getParams(name);

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
                string cmd = "op:" + def;

                for (int j = 0; j < lsize; j++)
                {
                    int idx = ((int) (i / mult[j])) % lists.at(j)->first.size();

                    char token[4];
                    sprintf(token, "%%%d", j);

                    cmd.replace(cmd.find(token), strlen(token), lists.at(j)->first.at(idx));
                }

                result.push_back(cmd);
            }
        }
    }

    return result;
}

DWORD WINAPI Command::CommandThreadReceive(LPVOID data)
{
    pipeClient->SetPipeAttributes("Fallout3client", 4096);
    pipeClient->CreateServer();
    pipeClient->ConnectToServer();

    pipeServer->SetPipeAttributes("Fallout3server", 4096);
    while (!pipeServer->ConnectToServer() && !endThread);

    string send;
    string recv;
    string low;
    string high;

    if (!endThread)
    {
        do
        {
            recv.clear();
            low.clear();
            high.clear();
            recv = pipeClient->Recv();

            int find = 0;

            find = recv.find("op:");
            if (find == string::npos) find = recv.find("st:");
            if (find == string::npos) find = recv.find("up:");
            if (find == string::npos) find = recv.find("ca:");

            if (find != string::npos)
            {
                low = recv.substr(find, 3);
                high = recv.substr(find + 3);

                if (low.compare("op:") == 0)
                {
                    unsigned long long Fallout3_result = 0x00;
                    DWORD Fallout3_opcode = 0x00;
                    DWORD Fallout3_refID = 0x00;
                    DWORD Fallout3_newRefID = 0x00;
                    unsigned char Fallout3_coord = 0x00;
                    unsigned char Fallout3_setcoord = 0x00;
                    unsigned char Fallout3_valcoord = 0x00;

                    char output[high.length()];
                    char* token;
                    strcpy(output, high.c_str());

                    Fallout3_opcode = strtoul(output, &token, 16);
                    Fallout3_refID = strtoul(token, &token, 16);
                    Fallout3_result = strtoull(token, &token, 16);
                    Fallout3_coord = (unsigned char) strtoul(token, &token, 16);
                    Fallout3_setcoord = (unsigned char) strtoul(token, &token, 16);
                    Fallout3_valcoord = (unsigned char) strtoul(token, &token, 16);
                    Fallout3_newRefID = strtoull(token, &token, 16);

                    vector<void*> result;
                    result.push_back(reinterpret_cast<void*>(&Fallout3_opcode));
                    result.push_back(reinterpret_cast<void*>(&Fallout3_refID));
                    result.push_back(reinterpret_cast<void*>(&Fallout3_result));
                    result.push_back(reinterpret_cast<void*>(&Fallout3_coord));
                    result.push_back(reinterpret_cast<void*>(&Fallout3_setcoord));
                    result.push_back(reinterpret_cast<void*>(&Fallout3_valcoord));
                    result.push_back(reinterpret_cast<void*>(&Fallout3_newRefID));

                    resultHandler(result);
                }
                else if (low.compare("st:") == 0)
                {
                    stringHandler(high);
                }
                else if (low.compare("up:") == 0)
                {
                    wakeup = true;

#ifdef VAULTMP_DEBUG
                    if (debug != NULL)
                        debug->Print((char*) "vaultmp process waked up (memory patches are done)", true);
#endif
                }
            }

            if (lookupProgramID((char*) data) == 0)
            {
                endThread = true;

#ifdef VAULTMP_DEBUG
                if (debug != NULL)
                    debug->Print((char*) "Game process missing, shutting down...", true);
#endif
            }
        }
        while (low.compare("ca:") != 0 && !endThread);
    }

    // kill game process if running

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->Print((char*) "Receive thread is going to terminate...", true);
#endif

    return ((DWORD) data);
}

DWORD WINAPI Command::CommandThreadSend(LPVOID data)
{
    while (!wakeup && !endThread);

    while (!endThread)
    {
        CommandList::iterator it;
        CommandList::iterator insertAt = cmdlist.end();

        if (!tmplist.empty())
        {
            OPENCMD();

            cmdlist.splice(cmdlist.begin(), tmplist);

            CLOSECMD();
        }

        for (it = cmdlist.begin(); it != cmdlist.end() && !endThread;)
        {
            if (!tmplist.empty())
            {
                OPENCMD();

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

                CLOSECMD();
            }

            if (insertAt == it)
                insertAt = cmdlist.end();

            string name = it->first->first;
            string def = defs.find(name)->second;
            ParamContainer param = it->first->second;
            vector<int>* data = &(it->second);

            if (data->size() != 4)
                data->push_back(data->at(2));

            if (data->at(3) != 0)
            {
                data->at(3)--;
                ++it;
                continue;
            }
            else
                data->at(3) = data->at(2);

            list<string> cmd = Command::Eval(name, def, param);

            if (cmd.size() != 0)
            {
                list<string>::iterator it2;

                for (it2 = cmd.begin(); it2 != cmd.end() && !endThread; ++it2)
                {
                    string send = *it2;
                    pipeServer->Send(&send);
                    Sleep(data->at(1));

#ifdef VAULTMP_DEBUG
                    if (debug != NULL)
                    {
                        char text[128];
                        ZeroMemory(text, sizeof(text));

                        sprintf(text, "Executing command (%s, sleep: %d)", send.c_str(), data->at(1));
                        debug->Print(text, true);
                    }
#endif
                }
            }

            if (data->at(0) == 0)
            {
                natives.erase(it->first);
                it = cmdlist.erase(it);
            }
            else
                ++it;
        }
    }

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->Print((char*) "Send thread is going to terminate...", true);
#endif

    return ((DWORD) data);
}
