#include "Fallout3.h"

using namespace std;
using namespace RakNet;
using namespace pipe;

typedef HINSTANCE (__stdcall *fLoadLibrary)(char*);
typedef LPVOID (__stdcall *fGetProcAddress)(HINSTANCE, char*);
typedef void (*fDLLjump)(bool);

enum {
    ID_MASTER_QUERY = ID_USER_PACKET_ENUM,
    ID_MASTER_ANNOUNCE,
    ID_MASTER_UPDATE,
    ID_GAME_INIT,
    ID_GAME_RUN,
    ID_GAME_START,
    ID_GAME_END,
    ID_NEW_PLAYER,
    ID_PLAYER_LEFT,
    ID_PLAYER_UPDATE
};

#pragma pack(push, 1)
struct Fallout3::pPlayerUpdate {
    unsigned char type;
    RakNetGUID guid;
    float X, Y, Z, A;
    float health;
    float baseHealth;
    float conds[6];
    bool dead;
    bool alerted;
    int moving;
};
#pragma pack(pop)

struct Fallout3::fCommand {
    string command;
    string refID;
    bool repeat;
    bool forplayers;
    int sleepmult;

    fCommand() {
        command = "";
        refID = "";
        repeat = false;
        forplayers = false;
        sleepmult = 1;
    }
};

bool Fallout3::NewVegas = false;
bool Fallout3::endThread = false;
bool Fallout3::wakeup = false;
HANDLE Fallout3::Fallout3pipethread;
HANDLE Fallout3::Fallout3gamethread;
Player* Fallout3::self;
queue<Player*> Fallout3::refqueue;
list<Fallout3::fCommand*> Fallout3::cmdlist;
bool Fallout3::cmdmutex = false;
Fallout3::pPlayerUpdate Fallout3::localPlayerUpdate;
PipeClient* Fallout3::pipeServer;
PipeServer* Fallout3::pipeClient;

DWORD Fallout3::lookupProgramID(const char process[])
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
         } while(Process32Next(hSnapshot, &ProcessEntry));

      CloseHandle(hSnapshot);

      return 0;
}

struct Fallout3::INJECT
{
      fLoadLibrary LoadLibrary;
      fGetProcAddress GetProcAddress;
      char DLLpath[256];
      char DLLjump[16];
      bool NewVegas;
};

DWORD WINAPI Fallout3::InjectedCode(LPVOID addr)
{
	HINSTANCE hDll;
	fDLLjump DLLjump;
	INJECT* is = (INJECT*) addr;
	hDll = is->LoadLibrary(is->DLLpath);
	DLLjump = (fDLLjump) is->GetProcAddress(hDll, is->DLLjump);
	DLLjump(is->NewVegas);
	return 0;
}

void Fallout3::InjectedEnd()
{
     /* This is to calculate the size of InjectedCode */
}

DWORD WINAPI Fallout3::Fallout3pipe(LPVOID data)
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

    char module[32];

    switch (NewVegas)
    {
        case true:
            strcpy(module, "FalloutNV.exe");
            break;
        case false:
            strcpy(module, "Fallout3.exe");
            break;
    }

    if (!endThread)
    {
        do
        {
            recv.clear(); low.clear(); high.clear();
            recv = pipeClient->Recv();
            low = recv.substr(0, 3);
            high = recv.substr(3);

            Player* lastRef = self;

            int find = 0;

            while (find != string::npos)
            {
                if (low.compare("op:") == 0)
                {
                    char output[high.length()];
                    char* token;
                    strcpy(output, high.c_str());
                    token = strtok(output, ":.<> ");

                    if (stricmp(token, "GetPos") == 0)
                    {
                        token = strtok(NULL, ":.<> ");

                        if (stricmp(token, "X") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float X = (float) atof(token);
                            lastRef->SetPlayerPos(0, X);
                        }
                        else if (stricmp(token, "Y") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float Y = (float) atof(token);
                            lastRef->SetPlayerPos(1, Y);
                        }
                        else if (stricmp(token, "Z") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float Z = (float) atof(token);
                            lastRef->SetPlayerPos(2, Z + 3.00);
                        }
                    }
                    else if (stricmp(token, "GetAngle") == 0)
                    {
                        token = strtok(NULL, ":.<> ");

                        if (stricmp(token, "Z") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float Z = (float) atof(token);
                            lastRef->SetPlayerAngle(Z);
                        }
                    }
                    else if (stricmp(token, "GetBaseActorValue") == 0)
                    {
                        token = strtok(NULL, ":.<> ");

                        if (stricmp(token, "Health") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float baseHealth = (float) atof(token);
                            lastRef->SetPlayerBaseHealth(baseHealth);
                        }
                    }
                    else if (stricmp(token, "GetActorValue") == 0)
                    {
                        token = strtok(NULL, ":.<> ");

                        if (stricmp(token, "Health") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float health = (float) atof(token);
                            lastRef->SetPlayerHealth(health);
                        }
                        else if (stricmp(token, "PerceptionCondition") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float cond = (float) atof(token);
                            lastRef->SetPlayerCondition(0, cond);
                        }
                        else if (stricmp(token, "EnduranceCondition") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float cond = (float) atof(token);
                            lastRef->SetPlayerCondition(1, cond);
                        }
                        else if (stricmp(token, "LeftAttackCondition") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float cond = (float) atof(token);
                            lastRef->SetPlayerCondition(2, cond);
                        }
                        else if (stricmp(token, "RightAttackCondition") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float cond = (float) atof(token);
                            lastRef->SetPlayerCondition(3, cond);
                        }
                        else if (stricmp(token, "LeftMobilityCondition") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float cond = (float) atof(token);
                            lastRef->SetPlayerCondition(4, cond);
                        }
                        else if (stricmp(token, "RightMobilityCondition") == 0)
                        {
                            token = strtok(NULL, ":<> ");
                            float cond = (float) atof(token);
                            lastRef->SetPlayerCondition(5, cond);
                        }
                    }
                    else if (stricmp(token, "GetDead") == 0)
                    {
                        token = strtok(NULL, ":<> ");
                        float dead = (float) atof(token);
                        lastRef->SetPlayerDead(dead != 0.00 ? true : false);
                    }
                    else if (stricmp(token, "Wants") == 0)
                    {
                        token = strtok(NULL, ",");
                        token = strtok(NULL, ",");

                        if (stricmp(token, " Weapon Drawn 0") == 0)
                            lastRef->SetPlayerAlerted(false);
                        else if (stricmp(token, " Weapon Drawn 1") == 0)
                            lastRef->SetPlayerAlerted(true);
                    }
                    else if (stricmp(token, "Movement") == 0)
                    {
                        token = strtok(NULL, ":.<>-/ ");

                        if (stricmp(token, "FastForward") == 0)
                            lastRef->SetPlayerMoving(1);
                        else if (stricmp(token, "FastBackward") == 0)
                            lastRef->SetPlayerMoving(2);
                        else if (stricmp(token, "FastLeft") == 0)
                            lastRef->SetPlayerMoving(3);
                        else if (stricmp(token, "FastRight") == 0)
                            lastRef->SetPlayerMoving(4);
                        else if (stricmp(token, "Forward") == 0)
                            lastRef->SetPlayerMoving(5);
                        else if (stricmp(token, "Backward") == 0)
                            lastRef->SetPlayerMoving(6);
                        else if (stricmp(token, "Left") == 0)
                            lastRef->SetPlayerMoving(7);
                        else if (stricmp(token, "Right") == 0)
                            lastRef->SetPlayerMoving(8);
                        else
                        {
                            token = strtok(NULL, ":.<>-/ ");

                            if (token != NULL)
                            {
                                if (stricmp(token, "FastForward") == 0)
                                    lastRef->SetPlayerMoving(1);
                                else if (stricmp(token, "FastBackward") == 0)
                                    lastRef->SetPlayerMoving(2);
                                else if (stricmp(token, "FastLeft") == 0)
                                    lastRef->SetPlayerMoving(3);
                                else if (stricmp(token, "FastRight") == 0)
                                    lastRef->SetPlayerMoving(4);
                                else if (stricmp(token, "Forward") == 0)
                                    lastRef->SetPlayerMoving(5);
                                else if (stricmp(token, "Backward") == 0)
                                    lastRef->SetPlayerMoving(6);
                                else if (stricmp(token, "Left") == 0)
                                    lastRef->SetPlayerMoving(7);
                                else if (stricmp(token, "Right") == 0)
                                    lastRef->SetPlayerMoving(8);
                            }
                        }
                    }
                    /*else if (stricmp(token, "Weapon") == 0)
                    {

                    }
                    else if (stricmp(token, "Idle") == 0)
                    {

                    }*/
                    else if (stricmp(token, "IsMoving") == 0)
                    {
                        token = strtok(NULL, ":.<> ");
                        int moving = atoi(token);
                        if (moving == 0) lastRef->SetPlayerMoving(moving);
                    }
                    else if (stricmp(token, "player") == 0)
                        lastRef = self;
                    else
                    {
                        Player* newRef = Player::GetPlayerFromRefID(token);
                        if (newRef != NULL)
                            lastRef = newRef;
                    }
                }
                else if (low.compare("re:") == 0)
                {
                    OPENCMD();

                    fCommand* cmd = new fCommand;
                    cmd->command = "setrestrained 1";
                    cmd->refID = high;
                    cmd->forplayers = false;
                    cmd->repeat = false;
                    PUSHCMD(cmd);

                    CLOSECMD();

                    if (!refqueue.empty())
                    {
                        Player* player = refqueue.front();
                        player->SetPlayerRefID(high);
                        refqueue.pop();
                    }
                }
                else if (low.compare("up:") == 0)
                    wakeup = true;

                find = high.find("op:");
                if (find == string::npos) find = high.find("re:");
                if (find == string::npos) find = high.find("up:");
                if (find == string::npos) find = high.find("ca:");
                if (find != string::npos)
                {
                    low = high.substr(find, 3);
                    high = high.substr(find + 3);
                }
            }

            if (lookupProgramID(module) == 0)
                endThread = true;
        } while (low.compare("ca:") != 0 && !endThread);
    }

    return ((DWORD) data);
}

DWORD WINAPI Fallout3::Fallout3game(LPVOID data)
{
    fCommand cmds[15];

    cmds[0].command = "getpos X";
    cmds[0].repeat = true;
    cmds[0].forplayers = true;

    cmds[1].command = "getpos Y";
    cmds[1].repeat = true;
    cmds[1].forplayers = true;

    cmds[2].command = "getpos Z";
    cmds[2].repeat = true;
    cmds[2].forplayers = true;

    cmds[3].command = "getangle Z";
    cmds[3].repeat = true;
    cmds[3].forplayers = false;

    cmds[4].command = "getbaseactorvalue Health";
    cmds[4].repeat = true;
    cmds[4].forplayers = false;

    cmds[5].command = "getactorvalue Health";
    cmds[5].repeat = true;
    cmds[5].forplayers = false;

    cmds[6].command = "getactorvalue PerceptionCondition";
    cmds[6].repeat = true;
    cmds[6].forplayers = false;

    cmds[7].command = "getactorvalue EnduranceCondition";
    cmds[7].repeat = true;
    cmds[7].forplayers = false;

    cmds[8].command = "getactorvalue LeftAttackCondition";
    cmds[8].repeat = true;
    cmds[8].forplayers = false;

    cmds[9].command = "getactorvalue RightAttackCondition";
    cmds[9].repeat = true;
    cmds[9].forplayers = false;

    cmds[10].command = "getactorvalue LeftMobilityCondition";
    cmds[10].repeat = true;
    cmds[10].forplayers = false;

    cmds[11].command = "getactorvalue RightMobilityCondition";
    cmds[11].repeat = true;
    cmds[11].forplayers = false;

    cmds[12].command = "getdead";
    cmds[12].repeat = true;
    cmds[12].forplayers = true;

    cmds[13].command = "showanim";
    cmds[13].repeat = true;
    cmds[13].forplayers = false;

    cmds[14].command = "ismoving";
    cmds[14].repeat = true;
    cmds[14].forplayers = false;

    for (int i = 0; i < sizeof(cmds) / sizeof(fCommand); i++)
        cmdlist.push_back(&cmds[i]);

    while (!endThread)
    {
        OPENCMD();

        list<fCommand*>::iterator it;

        for (it = cmdlist.begin(); it != cmdlist.end();)
        {
            fCommand* cmd = *it;

            if (!cmd->refID.empty())
            {
                if (cmd->refID.compare("none") == 0)
                {
                    string input = "op:" + cmd->command;
                    pipeServer->Send(&input);
                }
                else
                {
                    string input = "op:" + cmd->refID + "." + cmd->command;
                    pipeServer->Send(&input);
                }
                Sleep(FALLOUT3_TICKS * cmd->sleepmult);
            }
            else if (!cmd->forplayers)
            {
                string input = "op:player." + cmd->command;
                pipeServer->Send(&input);
                Sleep(FALLOUT3_TICKS * cmd->sleepmult);
            }
            else
            {
                map<RakNetGUID, string> players = Player::GetPlayerList();
                map<RakNetGUID, string>::iterator it2;

                for (it2 = players.begin(); it2 != players.end(); it2++)
                {
                    string refID = it2->second;

                    if (refID.compare("none") != 0)
                    {
                        string input = "op:" + refID + "." + cmd->command;
                        pipeServer->Send(&input);
                        Sleep(FALLOUT3_TICKS * cmd->sleepmult);
                    }
                }
            }

            if (!cmd->repeat)
            {
                delete cmd;
                it = cmdlist.erase(it);
            }
            else
                it++;
        }

        CLOSECMD();

        Sleep(FALLOUT3_TICKS); // let the packets fill the ranks!
    }

    return ((DWORD) data);
}

HANDLE Fallout3::InitializeFallout3(bool NewVegas)
{
    FILE* Fallout = NewVegas ? fopen("FalloutNV.exe", "rb") : fopen("Fallout3.exe", "rb");

    if (Fallout != NULL)
    {
        fclose(Fallout);

        char module[32];

        switch (NewVegas)
        {
            case false:
                strcpy(module, "Fallout3.exe");
                Fallout3::NewVegas = false;
                break;
            case true:
                strcpy(module, "FalloutNV.exe");
                putenv("SteamAppID=22380"); // necessary for Steam
                Fallout3::NewVegas = true;
                break;
        }

        FILE* vaultmp = fopen("vaultmp.dll", "rb");

        if (vaultmp != NULL)
        {
            fclose(vaultmp);

            if (lookupProgramID(module) == 0)
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;

                ZeroMemory(&si, sizeof(si));
                ZeroMemory(&pi, sizeof(pi));
                si.cb = sizeof(si);

                if (CreateProcess(module, NULL, NULL, NULL, FALSE, Fallout3::NewVegas ? 0 : CREATE_SUSPENDED, NULL, NULL, &si, &pi))
                {
                    if (Fallout3::NewVegas) Sleep(2000); // some Steam decrypt whatsoever needs time

                    HANDLE hProc;

                    hProc = OpenProcess(PROCESS_ALL_ACCESS, false, pi.dwProcessId);

                    if (hProc)
                    {
                        INJECT data;
                        HINSTANCE hDll;
                        TCHAR curdir[MAX_PATH + 1];
                        LPVOID start, thread;
                        DWORD codesize;

                        GetModuleFileName(GetModuleHandle(NULL), (LPTSTR) curdir, MAX_PATH);
                        PathRemoveFileSpec(curdir);

                        strcat(curdir, "\\vaultmp.dll");
                        strcpy(data.DLLpath, curdir);
                        strcpy(data.DLLjump, "DLLjump");

                        hDll = LoadLibrary("kernel32.dll");
                        data.LoadLibrary = (fLoadLibrary) GetProcAddress(hDll, "LoadLibraryA");
                        data.GetProcAddress = (fGetProcAddress) GetProcAddress(hDll, "GetProcAddress");
                        data.NewVegas = Fallout3::NewVegas;

                        codesize = (DWORD) InjectedEnd - (DWORD) InjectedCode;

                        start = VirtualAllocEx(hProc, 0, codesize + sizeof(INJECT), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
                        thread = (LPVOID) ((DWORD) start + sizeof(INJECT));

                        WriteProcessMemory(hProc, start, (LPVOID) &data, sizeof(INJECT), NULL);
                        WriteProcessMemory(hProc, thread, (LPVOID) InjectedCode, codesize, NULL);

                        CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE) thread, start, 0, 0);

                        /* Initalizing vaultmp.exe <-> Fallout3.exe / FalloutNV.exe pipe */

                        HANDLE PipeThread;
                        DWORD Fallout3pipeID;

                        PipeThread = CreateThread(NULL, 0, Fallout3pipe, (LPVOID) 0, 0, &Fallout3pipeID);

                        while (!wakeup) Sleep(2);

                        /* Resuming Fallout3.exe */

                        if (!Fallout3::NewVegas) ResumeThread(pi.hThread);

                        CloseHandle(hProc);

                        return PipeThread;
                    }
                    else
                        return NULL; // Process opening failed
                }
                else
                    return NULL; // Process creation failed
            }
            else
                return NULL; // Fallout3.exe / FalloutNV.exe running
        }
        else
            return NULL; // vaultmp.dll missing
    }
    else
         return NULL; // Fallout3.exe / FalloutNV.exe missing

    return NULL;
}

void Fallout3::InitializeVaultMP(RakPeerInterface* peer, SystemAddress addr, string name, string pwd, bool NewVegas)
{
    endThread = false;
    Fallout3pipethread = NULL;
    Fallout3gamethread = NULL;
    pipeClient = new PipeServer();
    pipeServer = new PipeClient();

    Player::DestroyInstances();

    self = new Player(peer->GetMyGUID());
    self->SetPlayerName(name);
    self->SetPlayerRefID("player");

    localPlayerUpdate.type = ID_PLAYER_UPDATE;
    localPlayerUpdate.guid = peer->GetMyGUID();
    localPlayerUpdate.X = 0.00;
    localPlayerUpdate.Y = 0.00;
    localPlayerUpdate.Z = 0.00;
    localPlayerUpdate.A = 0.00;
    localPlayerUpdate.health = 0.00;
    localPlayerUpdate.baseHealth = 0.00;
    localPlayerUpdate.conds[0] = 0.00;
    localPlayerUpdate.conds[1] = 0.00;
    localPlayerUpdate.conds[2] = 0.00;
    localPlayerUpdate.conds[3] = 0.00;
    localPlayerUpdate.conds[4] = 0.00;
    localPlayerUpdate.conds[5] = 0.00;
    localPlayerUpdate.dead = false;
    localPlayerUpdate.alerted = false;
    localPlayerUpdate.moving = 0;

    cmdlist.clear();

    if (peer->Connect(addr.ToString(false), addr.GetPort(), 0, 0, 0, 0, 3, 500, 0) == CONNECTION_ATTEMPT_STARTED)
    {
        bool query = true;

        Packet* packet;

        while (query)
        {
            if (Fallout3pipethread != NULL)
                if (WaitForSingleObject(Fallout3pipethread, 0) != WAIT_TIMEOUT)
                {
                    BitStream query;
                    query.Write((MessageID) ID_GAME_END);
                    peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, addr, false, 0);
                }

            if (Fallout3gamethread != NULL)
                if (WaitForSingleObject(Fallout3gamethread, 0) != WAIT_TIMEOUT)
                {
                    BitStream query;
                    query.Write((MessageID) ID_GAME_END);
                    peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, addr, false, 0);
                }

            for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
            {
                switch (packet->data[0])
                {
                case ID_CONNECTION_REQUEST_ACCEPTED:
                {
                    BitStream query;

                    query.Write((MessageID) ID_GAME_INIT);

                    RakString rname(name.c_str());
                    RakString rpwd(pwd.c_str());
                    query.Write(rname);
                    query.Write(rpwd);

                    peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);
                    break;
                }
                case ID_GAME_INIT:
                {
                    BitStream query;

                    Fallout3pipethread = InitializeFallout3(NewVegas);

                    Sleep(2500); // Let the game start

                    if (Fallout3pipethread != NULL)
                        query.Write((MessageID) ID_GAME_RUN);
                    else
                        query.Write((MessageID) ID_GAME_END);

                    peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);
                    break;
                }
                case ID_GAME_RUN:
                {
                    BitStream query(packet->data, packet->length, false);
                    query.IgnoreBytes(sizeof(MessageID));

                    RakString save;
                    query.Read(save);
                    query.Reset();

                    OPENCMD();

                    fCommand* cmd = new fCommand;
                    cmd->command = "load " + save;
                    cmd->refID = "none";
                    cmd->forplayers = false;
                    cmd->repeat = false;
                    PUSHCMD(cmd);

                    cmd = new fCommand;
                    cmd->command = "removeallitems";
                    cmd->forplayers = false;
                    cmd->repeat = false;
                    PUSHCMD(cmd);

                    CLOSECMD();

                    DWORD Fallout3gamethreadID;
                    Fallout3gamethread = CreateThread(NULL, 0, Fallout3game, (LPVOID) 0, 0, &Fallout3gamethreadID);

                    query.Write((MessageID) ID_GAME_START);
                    peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);
                    break;
                }
                case ID_NEW_PLAYER:
                {
                    BitStream query(packet->data, packet->length, false);
                    query.IgnoreBytes(sizeof(MessageID));

                    RakNetGUID guid;
                    RakString name;
                    query.Read(guid);
                    query.Read(name);
                    query.Reset();

                    Player* player = new Player(guid);
                    player->SetPlayerName(string(name.C_String()));
                    refqueue.push(player);

                    OPENCMD();

                    fCommand* cmd = new fCommand;
                    cmd->command = "placeatme 00000007 1";
                    cmd->forplayers = false;
                    cmd->repeat = false;
                    cmd->sleepmult = 10;
                    PUSHCMD(cmd);

                    CLOSECMD();
                    break;
                }
                case ID_PLAYER_LEFT:
                {
                    BitStream query(packet->data, packet->length, false);
                    query.IgnoreBytes(sizeof(MessageID));

                    RakNetGUID guid;
                    query.Read(guid);
                    query.Reset();

                    Player* player = Player::GetPlayerFromGUID(guid);
                    string refID = player->GetPlayerRefID();

                    if (refID.compare("none") != 0)
                    {
                        OPENCMD();

                        fCommand* cmd = new fCommand;
                        cmd->command = "disable";
                        cmd->refID = refID;
                        cmd->forplayers = false;
                        cmd->repeat = false;
                        PUSHCMD(cmd);

                        cmd = new fCommand;
                        cmd->command = "markfordelete";
                        cmd->refID = refID;
                        cmd->forplayers = false;
                        cmd->repeat = false;
                        PUSHCMD(cmd);

                        CLOSECMD();
                    }

                    delete player;
                    break;
                }
                case ID_PLAYER_UPDATE:
                {
                    pPlayerUpdate* update = (pPlayerUpdate*) packet->data;

                    if (packet->length != sizeof(pPlayerUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(update->guid);
                    string refID = player->GetPlayerRefID();

                    fCommand* cmd;

                    if (refID.compare("none") != 0)
                    {
                        OPENCMD();

                        char pos[16];

                        if (!player->IsPlayerNearPoint(update->X, update->Y, update->Z, 400.0))
                        {
                            sprintf(pos, "%f", update->X);

                            cmd = new fCommand;
                            cmd->command = "setpos X " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            sprintf(pos, "%f", update->Y);

                            cmd = new fCommand;
                            cmd->command = "setpos Y " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            sprintf(pos, "%f", update->Z);

                            cmd = new fCommand;
                            cmd->command = "setpos Z " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            player->SetPlayerPos(0, update->X);
                            player->SetPlayerPos(1, update->Y);
                            player->SetPlayerPos(2, update->Z);
                        }

                        sprintf(pos, "%f", update->A);

                        cmd = new fCommand;
                        cmd->command = "setangle Z " + string(pos);
                        cmd->refID = refID;
                        cmd->forplayers = false;
                        cmd->repeat = false;
                        PUSHCMD(cmd);

                        player->SetPlayerAngle(update->A);

                        if (update->alerted != player->IsPlayerAlerted())
                        {
                            cmd = new fCommand;
                            cmd->command = "setrestrained 0";
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            cmd = new fCommand;
                            cmd->command = "setalert ";
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;

                            switch (update->alerted)
                            {
                                case true:
                                    cmd->command.append("1");
                                    break;
                                case false:
                                    cmd->command.append("0");
                                    break;
                            }

                            PUSHCMD(cmd);

                            cmd = new fCommand;
                            cmd->command = "setrestrained 1";
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            player->SetPlayerAlerted(update->alerted);
                        }

                        if (update->moving != player->GetPlayerMoving())
                        {
                            cmd = new fCommand;
                            cmd->command = "playgroup ";
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;

                            switch (update->moving)
                            {
                                case 0:
                                    cmd->command.append("Idle 0");
                                    break;
                                case 1:
                                    cmd->command.append("FastForward 1");
                                    break;
                                case 2:
                                    cmd->command.append("FastBackward 1");
                                    break;
                                case 3:
                                    cmd->command.append("FastLeft 1");
                                    break;
                                case 4:
                                    cmd->command.append("FastRight 1");
                                    break;
                                case 5:
                                    cmd->command.append("Forward 1");
                                    break;
                                case 6:
                                    cmd->command.append("Backward 1");
                                    break;
                                case 7:
                                    cmd->command.append("Left 1");
                                    break;
                                case 8:
                                    cmd->command.append("Right 1");
                                    break;
                            }

                            PUSHCMD(cmd);

                            player->SetPlayerMoving(update->moving);
                        }

                        if (update->baseHealth != player->GetPlayerBaseHealth())
                        {
                            sprintf(pos, "%i", (int) update->baseHealth);

                            cmd = new fCommand;
                            cmd->command = "setactorvalue Health " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            player->SetPlayerBaseHealth(update->baseHealth);
                        }

                        sprintf(pos, "%i", (int) update->health);

                        cmd = new fCommand;
                        cmd->command = "forceactorvalue Health " + string(pos);
                        cmd->refID = refID;
                        cmd->forplayers = false;
                        cmd->repeat = false;
                        PUSHCMD(cmd);

                        player->SetPlayerHealth(update->health);

                        if (update->conds[0] != player->GetPlayerCondition(0))
                        {
                            sprintf(pos, "%i", (int) update->conds[0]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue PerceptionCondition " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            player->SetPlayerCondition(0, update->conds[0]);
                        }

                        if (update->conds[1] != player->GetPlayerCondition(1))
                        {
                            sprintf(pos, "%i", (int) update->conds[1]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue EnduranceCondition " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            player->SetPlayerCondition(1, update->conds[1]);
                        }

                        if (update->conds[2] != player->GetPlayerCondition(2))
                        {
                            sprintf(pos, "%i", (int) update->conds[2]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue LeftAttackCondition " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            player->SetPlayerCondition(2, update->conds[2]);
                        }

                        if (update->conds[3] != player->GetPlayerCondition(3))
                        {
                            sprintf(pos, "%i", (int) update->conds[3]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue RightAttackCondition " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            player->SetPlayerCondition(3, update->conds[3]);
                        }

                        if (update->conds[4] != player->GetPlayerCondition(4))
                        {
                            sprintf(pos, "%i", (int) update->conds[4]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue LeftMobilityCondition " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            player->SetPlayerCondition(4, update->conds[4]);
                        }

                        if (update->conds[5] != player->GetPlayerCondition(5))
                        {
                            sprintf(pos, "%i", (int) update->conds[5]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue RightMobilityCondition " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            player->SetPlayerCondition(5, update->conds[5]);
                        }

                        // more ActorValues

                        if (player->IsPlayerDead() != update->dead)
                        {
                            cmd = new fCommand;
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;

                            switch (update->dead)
                            {
                                case true:
                                    cmd->command = "killactor";
                                    PUSHCMD(cmd);
                                    break;
                                case false:
                                    cmd->command = "resurrect 0";
                                    PUSHCMD(cmd);

                                    cmd = new fCommand;
                                    cmd->command = "setrestrained 1";
                                    cmd->refID = refID;
                                    cmd->forplayers = false;
                                    cmd->repeat = false;
                                    PUSHCMD(cmd);
                                    break;
                            }
                            player->SetPlayerDead(update->dead);
                        }

                        CLOSECMD();
                    }
                    break;
                }
                case ID_NO_FREE_INCOMING_CONNECTIONS:
                case ID_CONNECTION_ATTEMPT_FAILED:
                case ID_DISCONNECTION_NOTIFICATION:
                case ID_CONNECTION_BANNED:
                case ID_CONNECTION_LOST:
                    query = false;
                    break;
                }
            }

            float X = self->GetPlayerPos(0);
            float Y = self->GetPlayerPos(1);
            float Z = self->GetPlayerPos(2);
            float A = self->GetPlayerAngle();
            float health = self->GetPlayerHealth();
            float baseHealth = self->GetPlayerBaseHealth();
            float conds[6] = {self->GetPlayerCondition(0), self->GetPlayerCondition(1), self->GetPlayerCondition(2), self->GetPlayerCondition(3), self->GetPlayerCondition(4), self->GetPlayerCondition(5)};
            bool dead = self->IsPlayerDead();
            bool alerted = self->IsPlayerAlerted();
            int moving = self->GetPlayerMoving();

            if (X != localPlayerUpdate.X || Y != localPlayerUpdate.Y || Z != localPlayerUpdate.Z || A != localPlayerUpdate.A || health != localPlayerUpdate.health || baseHealth != localPlayerUpdate.baseHealth ||
                conds[0] != localPlayerUpdate.conds[0] || conds[1] != localPlayerUpdate.conds[1] || conds[2] != localPlayerUpdate.conds[2] || conds[3] != localPlayerUpdate.conds[3] || conds[4] != localPlayerUpdate.conds[4] ||
                conds[5] != localPlayerUpdate.conds[5] || dead != localPlayerUpdate.dead || alerted != localPlayerUpdate.alerted || moving != localPlayerUpdate.moving)
            {
                localPlayerUpdate.X = X;
                localPlayerUpdate.Y = Y;
                localPlayerUpdate.Z = Z;
                localPlayerUpdate.A = A;
                localPlayerUpdate.health = health;
                localPlayerUpdate.baseHealth = baseHealth;
                localPlayerUpdate.conds[0] = conds[0];
                localPlayerUpdate.conds[1] = conds[1];
                localPlayerUpdate.conds[2] = conds[2];
                localPlayerUpdate.conds[3] = conds[3];
                localPlayerUpdate.conds[4] = conds[4];
                localPlayerUpdate.conds[5] = conds[5];
                localPlayerUpdate.dead = dead;
                localPlayerUpdate.alerted = alerted;
                localPlayerUpdate.moving = moving;
                peer->Send((char*) &localPlayerUpdate, sizeof(localPlayerUpdate), HIGH_PRIORITY, RELIABLE, 0, addr, false, 0);
            }

            RakSleep(2);
        }
    }

    if (Fallout3pipethread != NULL)
    {
        if (WaitForSingleObject(Fallout3pipethread, 0) == WAIT_TIMEOUT)
            endThread = true;
        CloseHandle(Fallout3pipethread);
    }

    if (Fallout3gamethread != NULL)
    {
        if (WaitForSingleObject(Fallout3gamethread, 0) == WAIT_TIMEOUT)
            endThread = true;
        CloseHandle(Fallout3gamethread);
    }

    delete self;
    delete pipeClient;
    delete pipeServer;
}
