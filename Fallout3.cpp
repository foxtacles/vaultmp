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
bool Fallout3::cmdmutex = false;
HANDLE Fallout3::Fallout3pipethread;
HANDLE Fallout3::Fallout3gamethread;
Player* Fallout3::self;
queue<Player*> Fallout3::refqueue;
list<fCommand*> Fallout3::cmdlist;
list<fCommand*> Fallout3::tmplist;
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
            //debug->Print(const_cast<char*>(recv.c_str()), false);
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

                    if (Fallout3_refID == 0x00000014)
                        lastRef = self;
                    else
                    {
                        char refstr[8];
                        ZeroMemory(refstr, sizeof(refstr));

                        sprintf(refstr, "%08x", Fallout3_refID);
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
                                        lastRef->SetPlayerPos(X_AXIS, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x59: // Y
                                        lastRef->SetPlayerPos(Y_AXIS, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x5A: // Z
                                        lastRef->SetPlayerPos(Z_AXIS, (float) *((double*) (&Fallout3_result)));
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
                                        lastRef->SetPlayerCondition(COND_PERCEPTION, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x1A: // EnduranceCondition
                                        lastRef->SetPlayerCondition(COND_ENDURANCE, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x1B: // LeftAttackCondition
                                        lastRef->SetPlayerCondition(COND_LEFTATTACK, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x1C: // RightAttackCondition
                                        lastRef->SetPlayerCondition(COND_RIGHTATTACK, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x1D: // LeftMobilityCondition
                                        lastRef->SetPlayerCondition(COND_LEFTMOBILITY, (float) *((double*) (&Fallout3_result)));
                                        break;
                                    case 0x1E: // RightMobilityCondition
                                        lastRef->SetPlayerCondition(COND_RIGHTMOBILITY, (float) *((double*) (&Fallout3_result)));
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
                                    lastRef->SetPlayerMoving(MOV_IDLE);
                                break;
                            }
                            case 0x1025: // PlaceAtMe
                            {
                                char refstr[8];
                                ZeroMemory(refstr, sizeof(refstr));

                                sprintf(refstr, "%08x", Fallout3_newRefID);

                                Player* player;

                                if (!refqueue.empty())
                                {
                                    player = refqueue.front();
                                    player->SetPlayerRefID(refstr);
                                    refqueue.pop();
                                }

                                OPENCMD();

                                fCommand* cmd = new fCommand;
                                cmd->command = "setrestrained 1";
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                cmd->sleep = 150;
                                PUSHCMD(cmd);

                                cmd = new fCommand;
                                cmd->command = "setname " + player->GetPlayerName();
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                PUSHCMD(cmd);

                                cmd = new fCommand;
                                cmd->command = "removeallitems";
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                PUSHCMD(cmd);

                                Inventory* handle = player->GetPlayerInventory();

                                if (handle != NULL && !handle->IsEmpty())
                                {
                                    list<Item*> items = handle->GetItemList();
                                    list<Item*>::iterator it;

                                    for (it = items.begin(); it != items.end(); ++it)
                                    {
                                        Item* item = *it;

                                        char amount[8];
                                        sprintf(amount, "%d", item->count);

                                        cmd = new fCommand;
                                        cmd->command = "additem " + string(item->item->first) + " ";
                                        cmd->command.append(amount);
                                        cmd->player = player;
                                        cmd->forplayers = false;
                                        cmd->repeat = false;
                                        PUSHCMD(cmd);

                                        if (item->worn)
                                        {
                                            cmd = new fCommand;
                                            cmd->command = "equipitem " + string(item->item->first) + " 1";
                                            cmd->player = player;
                                            cmd->forplayers = false;
                                            cmd->repeat = false;
                                            PUSHCMD(cmd);
                                        }
                                    }
                                }

                                CLOSECMD();

                                #ifdef VAULTMP_DEBUG
                                char text[128];
                                ZeroMemory(text, sizeof(text));

                                sprintf(text, "Received RefID from game: %s", refstr);
                                debug->Print(text, true);
                                #endif
                                break;
                            }
                            case 0x146D: // GetParentCell New Vegas
                            case 0x1495: // GetParentCell Fallout 3
                            {
                                DWORD cell = (DWORD) Fallout3_result;

                                if (lastRef != self && lastRef->GetPlayerRefID().compare("none") != 0)
                                {
                                    if (self->GetPlayerGameCell() == lastRef->GetPlayerNetworkCell() && lastRef->GetPlayerGameCell() != lastRef->GetPlayerNetworkCell())
                                    {
                                        #ifdef VAULTMP_DEBUG
                                        char text[128];
                                        ZeroMemory(text, sizeof(text));

                                        sprintf(text, "Moving player to cell (name: %s, ref: %s, cell: %x)", lastRef->GetPlayerName().c_str(), lastRef->GetPlayerRefID().c_str(), self->GetPlayerGameCell());
                                        debug->Print(text, true);
                                        #endif

                                        OPENCMD();

                                        lastRef->SetPlayerGameCell(self->GetPlayerGameCell());
                                        lastRef->ToggleNoOverride(SKIPFLAG_GETPARENTCELL, true);

                                        fCommand* cmd;

                                        if (lastRef->SetPlayerEnabled(true))
                                        {
                                            cmd = new fCommand;
                                            cmd->command = "enable";
                                            cmd->player = lastRef;
                                            cmd->forplayers = false;
                                            cmd->repeat = false;
                                            cmd->sleep = 150;
                                            PUSHCMD(cmd);
                                        }

                                        cmd = new fCommand;
                                        cmd->command = "moveto player";
                                        cmd->player = lastRef;
                                        cmd->forplayers = false;
                                        cmd->repeat = false;
                                        cmd->sleep = 150;
                                        PUSHCMD(cmd);

                                        CLOSECMD();
                                    }
                                }

                                lastRef->SetPlayerGameCell(cell);

                                if (lastRef != self && lastRef->GetPlayerRefID().compare("none") != 0)
                                {
                                    if (lastRef->GetPlayerNetworkCell() != self->GetPlayerGameCell() && !lastRef->IsPlayerNearPoint(self->GetPlayerPos(0), self->GetPlayerPos(1), self->GetPlayerPos(2), 20000.0))
                                    {
                                        if (lastRef->SetPlayerEnabled(false))
                                        {
                                            OPENCMD();

                                            fCommand* cmd = new fCommand;
                                            cmd->command = "disable";
                                            cmd->player = lastRef;
                                            cmd->forplayers = false;
                                            cmd->repeat = false;
                                            cmd->enabledonly = false;
                                            cmd->sleep = 150;
                                            PUSHCMD(cmd);

                                            CLOSECMD();
                                        }
                                    }
                                    else
                                    {
                                        if (lastRef->SetPlayerEnabled(true))
                                        {
                                            OPENCMD();

                                            fCommand* cmd = new fCommand;
                                            cmd->command = "enable";
                                            cmd->player = lastRef;
                                            cmd->forplayers = false;
                                            cmd->repeat = false;
                                            cmd->sleep = 150;
                                            PUSHCMD(cmd);

                                            CLOSECMD();
                                        }
                                    }
                                }

                                break;
                            }
                            case 0x1007: // SetPos
                            {
                                switch (Fallout3_setcoord)
                                {
                                    case 0x58: // X
                                        lastRef->ToggleNoOverride(SKIPFLAG_GETPOS_X, false);
                                        break;
                                    case 0x59: // Y
                                        lastRef->ToggleNoOverride(SKIPFLAG_GETPOS_Y, false);
                                        break;
                                    case 0x5A: // Z
                                        lastRef->ToggleNoOverride(SKIPFLAG_GETPOS_Z, false);
                                        break;
                                }
                                break;
                            }
                            case 0x1009: // SetAngle
                            {
                                switch (Fallout3_setcoord)
                                {
                                    case 0x58: // X
                                        break;
                                    case 0x59: // Y
                                        break;
                                    case 0x5A: // Z
                                        break;
                                }
                                break;
                            }
                            case 0x108C: // Resurrect
                            case 0x108B: // KillActor
                            {
                                lastRef->ToggleNoOverride(SKIPFLAG_GETDEAD, false);
                                break;
                            }
                            case 0x109E: // MoveTo
                            {
                                lastRef->ToggleNoOverride(SKIPFLAG_GETPARENTCELL, false);
                                break;
                            }
                            case 0x110E: // ForceActorValue
                            {
                                switch (Fallout3_valcoord)
                                {
                                    case 0x10: // Health
                                        lastRef->ToggleNoOverride(SKIPFLAG_GETHEALTH, false);
                                        break;
                                }
                                break;
                            }
                            case 0x10F3: // SetRestrained
                            case 0x10AD: // RemoveAllItems
                            case 0x1002: // AddItem
                            case 0x10EE: // EquipItem
                            case 0x10EF: // UnequipItem
                            case 0x1013: // PlayGroup
                            case 0x1021: // Enable
                            case 0x1022: // Disable
                                break; // more stuff to do here
                            case 0x11BB: // MarkForDelete
                            {
                                delete lastRef;
                                break;
                            }
                            case 0x1485: // SetName Fallout 3
                            case 0x144C: // SetName New Vegas
                            {
                                // player name was set in game
                                break;
                            }
                            case 0x014F: // LoadGame
                            {
                                // we successfully loaded the savegame! do something here.
                                break;
                            }
                            case 0x0114: // ShowAnim
                            {
                                // has been handled by string parser
                                break;
                            }
                            case 0x019B: // ShowInventory New Vegas
                            case 0x019C: // ShowInventory Fallout 3
                            {
                                // has been handled by string parser

                                lastRef->SetPlayerInventory(Inventory::TransferInventory());
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

                    int count = 0;
                    int iLastRef = 0;
                    int wantsdrawn = 0;
                    int weapdrawn = 0;
                    char action[64];

                    ZeroMemory(action, sizeof(action));

                    if (sscanf(output, "%d", &count))
                    {
                        char* data = strchr(output, '(');

                        int item = 0;
                        int type = 0;
                        float condition = 0.00;
                        char worn[8];

                        ZeroMemory(worn, sizeof(worn));

                        if (data != NULL && sscanf(data, "(%X) (%d,%f%%) - %s", &item, &type, &condition, worn))
                        {
                            char basestr[8];
                            ZeroMemory(basestr, sizeof(basestr));

                            bool bWorn = (stricmp(worn, "Worn") == 0) ? true : false;
                            sprintf(basestr, "%08x", item);

                            if (!Inventory::AddItem_Internal(string(basestr), count, type, condition, bWorn))
                            {
                                // Item not registered in database, send error to server
                            }
                        }
                    }
                    else if (sscanf(output, "Wants Weapon Drawn %d, Weapon Drawn %d", &wantsdrawn, &weapdrawn))
                    {
                        self->SetPlayerAlerted((bool) weapdrawn);
                    }
                    else if (sscanf(output, "Movement -> %s", action))
                    {
                        token = strtok(action, "/");

                        if (stricmp(token, "FastForward") == 0)
                            self->SetPlayerMoving(MOV_FASTFORWARD);
                        else if (stricmp(token, "FastBackward") == 0)
                            self->SetPlayerMoving(MOV_FASTBACKWARD);
                        else if (stricmp(token, "FastLeft") == 0)
                            self->SetPlayerMoving(MOV_FASTLEFT);
                        else if (stricmp(token, "FastRight") == 0)
                            self->SetPlayerMoving(MOV_FASTRIGHT);
                        else if (stricmp(token, "Forward") == 0)
                            self->SetPlayerMoving(MOV_FORWARD);
                        else if (stricmp(token, "Backward") == 0)
                            self->SetPlayerMoving(MOV_BACKWARD);
                        else if (stricmp(token, "Left") == 0)
                            self->SetPlayerMoving(MOV_LEFT);
                        else if (stricmp(token, "Right") == 0)
                            self->SetPlayerMoving(MOV_RIGHT);
                        else
                        {
                            token = strtok(NULL, "/");

                            if (token != NULL)
                            {
                                if (stricmp(token, "FastForward") == 0)
                                    self->SetPlayerMoving(MOV_FASTFORWARD);
                                else if (stricmp(token, "FastBackward") == 0)
                                    self->SetPlayerMoving(MOV_FASTBACKWARD);
                                else if (stricmp(token, "FastLeft") == 0)
                                    self->SetPlayerMoving(MOV_FASTLEFT);
                                else if (stricmp(token, "FastRight") == 0)
                                    self->SetPlayerMoving(MOV_FASTRIGHT);
                                else if (stricmp(token, "Forward") == 0)
                                    self->SetPlayerMoving(MOV_FORWARD);
                                else if (stricmp(token, "Backward") == 0)
                                    self->SetPlayerMoving(MOV_BACKWARD);
                                else if (stricmp(token, "Left") == 0)
                                    self->SetPlayerMoving(MOV_LEFT);
                                else if (stricmp(token, "Right") == 0)
                                    self->SetPlayerMoving(MOV_RIGHT);
                            }
                        }
                    }
                    /*else
                    {
                        #ifdef VAULTMP_DEBUG
                        char text[256];
                        ZeroMemory(text, sizeof(text));

                        sprintf(text, "String could not be processed: %s", const_cast<char*>(high.c_str()));
                        debug->Print(text, true);
                        #endif
                    }*/
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

    #ifdef VAULTMP_DEBUG
    debug->Print((char*) "Game thread is going to terminate...", true);
    #endif

    return ((DWORD) data);
}

DWORD WINAPI Fallout3::Fallout3game(LPVOID data)
{
    fCommand cmds[17];

    cmds[0].command = "getpos X";
    cmds[0].repeat = true;
    cmds[0].forplayers = true;
    cmds[0].skipflag = SKIPFLAG_GETPOS_X;

    cmds[1].command = "getpos Y";
    cmds[1].repeat = true;
    cmds[1].forplayers = true;
    cmds[1].skipflag = SKIPFLAG_GETPOS_Y;

    cmds[2].command = "getpos Z";
    cmds[2].repeat = true;
    cmds[2].forplayers = true;
    cmds[2].skipflag = SKIPFLAG_GETPOS_Z;

    cmds[3].command = "getparentcell";
    cmds[3].repeat = true;
    cmds[3].forplayers = true;
    cmds[3].enabledonly = false;
    cmds[3].skipflag = SKIPFLAG_GETPARENTCELL;

    cmds[4].command = "getangle Z";
    cmds[4].repeat = true;
    cmds[4].player = self;

    cmds[5].command = "getbaseactorvalue Health";
    cmds[5].repeat = true;
    cmds[5].player = self;
    cmds[5].priority = 50;

    cmds[6].command = "getactorvalue Health";
    cmds[6].repeat = true;
    cmds[6].player = self;
    cmds[6].priority = 10;
    cmds[6].skipflag = SKIPFLAG_GETHEALTH;

    cmds[7].command = "getactorvalue PerceptionCondition";
    cmds[7].repeat = true;
    cmds[7].player = self;
    cmds[7].priority = 15;

    cmds[8].command = "getactorvalue EnduranceCondition";
    cmds[8].repeat = true;
    cmds[8].player = self;
    cmds[8].priority = 15;

    cmds[9].command = "getactorvalue LeftAttackCondition";
    cmds[9].repeat = true;
    cmds[9].player = self;
    cmds[9].priority = 15;

    cmds[10].command = "getactorvalue RightAttackCondition";
    cmds[10].repeat = true;
    cmds[10].player = self;
    cmds[10].priority = 15;

    cmds[11].command = "getactorvalue LeftMobilityCondition";
    cmds[11].repeat = true;
    cmds[11].player = self;
    cmds[11].priority = 15;

    cmds[12].command = "getactorvalue RightMobilityCondition";
    cmds[12].repeat = true;
    cmds[12].player = self;
    cmds[12].priority = 15;

    cmds[13].command = "getdead";
    cmds[13].repeat = true;
    cmds[13].forplayers = true;
    cmds[13].priority = 10;
    cmds[13].skipflag = SKIPFLAG_GETDEAD;

    cmds[14].command = "showanim";
    cmds[14].repeat = true;
    cmds[14].player = self;

    cmds[15].command = "showinventory";
    cmds[15].repeat = true;
    cmds[15].player = self;
    cmds[15].priority = 10;

    cmds[16].command = "ismoving";
    cmds[16].repeat = true;
    cmds[16].player = self;

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

            if (cmd->curPriority != cmd->priority)
            {
                cmd->curPriority++;
                ++it;
                continue;
            }
            else
                cmd->curPriority = 0;

            if (cmd->player == NULL && !cmd->forplayers)
            {
                string input = "op:" + cmd->command;
                pipeServer->Send(&input);
                Sleep(cmd->sleep);
            }
            else if (cmd->forplayers)
            {
                map<RakNetGUID, string> players = Player::GetPlayerList();
                map<RakNetGUID, string>::iterator it2;

                for (it2 = players.begin(); it2 != players.end(); ++it2)
                {
                    Player* player = Player::GetPlayerFromGUID(it2->first);
                    string refID = it2->second;

                    if ((cmd->skipflag != -1 && player->GetPlayerOverrideFlag(cmd->skipflag) == true) || (cmd->enabledonly && player->GetPlayerEnabled() == false))
                        continue;

                    if (refID.compare("none") != 0)
                    {
                        string input = "op:" + refID + "." + cmd->command;
                        pipeServer->Send(&input);
                        Sleep(cmd->sleep);
                    }
                }
            }
            else
            {
                if ((cmd->skipflag != -1 && cmd->player->GetPlayerOverrideFlag(cmd->skipflag) == true) || (cmd->enabledonly && cmd->player->GetPlayerEnabled() == false))
                {
                    ++it;
                    continue;
                }

                string input = "op:" + cmd->player->GetPlayerRefID() + "." + cmd->command;
                pipeServer->Send(&input);
                Sleep(cmd->sleep);
            }

            #ifdef VAULTMP_DEBUG
            char text[128];
            ZeroMemory(text, sizeof(text));

            sprintf(text, "Executing command (%s on %s, sleep: %d, timediff: %dms)", cmd->command.c_str(), cmd->forplayers ? "all" : cmd->player != NULL ? cmd->player->GetPlayerRefID().c_str() : "-", cmd->sleep, !cmd->repeat ? (GetTickCount() - cmd->tcount) : -1);
            debug->Print(text, true);
            #endif

            if (!cmd->repeat)
            {
                delete cmd;
                it = cmdlist.erase(it);
            }
            else
                ++it;
        }
    }

    #ifdef VAULTMP_DEBUG
    debug->Print((char*) "Command thread is going to terminate...", true);
    #endif

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

        if (Fallout3::NewVegas ? (size == FALLOUTNV_EXE1_SIZE || size == FALLOUTNV_EXE2_SIZE || size == FALLOUTNV_EXE3_SIZE) : (size == FALLOUT3_EXE_SIZE))
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

                        if (size == VAULTMP_DLL_SIZE)
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
    Fallout3::NewVegas = NewVegas;

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
    Inventory::SetDebugHandler(debug);
    #endif

    Player::DestroyInstances();
    self = new Player(peer->GetMyGUID());
    self->SetPlayerName(name);
    self->SetPlayerRefID("player");

    Inventory::Cleanup();
    Inventory::Initialize(Fallout3::NewVegas);

    Inventory localPlayerInventory;

    pPlayerUpdate localPlayerUpdate = self->GetPlayerUpdateStruct();
    pPlayerStateUpdate localPlayerStateUpdate = self->GetPlayerStateUpdateStruct();
    pPlayerCellUpdate localPlayerCellUpdate = self->GetPlayerCellUpdateStruct();
    list<pPlayerItemUpdate> localPlayerItemUpdate;

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
                    cmd->forplayers = false;
                    cmd->repeat = false;
                    cmd->sleep = 150;
                    PUSHCMD(cmd);

                    cmd = new fCommand;
                    cmd->command = "setname " + self->GetPlayerName();
                    cmd->player = self;
                    cmd->forplayers = false;
                    cmd->repeat = false;
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
                    cmd->player = self;
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
                        cmd->player = player;
                        cmd->forplayers = false;
                        cmd->repeat = false;
                        cmd->sleep = 150;
                        PUSHCMD(cmd);

                        cmd = new fCommand;
                        cmd->command = "markfordelete";
                        cmd->player = player;
                        cmd->forplayers = false;
                        cmd->repeat = false;
                        cmd->sleep = 150;
                        PUSHCMD(cmd);

                        CLOSECMD();
                    }
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

                        if (player->SetPlayerAngle(update->A) && player->GetPlayerEnabled())
                        {
                            sprintf(pos, "%f", update->A);

                            cmd = new fCommand;
                            cmd->command = "setangle Z " + string(pos);
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            PUSHCMD(cmd);
                        }

                        if (player->SetPlayerMoving(update->moving) && player->GetPlayerEnabled())
                        {
                            cmd = new fCommand;
                            cmd->command = "playgroup ";
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;

                            switch (update->moving)
                            {
                                case MOV_IDLE:
                                    cmd->command.append("Idle 0");
                                    break;
                                case MOV_FASTFORWARD:
                                    cmd->command.append("FastForward 1");
                                    break;
                                case MOV_FASTBACKWARD:
                                    cmd->command.append("FastBackward 1");
                                    break;
                                case MOV_FASTLEFT:
                                    cmd->command.append("FastLeft 1");
                                    break;
                                case MOV_FASTRIGHT:
                                    cmd->command.append("FastRight 1");
                                    break;
                                case MOV_FORWARD:
                                    cmd->command.append("Forward 1");
                                    break;
                                case MOV_BACKWARD:
                                    cmd->command.append("Backward 1");
                                    break;
                                case MOV_LEFT:
                                    cmd->command.append("Left 1");
                                    break;
                                case MOV_RIGHT:
                                    cmd->command.append("Right 1");
                                    break;
                            }

                            PUSHCMD(cmd);
                        }

                        if (!player->IsCoordinateInRange(X_AXIS, update->X, 350.0))
                        {
                            if (player->SetPlayerPos(X_AXIS, update->X) && player->GetPlayerEnabled())
                            {
                                player->ToggleNoOverride(SKIPFLAG_GETPOS_X, true);

                                sprintf(pos, "%f", update->X);

                                cmd = new fCommand;
                                cmd->command = "setpos X " + string(pos);
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                PUSHCMD(cmd);
                            }
                        }

                        if (!player->IsCoordinateInRange(Y_AXIS, update->Y, 350.0))
                        {
                            if (player->SetPlayerPos(Y_AXIS, update->Y) && player->GetPlayerEnabled())
                            {
                                player->ToggleNoOverride(SKIPFLAG_GETPOS_Y, true);

                                sprintf(pos, "%f", update->Y);

                                cmd = new fCommand;
                                cmd->command = "setpos Y " + string(pos);
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                PUSHCMD(cmd);
                            }
                        }

                        if (!player->IsCoordinateInRange(Z_AXIS, update->Z, 200.0))
                        {
                            if (player->SetPlayerPos(Z_AXIS, update->Z) && player->GetPlayerEnabled())
                            {
                                player->ToggleNoOverride(SKIPFLAG_GETPOS_Z, true);

                                sprintf(pos, "%f", update->Z);

                                cmd = new fCommand;
                                cmd->command = "setpos Z " + string(pos);
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                PUSHCMD(cmd);
                            }
                        }

                        /*
                        if (player->GetPlayerMoving() == 0 && !player->IsPlayerNearPoint(update->X, update->Y, update->Z, 400.0) && player->GetPlayerEnabled())
                        {
                            if (player->SetPlayerPos(0, update->X))
                            {
                                player->ToggleNoOverride(SKIPFLAG_GETPOS_X, true);

                                sprintf(pos, "%f", update->X);

                                cmd = new fCommand;
                                cmd->command = "setpos X " + string(pos);
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                PUSHCMD(cmd);
                            }

                            if (player->SetPlayerPos(1, update->Y))
                            {
                                player->ToggleNoOverride(SKIPFLAG_GETPOS_Y, true);

                                sprintf(pos, "%f", update->Y);

                                cmd = new fCommand;
                                cmd->command = "setpos Y " + string(pos);
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                PUSHCMD(cmd);
                            }
                        }
                        */

                        if (player->SetPlayerAlerted(update->alerted) && player->GetPlayerEnabled())
                        {
                            cmd = new fCommand;
                            cmd->command = "setrestrained 0";
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->sleep = 150;
                            PUSHCMD(cmd);

                            cmd = new fCommand;
                            cmd->command = "setalert ";
                            cmd->player = player;
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
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->sleep = 150;
                            PUSHCMD(cmd);
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

                        if (player->SetPlayerBaseHealth(update->baseHealth))
                        {
                            sprintf(pos, "%i", (int) update->baseHealth);

                            cmd = new fCommand;
                            cmd->command = "setactorvalue Health " + string(pos);
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;
                            PUSHCMD(cmd);
                        }

                        if (player->SetPlayerHealth(update->health))
                        {
                            player->ToggleNoOverride(SKIPFLAG_GETHEALTH, true);

                            sprintf(pos, "%i", (int) update->health);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue Health " + string(pos);
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;
                            PUSHCMD(cmd);
                        }

                        if (player->SetPlayerCondition(COND_PERCEPTION, update->conds[0]))
                        {
                            sprintf(pos, "%i", (int) update->conds[0]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue PerceptionCondition " + string(pos);
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;
                            PUSHCMD(cmd);
                        }

                        if (player->SetPlayerCondition(COND_ENDURANCE, update->conds[1]))
                        {
                            sprintf(pos, "%i", (int) update->conds[1]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue EnduranceCondition " + string(pos);
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;
                            PUSHCMD(cmd);
                        }

                        if (player->SetPlayerCondition(COND_LEFTATTACK, update->conds[2]))
                        {
                            sprintf(pos, "%i", (int) update->conds[2]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue LeftAttackCondition " + string(pos);
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;
                            PUSHCMD(cmd);
                        }

                        if (player->SetPlayerCondition(COND_RIGHTATTACK, update->conds[3]))
                        {
                            sprintf(pos, "%i", (int) update->conds[3]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue RightAttackCondition " + string(pos);
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;
                            PUSHCMD(cmd);
                        }

                        if (player->SetPlayerCondition(COND_LEFTMOBILITY, update->conds[4]))
                        {
                            sprintf(pos, "%i", (int) update->conds[4]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue LeftMobilityCondition " + string(pos);
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;
                            PUSHCMD(cmd);
                        }

                        if (player->SetPlayerCondition(COND_RIGHTMOBILITY, update->conds[5]))
                        {
                            sprintf(pos, "%i", (int) update->conds[5]);

                            cmd = new fCommand;
                            cmd->command = "forceactorvalue RightMobilityCondition " + string(pos);
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;
                            PUSHCMD(cmd);
                        }

                        if (player->SetPlayerDead(update->dead))
                        {
                            player->ToggleNoOverride(SKIPFLAG_GETDEAD, true);

                            cmd = new fCommand;
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;
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
                                    cmd->player = player;
                                    cmd->forplayers = false;
                                    cmd->repeat = false;
                                    cmd->enabledonly = false;
                                    cmd->sleep = 150;
                                    PUSHCMD(cmd);
                                    break;
                            }
                        }

                        CLOSECMD();
                    }
                    break;
                }
                case ID_PLAYER_CELL_UPDATE:
                {
                    pPlayerCellUpdate* update = (pPlayerCellUpdate*) packet->data;

                    if (packet->length != sizeof(pPlayerCellUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(update->guid);
                    string refID = player->GetPlayerRefID();

                    #ifdef VAULTMP_DEBUG
                    char text[128];
                    ZeroMemory(text, sizeof(text));

                    sprintf(text, "Received player cell update packet (name: %s, ref: %s, guid: %s)", player->GetPlayerName().c_str(), refID.c_str(), update->guid.ToString());
                    debug->Print(text, true);
                    #endif

                    if (refID.compare("none") != 0)
                    {
                        player->SetPlayerNetworkCell(update->cell);
                    }
                    break;
                }
                case ID_PLAYER_ITEM_UPDATE:
                {
                    pPlayerItemUpdate* update = (pPlayerItemUpdate*) packet->data;

                    if (packet->length != sizeof(pPlayerItemUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(update->guid);
                    string refID = player->GetPlayerRefID();

                    #ifdef VAULTMP_DEBUG
                    char text[128];
                    ZeroMemory(text, sizeof(text));

                    sprintf(text, "Received player item update packet (name: %s, ref: %s, guid: %s)", player->GetPlayerName().c_str(), refID.c_str(), update->guid.ToString());
                    debug->Print(text, true);
                    #endif

                    fCommand* cmd;

                    if (refID.compare("none") != 0)
                    {
                        OPENCMD();

                        string baseID = string(update->baseID);
                        char amount[8];
                        sprintf(amount, "%d", abs(update->item.count));

                        if (update->item.count > 0)
                        {
                            cmd = new fCommand;
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;

                            cmd->command = "additem " + baseID + " ";
                            cmd->command.append(amount);
                            if (update->hidden) cmd->command.append(" 1");
                            else cmd->command.append(" 0");
                            PUSHCMD(cmd);
                        }
                        else if (update->item.count < 0)
                        {
                            cmd = new fCommand;
                            cmd->player = player;
                            cmd->forplayers = false;
                            cmd->repeat = false;
                            cmd->enabledonly = false;

                            cmd->command = "removeitem " + baseID + " ";
                            cmd->command.append(amount);
                            if (update->hidden) cmd->command.append(" 1");
                            else cmd->command.append(" 0");
                            PUSHCMD(cmd);
                        }

                        if (update->item.count >= 0)
                        {
                            if (update->item.worn)
                            {
                                cmd = new fCommand;
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                cmd->enabledonly = false;

                                cmd->command = "equipitem " + baseID + " ";
                                if (player != self) cmd->command.append("1");
                                else cmd->command.append("0");
                                if (update->hidden) cmd->command.append(" 1");
                                else cmd->command.append(" 0");

                                PUSHCMD(cmd);
                            }
                            else/* if (update->item.condition != 0.00)*/
                            {
                                cmd = new fCommand;

                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                cmd->enabledonly = false;

                                cmd->command = "unequipitem " + baseID + " ";
                                if (player != self) cmd->command.append("1");
                                else cmd->command.append("0");
                                if (update->hidden) cmd->command.append(" 1");
                                else cmd->command.append(" 0");

                                PUSHCMD(cmd);
                            }
                        }

                        CLOSECMD();
                    }

                    if (player != self)
                    {
                        Inventory* handle = player->GetPlayerInventory();

                        if (handle == NULL)
                        {
                            handle = new Inventory();
                            player->SetPlayerInventory(handle);
                        }

                        if (update->item.count == 0)
                        {
                            if (!handle->UpdateItem(string(update->baseID), update->item.condition, update->item.worn))
                            {

                            }
                        }
                        else if (update->item.count > 0)
                        {
                            if (!handle->AddItem(string(update->baseID), update->item.count, update->item.type, update->item.condition, update->item.worn))
                            {

                            }
                        }
                        else if (update->item.count < 0)
                        {
                            if (!handle->RemoveItem(string(update->baseID), abs(update->item.count)))
                            {

                            }
                        }
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

            if (self->UpdatePlayerCellUpdateStruct(&localPlayerCellUpdate))
            {
                peer->Send((char*) &localPlayerCellUpdate, sizeof(localPlayerCellUpdate), HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_PLAYER_CELL_UPDATE, addr, false, 0);

                #ifdef VAULTMP_DEBUG
                char text[128];
                ZeroMemory(text, sizeof(text));

                sprintf(text, "Sent player cell update packet (%s)", addr.ToString());
                debug->Print(text, true);
                #endif
            }

            if (self->UpdatePlayerItemUpdateStruct(&localPlayerItemUpdate, &localPlayerInventory))
            {
                list<pPlayerItemUpdate>::iterator it;

                for (it = localPlayerItemUpdate.begin(); it != localPlayerItemUpdate.end(); ++it)
                    peer->Send((char*) &(*it), sizeof(pPlayerItemUpdate), HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_PLAYER_ITEM_UPDATE, addr, false, 0);

                #ifdef VAULTMP_DEBUG
                char text[128];
                ZeroMemory(text, sizeof(text));

                sprintf(text, "Sent %d player item update packets (%s)", localPlayerItemUpdate.size(), addr.ToString());
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
    debug->Print((char*) "Network thread is going to terminate...", true);
    delete debug;
    #endif
}
