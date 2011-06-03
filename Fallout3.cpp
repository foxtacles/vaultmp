#include "Fallout3.h"

using namespace std;
using namespace RakNet;
using namespace pipe;

typedef HINSTANCE (__stdcall *fLoadLibrary)(char*);
typedef LPVOID (__stdcall *fGetProcAddress)(HINSTANCE, char*);
typedef void (*fDLLjump)(void);

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
    int moving;
};
#pragma pack(pop)

bool Fallout3::endThread = false;
bool Fallout3::wakeup = false;
HANDLE Fallout3::Fallout3pipethread;
HANDLE Fallout3::Fallout3gamethread;
Player* Fallout3::self;
queue<Player*> Fallout3::refqueue;
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
};

DWORD WINAPI Fallout3::InjectedCode(LPVOID addr)
{
	HINSTANCE hDll;
	fDLLjump DLLjump;
	INJECT* is = (INJECT*) addr;
	hDll = is->LoadLibrary(is->DLLpath);
	DLLjump = (fDLLjump) is->GetProcAddress(hDll, is->DLLjump);
	DLLjump();
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
                    else if (stricmp(token, "IsMoving") == 0)
                    {
                        token = strtok(NULL, ":.<> ");
                        int moving = atoi(token);
                        lastRef->SetPlayerMoving(moving);
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
                    string input = "op:";
                    input.append(high);
                    input.append(".setrestrained 1");
                    pipeServer->Send(&input);

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

            if (lookupProgramID("Fallout3.exe") == 0)
                endThread = true;
        } while (low.compare("ca:") != 0 && !endThread);
    }

    return ((DWORD) data);
}

DWORD WINAPI Fallout3::Fallout3game(LPVOID data)
{
    while (!endThread)
    {
        string input;

        input = "op:player.getpos X";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getpos Y";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getpos Z";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getangle Z";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getbaseactorvalue Health";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getactorvalue Health";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getactorvalue PerceptionCondition";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getactorvalue EnduranceCondition";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getactorvalue LeftAttackCondition";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getactorvalue RightAttackCondition";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getactorvalue LeftMobilityCondition";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getactorvalue RightMobilityCondition";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.getdead";
        pipeServer->Send(&input);
        Sleep(50);

        input = "op:player.ismoving";
        pipeServer->Send(&input);
        Sleep(50);

        map<RakNetGUID, string> players = Player::GetPlayerList();
        map<RakNetGUID, string>::iterator it;

        for (it = players.begin(); it != players.end(); it++)
        {
            string refID = it->second;

            if (refID.compare("none") != 0 && refID.compare("player") != 0)
            {
                input = "op:";
                input.append(refID);
                input.append(".getpos X");
                pipeServer->Send(&input);
                Sleep(50);

                input = "op:";
                input.append(refID);
                input.append(".getpos Y");
                pipeServer->Send(&input);
                Sleep(50);

                input = "op:";
                input.append(refID);
                input.append(".getpos Z");
                pipeServer->Send(&input);
                Sleep(50);

                input = "op:";
                input.append(refID);
                input.append(".getdead");
                pipeServer->Send(&input);
                Sleep(50);
            }
        }
    }

    return ((DWORD) data);
}

HANDLE Fallout3::InitalizeFallout3()
{
    FILE* Fallout3 = fopen("Fallout3.exe", "rb");

    if (Fallout3 != NULL)
    {
        fclose(Fallout3);

        FILE* vaultmp = fopen("vaultmp.dll", "rb");

        if (vaultmp != NULL)
        {
            fclose(vaultmp);

            if (lookupProgramID("Fallout3.exe") == 0)
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;

                ZeroMemory(&si, sizeof(si));
                ZeroMemory(&pi, sizeof(pi));
                si.cb = sizeof(si);

                if (CreateProcess("Fallout3.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
                {
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

                        codesize = (DWORD) InjectedEnd - (DWORD) InjectedCode;

                        start = VirtualAllocEx(hProc, 0, codesize + sizeof(INJECT), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
                        thread = (LPVOID) ((DWORD) start + sizeof(INJECT));

                        WriteProcessMemory(hProc, start, (LPVOID) &data, sizeof(INJECT), NULL);
                        WriteProcessMemory(hProc, thread, (LPVOID) InjectedCode, codesize, NULL);

                        CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE) thread, start, 0, 0);

                        /* Initalizing vaultmp.exe <-> Fallout3.exe pipe */

                        HANDLE PipeThread;
                        DWORD Fallout3pipeID;

                        PipeThread = CreateThread(NULL, 0, Fallout3pipe, (LPVOID) 0, 0, &Fallout3pipeID);

                        while (!wakeup) Sleep(2);

                        /* Resuming Fallout3.exe */

                        ResumeThread(pi.hThread);

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
                return NULL; // Fallout3.exe running
        }
        else
            return NULL; // vaultmp.dll missing
    }
    else
         return NULL; // Fallout3.exe missing

    return NULL;
}

void Fallout3::InitalizeVaultMP(RakPeerInterface* peer, SystemAddress addr, string name, string pwd)
{
    endThread = false;
    Fallout3pipethread = NULL;
    Fallout3gamethread = NULL;
    pipeClient = new PipeServer();
    pipeServer = new PipeClient();

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
    localPlayerUpdate.moving = 0;

    if (peer->Connect(addr.ToString(false), addr.port, 0, 0, 0, 0, 3, 500, 0) == CONNECTION_ATTEMPT_STARTED)
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

                    Fallout3pipethread = InitalizeFallout3();

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

                    string input("op:load "); // Load Fallout3 savegame
                    input.append(save);
                    pipeServer->Send(&input);

                    input = "op:player.removeallitems"; // Reset the players inventory
                    pipeServer->Send(&input);

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

                    string input("op:player.placeatme 30D82 1"); // Test
                    pipeServer->Send(&input);
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
                        string input = "op:";
                        input.append(refID);
                        input.append(".disable");
                        pipeServer->Send(&input);

                        input = "op:";
                        input.append(refID);
                        input.append(".markfordelete");
                        pipeServer->Send(&input);
                    }

                    delete player;
                    break;
                }
                case ID_PLAYER_UPDATE:
                {
                    pPlayerUpdate* update = (pPlayerUpdate*) packet->data;

                    Player* player = Player::GetPlayerFromGUID(update->guid);
                    string refID = player->GetPlayerRefID();

                    if (refID.compare("none") != 0)
                    {
                        string input;
                        char pos[16];

                        if (!player->IsPlayerNearPoint(update->X, update->Y, update->Z, 400.0))
                        {
                            input = "op:";
                            sprintf(pos, "%f", update->X);
                            input.append(refID);
                            input.append(".setpos X ");
                            input.append(pos);
                            pipeServer->Send(&input);

                            input = "op:";
                            sprintf(pos, "%f", update->Y);
                            input.append(refID);
                            input.append(".setpos Y ");
                            input.append(pos);
                            pipeServer->Send(&input);

                            input = "op:";
                            sprintf(pos, "%f", update->Z);
                            input.append(refID);
                            input.append(".setpos Z ");
                            input.append(pos);
                            pipeServer->Send(&input);

                            player->SetPlayerPos(0, update->X);
                            player->SetPlayerPos(1, update->Y);
                            player->SetPlayerPos(2, update->Z);
                        }

                        input = "op:";
                        sprintf(pos, "%f", update->A);
                        input.append(refID);
                        input.append(".setangle Z ");
                        input.append(pos);
                        pipeServer->Send(&input);
                        player->SetPlayerAngle(update->A);

                        if (update->moving != player->GetPlayerMoving())
                        {
                            input = "op:";
                            input.append(refID);
                            switch (update->moving)
                            {
                                case 0:
                                    input.append(".playgroup Idle 0");
                                    break;
                                case 1:
                                    input.append(".playgroup FastForward 1");
                                    break;
                                case 2:
                                    input.append(".playgroup FastBackward 1");
                                    break;
                                case 3:
                                    input.append(".playgroup FastLeft 1");
                                    break;
                                case 4:
                                    input.append(".playgroup FastRight 1");
                                    break;
                            }
                            pipeServer->Send(&input);
                            player->SetPlayerMoving(update->moving);
                        }

                        if (update->baseHealth != player->GetPlayerBaseHealth())
                        {
                            input = "op:";
                            sprintf(pos, "%i", (int) update->baseHealth);
                            input.append(refID);
                            input.append(".setactorvalue Health ");
                            input.append(pos);
                            pipeServer->Send(&input);
                            player->SetPlayerBaseHealth(update->baseHealth);
                        }

                        input = "op:";
                        sprintf(pos, "%i", (int) update->health);
                        input.append(refID);
                        input.append(".forceactorvalue Health ");
                        input.append(pos);
                        pipeServer->Send(&input);
                        player->SetPlayerHealth(update->health);

                        if (update->conds[0] != player->GetPlayerCondition(0))
                        {
                            input = "op:";
                            sprintf(pos, "%i", (int) update->conds[0]);
                            input.append(refID);
                            input.append(".forceactorvalue PerceptionCondition ");
                            input.append(pos);
                            pipeServer->Send(&input);
                            player->SetPlayerCondition(0, update->conds[0]);
                        }

                        if (update->conds[1] != player->GetPlayerCondition(1))
                        {
                            input = "op:";
                            sprintf(pos, "%i", (int) update->conds[1]);
                            input.append(refID);
                            input.append(".forceactorvalue EnduranceCondition ");
                            input.append(pos);
                            pipeServer->Send(&input);
                            player->SetPlayerCondition(1, update->conds[1]);
                        }

                        if (update->conds[2] != player->GetPlayerCondition(2))
                        {
                            input = "op:";
                            sprintf(pos, "%i", (int) update->conds[2]);
                            input.append(refID);
                            input.append(".forceactorvalue LeftAttackCondition ");
                            input.append(pos);
                            pipeServer->Send(&input);
                            player->SetPlayerCondition(2, update->conds[2]);
                        }

                        if (update->conds[3] != player->GetPlayerCondition(3))
                        {
                            input = "op:";
                            sprintf(pos, "%i", (int) update->conds[3]);
                            input.append(refID);
                            input.append(".forceactorvalue RightAttackCondition ");
                            input.append(pos);
                            pipeServer->Send(&input);
                            player->SetPlayerCondition(3, update->conds[3]);
                        }

                        if (update->conds[4] != player->GetPlayerCondition(4))
                        {
                            input = "op:";
                            sprintf(pos, "%i", (int) update->conds[4]);
                            input.append(refID);
                            input.append(".forceactorvalue LeftMobilityCondition ");
                            input.append(pos);
                            pipeServer->Send(&input);
                            player->SetPlayerCondition(4, update->conds[4]);
                        }

                        if (update->conds[5] != player->GetPlayerCondition(5))
                        {
                            input = "op:";
                            sprintf(pos, "%i", (int) update->conds[5]);
                            input.append(refID);
                            input.append(".forceactorvalue RightMobilityCondition ");
                            input.append(pos);
                            pipeServer->Send(&input);
                            player->SetPlayerCondition(5, update->conds[5]);
                        }

                        // more ActorValues

                        if (player->IsPlayerDead() != update->dead)
                        {
                            input = "op:";
                            input.append(refID);
                            switch (update->dead)
                            {
                                case true:
                                    input.append(".killactor");
                                    pipeServer->Send(&input);
                                    break;
                                case false:
                                    input.append(".resurrect 0");
                                    pipeServer->Send(&input);
                                    input = "op:";
                                    input.append(refID);
                                    input.append(".setrestrained 1");
                                    pipeServer->Send(&input);
                                    break;
                            }
                            player->SetPlayerDead(update->dead);
                        }
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
            int moving = self->GetPlayerMoving();

            if (X != localPlayerUpdate.X || Y != localPlayerUpdate.Y || Z != localPlayerUpdate.Z || A != localPlayerUpdate.A || health != localPlayerUpdate.health || baseHealth != localPlayerUpdate.baseHealth ||
                conds[0] != localPlayerUpdate.conds[0] || conds[1] != localPlayerUpdate.conds[1] || conds[2] != localPlayerUpdate.conds[2] || conds[3] != localPlayerUpdate.conds[3] || conds[4] != localPlayerUpdate.conds[4] ||
                conds[5] != localPlayerUpdate.conds[5] || dead != localPlayerUpdate.dead || moving != localPlayerUpdate.moving)
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
