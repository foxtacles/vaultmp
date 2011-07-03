#include "Fallout3.h"

using namespace std;
using namespace RakNet;
using namespace pipe;

typedef HINSTANCE (__stdcall *fLoadLibrary)(char*);
typedef LPVOID (__stdcall *fGetProcAddress)(HINSTANCE, char*);
typedef void (*fDLLjump)(bool);

bool Fallout3::NewVegas = false;
bool Fallout3::endThread = false;
bool Fallout3::wakeup = false;
HANDLE Fallout3::Fallout3pipethread;
HANDLE Fallout3::Fallout3gamethread;
Player* Fallout3::self;
queue<Player*> Fallout3::refqueue;
list<fCommand*> Fallout3::cmdlist;
list<fCommand*> Fallout3::tmplist;
fCommand* Fallout3::skipcmds[MAX_SKIP_FLAGS];
bool Fallout3::cmdmutex = false;
PipeClient* Fallout3::pipeServer;
PipeServer* Fallout3::pipeClient;

#ifdef VAULTMP_DEBUG
Debug* Fallout3::debug;
#endif

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
        Player* lastRef = self;

        do
        {
            recv.clear(); low.clear(); high.clear();
            recv = pipeClient->Recv();

            #ifdef VAULTMP_DEBUG
            //debug->Print(const_cast<char*>(recv.c_str()), true);
            #endif

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

                    char output[high.length()];
                    char* token;
                    strcpy(output, high.c_str());

                    Fallout3_opcode = strtoul(output, &token, 16);
                    Fallout3_refID = strtoul(token, &token, 16);
                    Fallout3_result = strtoull(token, &token, 16);
                    Fallout3_coord = (unsigned char) strtoul(token, &token, 16);
                    Fallout3_newRefID = strtoull(token, &token, 16);

                    if (Fallout3_refID == 0x00000014)
                        lastRef = self;
                    else
                    {
                        char refstr[8];
                        ZeroMemory(refstr, sizeof(refstr));

                        sprintf(refstr, "%x", Fallout3_refID);
                        lastRef = Player::GetPlayerFromRefID(refstr);
                    }

                    if (lastRef != NULL)
                    {
                        switch (Fallout3_opcode)
                        {
                            case 0x1006: // GetPos
                            {
                                switch (Fallout3_coord)
                                {
                                    case 0x58: // X
                                        lastRef->SetPlayerPos(0, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x59: // Y
                                        lastRef->SetPlayerPos(1, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x5A: // Z
                                        lastRef->SetPlayerPos(2, (float) *((double*) (&Fallout3_result)));
                                        break;
                                }
                                break;
                            }
                            case 0x1008: // GetAngle
                            {
                                switch (Fallout3_coord)
                                {
                                    case 0x5A: // Z
                                        lastRef->SetPlayerAngle((float) *((double*) (&Fallout3_result)));
                                        break;
                                }
                                break;
                            }
                            case 0x1115: // GetBaseActorValue
                            {
                                switch (Fallout3_coord)
                                {
                                    case 0x10: // Health
                                        lastRef->SetPlayerBaseHealth((float) *((double*) (&Fallout3_result)));
                                        break;
                                }
                                break;
                            }
                            case 0x100E: // GetActorValue
                            {
                                switch (Fallout3_coord)
                                {
                                    case 0x10: // Health
                                        lastRef->SetPlayerHealth((float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x19: // PerceptionCondition
                                        lastRef->SetPlayerCondition(0, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x1A: // EnduranceCondition
                                        lastRef->SetPlayerCondition(1, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x1B: // LeftAttackCondition
                                        lastRef->SetPlayerCondition(2, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x1C: // RightAttackCondition
                                        lastRef->SetPlayerCondition(3, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x1D: // LeftMobilityCondition
                                        lastRef->SetPlayerCondition(4, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x1E: // RightMobilityCondition
                                        lastRef->SetPlayerCondition(5, (float) *((double*) (&Fallout3_result)));
                                        break;
                                }
                                break;
                            }
                            case 0x102E: // GetDead
                            {
                                if (Fallout3_result == 0x00)
                                    lastRef->SetPlayerDead(false);
                                else
                                    lastRef->SetPlayerDead(true);
                                break;
                            }
                            case 0x1019: // IsMoving
                            {
                                if (Fallout3_result == 0x00)
                                    lastRef->SetPlayerMoving(0);
                                break;
                            }
                            case 0x1025: // PlaceAtMe
                            {
                                char refstr[8];
                                ZeroMemory(refstr, sizeof(refstr));

                                sprintf(refstr, "%x", Fallout3_newRefID);

                                OPENCMD();

                                fCommand* cmd = new fCommand;
                                cmd->command = "setrestrained 1";
                                cmd->refID = refstr;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                cmd->sleep = 150;
                                PUSHCMD(cmd);

                                CLOSECMD();

                                #ifdef VAULTMP_DEBUG
                                char text[128];
                                ZeroMemory(text, sizeof(text));

                                sprintf(text, "Received RefID from game: %s", refstr);
                                debug->Print(text, true);
                                #endif

                                if (!refqueue.empty())
                                {
                                    Player* player = refqueue.front();
                                    player->SetPlayerRefID(refstr);
                                    refqueue.pop();
                                }
                                break;
                            }
                            case 0x0114: // ShowAnim
                            {
                                // has been handled by string parser
                                break;
                            }
                            default:
                            {
                                #ifdef VAULTMP_DEBUG
                                char text[128];
                                ZeroMemory(text, sizeof(text));

                                sprintf(text, "Command could not be processed: %s", const_cast<char*>(high.c_str()));
                                debug->Print(text, true);
                                #endif
                                break;
                            }
                        }
                    }
                }
                else if (low.compare("st:") == 0)
                {
                    char output[high.length()];
                    char* token;
                    strcpy(output, high.c_str());
                    token = strtok(output, ":.<> ");

                    if (stricmp(token, "Wants") == 0)
                    {
                        token = strtok(NULL, ",");
                        token = strtok(NULL, ",");

                        if (stricmp(token, " Weapon Drawn 0") == 0)
                            self->SetPlayerAlerted(false);
                        else if (stricmp(token, " Weapon Drawn 1") == 0)
                            self->SetPlayerAlerted(true);
                    }
                    else if (stricmp(token, "Movement") == 0)
                    {
                        token = strtok(NULL, ":.<>-/ ");

                        if (stricmp(token, "FastForward") == 0)
                            self->SetPlayerMoving(1);
                        else if (stricmp(token, "FastBackward") == 0)
                            self->SetPlayerMoving(2);
                        else if (stricmp(token, "FastLeft") == 0)
                            self->SetPlayerMoving(3);
                        else if (stricmp(token, "FastRight") == 0)
                            self->SetPlayerMoving(4);
                        else if (stricmp(token, "Forward") == 0)
                            self->SetPlayerMoving(5);
                        else if (stricmp(token, "Backward") == 0)
                            self->SetPlayerMoving(6);
                        else if (stricmp(token, "Left") == 0)
                            self->SetPlayerMoving(7);
                        else if (stricmp(token, "Right") == 0)
                            self->SetPlayerMoving(8);
                        else
                        {
                            token = strtok(NULL, ":.<>-/ ");

                            if (token != NULL)
                            {
                                if (stricmp(token, "FastForward") == 0)
                                    self->SetPlayerMoving(1);
                                else if (stricmp(token, "FastBackward") == 0)
                                    self->SetPlayerMoving(2);
                                else if (stricmp(token, "FastLeft") == 0)
                                    self->SetPlayerMoving(3);
                                else if (stricmp(token, "FastRight") == 0)
                                    self->SetPlayerMoving(4);
                                else if (stricmp(token, "Forward") == 0)
                                    self->SetPlayerMoving(5);
                                else if (stricmp(token, "Backward") == 0)
                                    self->SetPlayerMoving(6);
                                else if (stricmp(token, "Left") == 0)
                                    self->SetPlayerMoving(7);
                                else if (stricmp(token, "Right") == 0)
                                    self->SetPlayerMoving(8);
                            }
                        }
                    }
                    /*else if (stricmp(token, "Weapon") == 0)
                    {

                    }
                    else if (stricmp(token, "Idle") == 0)
                    {

                    }*/
                    else
                    {
                        #ifdef VAULTMP_DEBUG
                        char text[256];
                        ZeroMemory(text, sizeof(text));

                        sprintf(text, "String could not be processed: %s", const_cast<char*>(high.c_str()));
                        debug->Print(text, true);
                        #endif
                    }
                }
                else if (low.compare("up:") == 0)
                {
                    wakeup = true;

                    #ifdef VAULTMP_DEBUG
                    debug->Print((char*) "vaultmp process waked up (memory patches are done)", true);
                    #endif
                }
            }

            if (lookupProgramID(module) == 0)
            {
                endThread = true;

                #ifdef VAULTMP_DEBUG
                debug->Print((char*) "Game process missing, shutting down...", true);
                #endif
            }
        } while (low.compare("ca:") != 0 && !endThread);
    }

    // kill game process if running

    return ((DWORD) data);
}

DWORD WINAPI Fallout3::Fallout3game(LPVOID data)
{
    fCommand cmds[15];

    cmds[0].command = "getpos X";
    cmds[0].repeat = true;
    cmds[0].forplayers = true;
    skipcmds[SKIPFLAG_GETPOS_X] = &cmds[0];

    cmds[1].command = "getpos Y";
    cmds[1].repeat = true;
    cmds[1].forplayers = true;
    skipcmds[SKIPFLAG_GETPOS_Y] = &cmds[1];

    cmds[2].command = "getpos Z";
    cmds[2].repeat = true;
    cmds[2].forplayers = true;
    skipcmds[SKIPFLAG_GETPOS_Z] = &cmds[2];

    cmds[3].command = "getangle Z";
    cmds[3].repeat = true;
    cmds[3].forplayers = false;

    cmds[4].command = "getbaseactorvalue Health";
    cmds[4].repeat = true;
    cmds[4].forplayers = false;
    cmds[4].priority = 50;

    cmds[5].command = "getactorvalue Health";
    cmds[5].repeat = true;
    cmds[5].forplayers = true;
    cmds[5].priority = 10;
    skipcmds[SKIPFLAG_GETHEALTH] = &cmds[5];

    cmds[6].command = "getactorvalue PerceptionCondition";
    cmds[6].repeat = true;
    cmds[6].forplayers = false;
    cmds[6].priority = 15;

    cmds[7].command = "getactorvalue EnduranceCondition";
    cmds[7].repeat = true;
    cmds[7].forplayers = false;
    cmds[7].priority = 15;

    cmds[8].command = "getactorvalue LeftAttackCondition";
    cmds[8].repeat = true;
    cmds[8].forplayers = false;
    cmds[8].priority = 15;

    cmds[9].command = "getactorvalue RightAttackCondition";
    cmds[9].repeat = true;
    cmds[9].forplayers = false;
    cmds[9].priority = 15;

    cmds[10].command = "getactorvalue LeftMobilityCondition";
    cmds[10].repeat = true;
    cmds[10].forplayers = false;
    cmds[10].priority = 15;

    cmds[11].command = "getactorvalue RightMobilityCondition";
    cmds[11].repeat = true;
    cmds[11].forplayers = false;
    cmds[11].priority = 15;

    cmds[12].command = "getdead";
    cmds[12].repeat = true;
    cmds[12].forplayers = true;
    cmds[12].priority = 7;
    skipcmds[SKIPFLAG_GETDEAD] = &cmds[12];

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
        list<fCommand*>::iterator it;
        list<fCommand*>::iterator insertAt = cmdlist.end();

        for (it = cmdlist.begin(); it != cmdlist.end() && !endThread;)
        {
            if (!tmplist.empty())
            {
                OPENCMD();

                int count = tmplist.size();

                if (insertAt != cmdlist.end())
                {
                    list<fCommand*>::iterator insertAt_tmp = insertAt;
                    insertAt_tmp++;
                    cmdlist.splice(insertAt_tmp, tmplist);
                }
                else
                {
                    list<fCommand*>::iterator it_tmp = it;
                    it_tmp++;
                    cmdlist.splice(it_tmp, tmplist);
                }

                list<fCommand*>::iterator it_tmp = it;
                advance(it_tmp, count);

                insertAt = it_tmp;

                CLOSECMD();
            }

            if (insertAt == it)
                insertAt = cmdlist.end();

            fCommand* cmd = *it;

            if (cmd->skipflag)
            {
                it++;
                continue;
            }

            if (cmd->curPriority != cmd->priority)
            {
                cmd->curPriority++;
                it++;
                continue;
            }
            else
                cmd->curPriority = 0;

            #ifdef VAULTMP_DEBUG
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Executing command (%s on %s, sleep: %d, timediff: %dms)", cmd->command.c_str(), cmd->forplayers ? "all" : cmd->refID.empty() ? "player" : cmd->refID.c_str(), cmd->sleep, !cmd->repeat ? (GetTickCount() - cmd->tcount) : -1);
            debug->Print(text, true);
            #endif

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
                Sleep(cmd->sleep);
            }
            else if (!cmd->forplayers)
            {
                string input = "op:player." + cmd->command;
                pipeServer->Send(&input);
                Sleep(cmd->sleep);
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
                        Sleep(cmd->sleep);
                    }
                }
            }

            if (cmd->flipcmd != -1)
            {
                FLIPFLAG(cmd->flipcmd);

                if (!cmd->refID.empty() && cmd->refID.compare("none") != 0)
                {
                    Player* player = Player::GetPlayerFromRefID(cmd->refID);
                    if (player != NULL)
                        player->ToggleNoOverride(cmd->flipcmd, GETFLAG(cmd->flipcmd));
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
    }

    return ((DWORD) data);
}

HANDLE Fallout3::InitializeFallout3(bool NewVegas)
{
    FILE* Fallout = NewVegas ? fopen("FalloutNV.exe", "rb") : fopen("Fallout3.exe", "rb");

    if (Fallout != NULL)
    {
        char module[32];
        long int size = 0;

        fseek(Fallout, 0, SEEK_END);
        size = ftell(Fallout);
        fclose(Fallout);

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

        if (Fallout3::NewVegas ? (size == FALLOUTNV_EXE1_SIZE || size == FALLOUTNV_EXE2_SIZE) : (size == FALLOUT3_EXE_SIZE))
        {
            FILE* xlive = fopen("xlive.dll", "rb");

            if (xlive != NULL)
            {
                fseek(xlive, 0, SEEK_END);
                size = ftell(xlive);
                fclose(xlive);
            }

            if (Fallout3::NewVegas || (xlive != NULL && size == XLIVE_DLL_SIZE))
            {
                FILE* fose = fopen(Fallout3::NewVegas ? "nvse_1_1.dll" : "fose_1_7.dll", "rb");

                if (fose != NULL)
                {
                    fclose(fose);

                    FILE* vaultmp = fopen("vaultmp.dll", "rb");

                    if (vaultmp != NULL)
                    {
                        fseek(vaultmp, 0, SEEK_END);
                        size = ftell(vaultmp);
                        fclose(vaultmp);

                        if (size == VAULTMP_DLL_SIZE || true) // FIXME
                        {
                            if (lookupProgramID(module) == 0)
                            {
                                STARTUPINFO si;
                                PROCESS_INFORMATION pi;

                                ZeroMemory(&si, sizeof(si));
                                ZeroMemory(&pi, sizeof(pi));
                                si.cb = sizeof(si);

                                if (CreateProcess(module, NULL, NULL, NULL, FALSE, Fallout3::NewVegas ? 0 : CREATE_SUSPENDED, NULL, NULL, &si, &pi))
                                {
                                    if (Fallout3::NewVegas) Sleep(2000); // some decrypt whatsoever needs time

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
                                        MessageBox(NULL, "Failed opening the game process!", "Error", MB_OK | MB_ICONERROR);
                                }
                                else
                                    MessageBox(NULL, "Failed creating the game process!", "Error", MB_OK | MB_ICONERROR);
                            }
                            else
                                MessageBox(NULL, "Either Fallout 3 or Fallout: New Vegas is already runnning!", "Error", MB_OK | MB_ICONERROR);
                        }
                        else
                            MessageBox(NULL, "vaultmp.dll is either corrupted or not up to date!", "Error", MB_OK | MB_ICONERROR);
                    }
                    else
                        MessageBox(NULL, "Could not find vaultmp.dll!", "Error", MB_OK | MB_ICONERROR);
                }
                else
                    MessageBox(NULL, "Could not find FOSE 1.7 / NVSE 1.1!\nhttp://fose.silverlock.org/\nhttp://nvse.silverlock.org/", "Error", MB_OK | MB_ICONERROR);
            }
            else
                MessageBox(NULL, "xlive.dll is either missing or un-patched!", "Error", MB_OK | MB_ICONERROR);
        }
        else
            MessageBox(NULL, "Fallout3.exe / FalloutNV.exe is either corrupted or not supported!", "Error", MB_OK | MB_ICONERROR);
    }
    else
         MessageBox(NULL, "Could not find either Fallout3.exe or FalloutNV.exe!", "Error", MB_OK | MB_ICONERROR);

    return NULL;
}

void Fallout3::InitializeVaultMP(RakPeerInterface* peer, SystemAddress addr, string name, string pwd, bool NewVegas)
{
    endThread = false;
    Fallout3pipethread = NULL;
    Fallout3gamethread = NULL;
    pipeClient = new PipeServer();
    pipeServer = new PipeClient();

    #ifdef VAULTMP_DEBUG
    debug = new Debug((char*) "vaultmp");

    char text[128];
    ZeroMemory(text, sizeof(text));
    sprintf(text, "Vault-Tec Multiplayer Mod client debug log (%s)", CLIENT_VERSION);
    debug->Print(text, false);

    ZeroMemory(text, sizeof(text));
    sprintf(text, "Connecting to server: %s (name: %s, password: %s, game: %s)", addr.ToString(), name.c_str(), pwd.c_str(), NewVegas ? (char*) "Fallout New Vegas" : (char*) "Fallout 3");
    debug->Print(text, false);

    debug->Print((char*) "Visit www.vaultmp.com for help and upload this log if you experience problems with the mod.", false);
    debug->Print((char*) "-----------------------------------------------------------------------------------------------------", false);
    //debug->PrintSystem();
    Player::SetDebugHandler(debug);
    #endif

    Player::DestroyInstances();

    self = new Player(peer->GetMyGUID());
    self->SetPlayerName(name);
    self->SetPlayerRefID("player");

    pPlayerUpdate localPlayerUpdate = self->GetPlayerUpdateStruct();
    pPlayerStateUpdate localPlayerStateUpdate = self->GetPlayerStateUpdateStruct();

    cmdlist.clear();

    if (peer->Connect(addr.ToString(false), addr.GetPort(), DEDICATED_VERSION, sizeof(DEDICATED_VERSION), 0, 0, 3, 500, 0) == CONNECTION_ATTEMPT_STARTED)
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
                    peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, addr, false, 0);
                }

            if (Fallout3gamethread != NULL)
                if (WaitForSingleObject(Fallout3gamethread, 0) != WAIT_TIMEOUT)
                {
                    BitStream query;
                    query.Write((MessageID) ID_GAME_END);
                    peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, addr, false, 0);
                }

            for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
            {
                switch (packet->data[0])
                {
                case ID_CONNECTION_REQUEST_ACCEPTED:
                {
                    #ifdef VAULTMP_DEBUG
                    char text[128];
                    ZeroMemory(text, sizeof(text));

                    sprintf(text, "Connection request accepted (%s)", packet->systemAddress.ToString());
                    debug->Print(text, true);
                    #endif

                    BitStream query;

                    query.Write((MessageID) ID_GAME_INIT);

                    RakString rname(name.c_str());
                    RakString rpwd(pwd.c_str());
                    query.Write(rname);
                    query.Write(rpwd);

                    peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, packet->systemAddress, false, 0);
                    break;
                }
                case ID_GAME_INIT:
                {
                    #ifdef VAULTMP_DEBUG
                    char text[128];
                    ZeroMemory(text, sizeof(text));

                    sprintf(text, "We were successfully authenticated (%s)", packet->systemAddress.ToString());
                    debug->Print(text, true);
                    #endif

                    BitStream query;

                    Fallout3pipethread = InitializeFallout3(NewVegas);

                    Sleep(2500); // Let the game start

                    if (Fallout3pipethread != NULL)
                        query.Write((MessageID) ID_GAME_RUN);
                    else
                        query.Write((MessageID) ID_GAME_END);

                    peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, packet->systemAddress, false, 0);
                    break;
                }
                case ID_GAME_RUN:
                {
                    #ifdef VAULTMP_DEBUG
                    debug->Print((char*) "Initiating vaultmp game thread...", true);
                    #endif

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
                    cmd->sleep = 150;
                    PUSHCMD(cmd);

                    CLOSECMD();

                    DWORD Fallout3gamethreadID;
                    Fallout3gamethread = CreateThread(NULL, 0, Fallout3game, (LPVOID) 0, 0, &Fallout3gamethreadID);

                    query.Write((MessageID) ID_GAME_START);
                    peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, packet->systemAddress, false, 0);
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

                    #ifdef VAULTMP_DEBUG
                    char text[128];
                    ZeroMemory(text, sizeof(text));

                    sprintf(text, "New player on the server (name: %s, guid: %s)", name.C_String(), guid.ToString());
                    debug->Print(text, true);
                    #endif

                    Player* player = new Player(guid);
                    player->SetPlayerName(string(name.C_String()));
                    refqueue.push(player);

                    OPENCMD();

                    fCommand* cmd = new fCommand;
                    if (NewVegas) cmd->command = "placeatme 8D0E7 1";
                    else cmd->command = "placeatme 30D82 1";
                    cmd->forplayers = false;
                    cmd->repeat = false;
                    cmd->sleep = 1500;
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

                    #ifdef VAULTMP_DEBUG
                    char text[128];
                    ZeroMemory(text, sizeof(text));

                    sprintf(text, "Player left (name: %s, guid: %s)", player->GetPlayerName().c_str(), guid.ToString());
                    debug->Print(text, true);
                    #endif

                    if (refID.compare("none") != 0)
                    {
                        OPENCMD();

                        fCommand* cmd = new fCommand;
                        cmd->command = "disable";
                        cmd->refID = refID;
                        cmd->forplayers = false;
                        cmd->repeat = false;
                        cmd->sleep = 150;
                        PUSHCMD(cmd);

                        cmd = new fCommand;
                        cmd->command = "markfordelete";
                        cmd->refID = refID;
                        cmd->forplayers = false;
                        cmd->repeat = false;
                        cmd->sleep = 150;
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

                    #ifdef VAULTMP_DEBUG
                    char text[128];
                    ZeroMemory(text, sizeof(text));

                    sprintf(text, "Received player update packet (name: %s, ref: %s, guid: %s)", player->GetPlayerName().c_str(), refID.c_str(), update->guid.ToString());
                    debug->Print(text, true);
                    #endif

                    fCommand* cmd;

                    if (refID.compare("none") != 0)
                    {
                        OPENCMD();

                        char pos[16];

                        if (!player->IsCoordinateInRange(0, update->X, 350.0))
                        {
                            if (player->SetPlayerPos(0, update->X))
                            {
                                FLAG(SKIPFLAG_GETPOS_X);
                                player->ToggleNoOverride(SKIPFLAG_GETPOS_X, true);

                                sprintf(pos, "%f", update->X);

                                cmd = new fCommand;
                                cmd->command = "setpos X " + string(pos);
                                cmd->refID = refID;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                cmd->flipcmd = SKIPFLAG_GETPOS_X;
                                PUSHCMD(cmd);
                            }
                        }

                        if (!player->IsCoordinateInRange(1, update->Y, 350.0))
                        {
                            if (player->SetPlayerPos(1, update->Y))
                            {
                                FLAG(SKIPFLAG_GETPOS_Y);
                                player->ToggleNoOverride(SKIPFLAG_GETPOS_Y, true);

                                sprintf(pos, "%f", update->Y);

                                cmd = new fCommand;
                                cmd->command = "setpos Y " + string(pos);
                                cmd->refID = refID;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                cmd->flipcmd = SKIPFLAG_GETPOS_Y;
                                PUSHCMD(cmd);
                            }
                        }

                        if (!player->IsCoordinateInRange(2, update->Z, 150.0))
                        {
                            if (player->SetPlayerPos(2, update->Z))
                            {
                                FLAG(SKIPFLAG_GETPOS_Z);
                                player->ToggleNoOverride(SKIPFLAG_GETPOS_Z, true);

                                sprintf(pos, "%f", update->Z);

                                cmd = new fCommand;
                                cmd->command = "setpos Z " + string(pos);
                                cmd->refID = refID;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                cmd->flipcmd = SKIPFLAG_GETPOS_Z;
                                PUSHCMD(cmd);
                            }
                        }

                        if (update->A != player->GetPlayerAngle())
                        {
                            sprintf(pos, "%f", update->A);

                            cmd = new fCommand;
                            cmd->command = "setangle Z " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);

                            player->SetPlayerAngle(update->A);
                        }

                        if (update->alerted != player->IsPlayerAlerted())
                        {
                            cmd = new fCommand;
                            cmd->command = "setrestrained 0";
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->sleep = 150;

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
                            cmd->sleep = 150;
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

                        CLOSECMD();
                    }
                    break;
                }
                case ID_PLAYER_STATE_UPDATE:
                {
                    pPlayerStateUpdate* update = (pPlayerStateUpdate*) packet->data;

                    if (packet->length != sizeof(pPlayerStateUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(update->guid);
                    string refID = player->GetPlayerRefID();

                    #ifdef VAULTMP_DEBUG
                    char text[128];
                    ZeroMemory(text, sizeof(text));

                    sprintf(text, "Received player state update packet (name: %s, ref: %s, guid: %s)", player->GetPlayerName().c_str(), refID.c_str(), update->guid.ToString());
                    debug->Print(text, true);
                    #endif

                    fCommand* cmd;

                    if (refID.compare("none") != 0)
                    {
                        OPENCMD();

                        char pos[16];

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

                        if (update->health != player->GetPlayerHealth())
                        {
                            FLAG(SKIPFLAG_GETHEALTH);
                            player->ToggleNoOverride(SKIPFLAG_GETHEALTH, true);

                            sprintf(pos, "%i", (int) update->health);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue Health " + string(pos);
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->flipcmd = SKIPFLAG_GETHEALTH;
                            PUSHCMD(cmd);

                            player->SetPlayerHealth(update->health);
                        }

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

                        if (update->dead != player->IsPlayerDead())
                        {
                            FLAG(SKIPFLAG_GETDEAD);
                            player->ToggleNoOverride(SKIPFLAG_GETDEAD, true);

                            cmd = new fCommand;
                            cmd->refID = refID;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->flipcmd = SKIPFLAG_GETDEAD;
                            cmd->sleep = 150;

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
                                    cmd->sleep = 150;
                                    PUSHCMD(cmd);
                                    break;
                            }
                            player->SetPlayerDead(update->dead);
                        }

                        CLOSECMD();
                    }
                    break;
                }
                case ID_INVALID_PASSWORD:
                    MessageBox(NULL, "Dedicated server version mismatch.\nPlease download the most recent binaries from www.vaultmp.com", "Error", MB_OK | MB_ICONERROR);
                case ID_NO_FREE_INCOMING_CONNECTIONS:
                case ID_CONNECTION_ATTEMPT_FAILED:
                case ID_DISCONNECTION_NOTIFICATION:
                case ID_CONNECTION_BANNED:
                case ID_CONNECTION_LOST:
                    query = false;
                    break;
                }
            }

            /* Send updated data to server */

            if (self->UpdatePlayerUpdateStruct(&localPlayerUpdate))
            {
                peer->Send((char*) &localPlayerUpdate, sizeof(localPlayerUpdate), MEDIUM_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_PLAYER_UPDATE, addr, false, 0);

                #ifdef VAULTMP_DEBUG
                char text[128];
                ZeroMemory(text, sizeof(text));

                sprintf(text, "Sent player update packet (%s)", addr.ToString());
                debug->Print(text, true);
                #endif
            }

            if (self->UpdatePlayerStateUpdateStruct(&localPlayerStateUpdate))
            {
                peer->Send((char*) &localPlayerStateUpdate, sizeof(localPlayerStateUpdate), HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_PLAYER_STATE_UPDATE, addr, false, 0);

                #ifdef VAULTMP_DEBUG
                char text[128];
                ZeroMemory(text, sizeof(text));

                sprintf(text, "Sent player state update packet (%s)", addr.ToString());
                debug->Print(text, true);
                #endif
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

    #ifdef VAULTMP_DEBUG
    delete debug;
    #endif
}
