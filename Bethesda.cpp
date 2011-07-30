#include "Bethesda.h"

using namespace std;
using namespace RakNet;

typedef HINSTANCE (__stdcall *fLoadLibrary)(char*);
typedef LPVOID (__stdcall *fGetProcAddress)(HINSTANCE, char*);
typedef void (*fDLLjump)(bool);

bool Bethesda::NewVegas = false;
bool Bethesda::initialized = false;
string Bethesda::savegame = "";
Player* Bethesda::self = NULL;
list<Player*> Bethesda::refqueue;

#ifdef VAULTMP_DEBUG
Debug* Bethesda::debug;
#endif

struct Bethesda::INJECT
{
    fLoadLibrary LoadLibrary;
    fGetProcAddress GetProcAddress;
    char DLLpath[256];
    char DLLjump[16];
    bool NewVegas;
};

DWORD WINAPI Bethesda::InjectedCode(LPVOID addr)
{
    HINSTANCE hDll;
    fDLLjump DLLjump;
    INJECT* is = (INJECT*) addr;
    hDll = is->LoadLibrary(is->DLLpath);
    DLLjump = (fDLLjump) is->GetProcAddress(hDll, is->DLLjump);
    DLLjump(is->NewVegas);
    return 0;
}

void Bethesda::InjectedEnd()
{
    /* This is to calculate the size of InjectedCode */
}

void Bethesda::CommandHandler(vector<void*> command)
{
    if (command.size() != 7)
        return;

    unsigned long long Fallout3_result = 0x00;
    DWORD Fallout3_opcode = 0x00;
    DWORD Fallout3_refID = 0x00;
    DWORD Fallout3_newRefID = 0x00;
    unsigned char Fallout3_coord = 0x00;
    unsigned char Fallout3_setcoord = 0x00;
    unsigned char Fallout3_valcoord = 0x00;

    Fallout3_opcode = *(reinterpret_cast<DWORD*>(command[0]));
    Fallout3_refID = *(reinterpret_cast<DWORD*>(command[1]));
    Fallout3_result = *(reinterpret_cast<unsigned long long*>(command[2]));
    Fallout3_coord = *(reinterpret_cast<unsigned char*>(command[3]));
    Fallout3_setcoord = *(reinterpret_cast<unsigned char*>(command[4]));
    Fallout3_valcoord = *(reinterpret_cast<unsigned char*>(command[5]));
    Fallout3_newRefID = *(reinterpret_cast<DWORD*>(command[6]));

    Player* lastRef = NULL;

    if (Fallout3_refID == PLAYER_REFERENCE)
        lastRef = self;
    else
    {
        char refstr[16];
        snprintf(refstr, sizeof(refstr), "%08x", Fallout3_refID);
        lastRef = Player::GetPlayerFromRefID(refstr);
    }

    if (lastRef != NULL)
    {
        lastRef->StartSession();

        switch (Fallout3_opcode)
        {
        case 0x1006: // GetPos
        {
            switch (Fallout3_coord)
            {
            case 0x58: // X
                lastRef->SetActorPos(X_AXIS, (float) *((double*) (&Fallout3_result)));
                break;
            case 0x59: // Y
                lastRef->SetActorPos(Y_AXIS, (float) *((double*) (&Fallout3_result)));
                break;
            case 0x5A: // Z
                lastRef->SetActorPos(Z_AXIS, (float) *((double*) (&Fallout3_result)));
                break;
            }
            break;
        }
        case 0x1008: // GetAngle
        {
            switch (Fallout3_coord)
            {
            case 0x5A: // Z
                lastRef->SetActorAngle((float) *((double*) (&Fallout3_result)));
                break;
            }
            break;
        }
        case 0x1115: // GetBaseActorValue
        {
            switch (Fallout3_coord)
            {
            case 0x10: // Health
                lastRef->SetActorBaseHealth((float) *((double*) (&Fallout3_result)));
                break;
            }
            break;
        }
        case 0x100E: // GetActorValue
        {
            switch (Fallout3_coord)
            {
            case 0x10: // Health
                lastRef->SetActorHealth((float) *((double*) (&Fallout3_result)));
                break;
            case 0x19: // PerceptionCondition
                lastRef->SetActorCondition(COND_PERCEPTION, (float) *((double*) (&Fallout3_result)));
                break;
            case 0x1A: // EnduranceCondition
                lastRef->SetActorCondition(COND_ENDURANCE, (float) *((double*) (&Fallout3_result)));
                break;
            case 0x1B: // LeftAttackCondition
                lastRef->SetActorCondition(COND_LEFTATTACK, (float) *((double*) (&Fallout3_result)));
                break;
            case 0x1C: // RightAttackCondition
                lastRef->SetActorCondition(COND_RIGHTATTACK, (float) *((double*) (&Fallout3_result)));
                break;
            case 0x1D: // LeftMobilityCondition
                lastRef->SetActorCondition(COND_LEFTMOBILITY, (float) *((double*) (&Fallout3_result)));
                break;
            case 0x1E: // RightMobilityCondition
                lastRef->SetActorCondition(COND_RIGHTMOBILITY, (float) *((double*) (&Fallout3_result)));
                break;
            }
            break;
        }
        case 0x102E: // GetDead
        {
            lastRef->SetActorDead((bool) Fallout3_result);
            break;
        }
        case 0x1019: // IsMoving
        {
            if (Fallout3_result == 0x00)
                lastRef->SetActorMoving(MOV_IDLE);
            break;
        }
        case 0x1025: // PlaceAtMe
        {
            char refstr[16];
            snprintf(refstr, sizeof(refstr), "%08x", Fallout3_newRefID);

            Player* player;

            if (!refqueue.empty())
            {
                player = refqueue.front();
                player->SetReference(refstr);
                player->SetActorEnabled(true);
                player->ToggleNoOverride(SKIPFLAG_GETDEAD, false);
                player->SetActorDead(false);
                refqueue.pop_front();
            }
            else
                break;

            Command::StartSession();

            ParamList param_SetRestrained;
            param_SetRestrained.push_back(player->GetActorRefParam());
            param_SetRestrained.push_back(Data::Param_True);
            ParamContainer SetRestrained = ParamContainer(param_SetRestrained, &Data::AlwaysTrue);
            Command::ExecuteCommandOnce("SetRestrained", SetRestrained, 0, 150);

            ParamList param_SetName;
            param_SetName.push_back(player->GetActorRefParam());
            param_SetName.push_back(player->GetActorNameParam());
            ParamContainer SetName = ParamContainer(param_SetName, &Data::AlwaysTrue);
            Command::ExecuteCommandOnce("SetName", SetName);

            ParamList param_RemoveAllItems;
            param_RemoveAllItems.push_back(player->GetActorRefParam());
            ParamContainer RemoveAllItems = ParamContainer(param_RemoveAllItems, &Data::AlwaysTrue);
            Command::ExecuteCommandOnce("RemoveAllItems", RemoveAllItems);

            if (!player->IsEmpty())
            {
                list<ParamList> params = player->GetItemParamList_AddItem(true);
                list<ParamList>::iterator it;

                for (it = params.begin(); it != params.end(); ++it)
                {
                    ParamList param_AddItem;
                    param_AddItem.push_back(player->GetActorRefParam());
                    param_AddItem.splice(param_AddItem.end(), *it);
                    ParamContainer AddItem = ParamContainer(param_AddItem, &Data::AlwaysTrue);
                    Command::ExecuteCommandOnce("AddItem", AddItem);
                }

                params = player->GetItemParamList_EquipItem(true, true);

                for (it = params.begin(); it != params.end(); ++it)
                {
                    ParamList param_EquipItem;
                    param_EquipItem.push_back(player->GetActorRefParam());
                    param_EquipItem.splice(param_EquipItem.end(), *it);
                    ParamContainer EquipItem = ParamContainer(param_EquipItem, &Data::AlwaysTrue);
                    Command::ExecuteCommandOnce("EquipItem", EquipItem);
                }
            }

            Command::EndSession();

#ifdef VAULTMP_DEBUG
            char text[128];
            snprintf(text, sizeof(text), "Received RefID %s for actor %s", refstr, player->GetActorName().c_str());
            debug->Print(text, true);
#endif
            break;
        }
        case 0x146D: // GetParentCell New Vegas
        case 0x1495: // GetParentCell Fallout 3
        {
            DWORD cell = (DWORD) Fallout3_result;

            if (lastRef != self && !lastRef->GetReference().empty())
            {
                if (self->GetActorGameCell() == lastRef->GetActorNetworkCell() && lastRef->GetActorGameCell() != lastRef->GetActorNetworkCell())
                {
#ifdef VAULTMP_DEBUG
                    char text[128];
                    snprintf(text, sizeof(text), "Moving player to cell (name: %s, ref: %s, cell: %x)", lastRef->GetActorName().c_str(), lastRef->GetReference().c_str(), self->GetActorGameCell());
                    debug->Print(text, true);
#endif

                    lastRef->SetActorGameCell(self->GetActorGameCell());
                    lastRef->ToggleNoOverride(SKIPFLAG_GETPARENTCELL, true);

                    Command::StartSession();

                    if (lastRef->SetActorEnabled(true))
                    {
                        ParamList param_Enable;
                        param_Enable.push_back(lastRef->GetActorRefParam());
                        param_Enable.push_back(Data::Param_True);
                        ParamContainer Enable = ParamContainer(param_Enable, &Data::AlwaysTrue);
                        Command::ExecuteCommandOnce("Enable", Enable, 0, 150);
                    }

                    ParamList param_MoveTo;
                    param_MoveTo.push_back(lastRef->GetActorRefParam());
                    param_MoveTo.push_back(self->GetActorRefParam());
                    param_MoveTo.push_back(Data::Param_False);
                    param_MoveTo.push_back(Data::Param_False);
                    param_MoveTo.push_back(Data::Param_False);
                    ParamContainer MoveTo = ParamContainer(param_MoveTo, &Data::AlwaysTrue);
                    Command::ExecuteCommandOnce("MoveTo", MoveTo, 0, 150);

                    Command::EndSession();
                }
            }

            lastRef->SetActorGameCell(cell);

            if (lastRef != self && !lastRef->GetReference().empty())
            {
                if (lastRef->GetActorNetworkCell() != self->GetActorGameCell() && !lastRef->IsActorNearPoint(self->GetActorPos(0), self->GetActorPos(1), self->GetActorPos(2), 20000.0))
                {
                    if (lastRef->SetActorEnabled(false))
                    {
                        Command::StartSession();

                        ParamList param_Disable;
                        param_Disable.push_back(lastRef->GetActorRefParam());
                        param_Disable.push_back(Data::Param_False);
                        ParamContainer Disable = ParamContainer(param_Disable, &Data::AlwaysTrue);
                        Command::ExecuteCommandOnce("Disable", Disable, 0, 150);

                        Command::EndSession();
                    }
                }
                else
                {
                    if (lastRef->SetActorEnabled(true))
                    {
                        Command::StartSession();

                        ParamList param_Enable;
                        param_Enable.push_back(lastRef->GetActorRefParam());
                        param_Enable.push_back(Data::Param_True);
                        ParamContainer Enable = ParamContainer(param_Enable, &Data::AlwaysTrue);
                        Command::ExecuteCommandOnce("Enable", Enable, 0, 150);

                        Command::EndSession();
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
            case 0x59: // Y
            case 0x5A: // Z
                lastRef->ToggleNoOverride(SKIPFLAG_GETPOS, false);
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
                lastRef->ToggleNoOverride(SKIPFLAG_GETACTORVALUE, false);
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

            Inventory::TransferInventory(lastRef);
            break;
        }
        default:
        {
#ifdef VAULTMP_DEBUG
            char text[128];
            snprintf(text, sizeof(text), "Command could not be processed: %x %x %llx %x %x %x %x", Fallout3_opcode, Fallout3_refID, Fallout3_result, Fallout3_coord, Fallout3_setcoord, Fallout3_valcoord, Fallout3_newRefID);
            debug->Print(text, true);
#endif
            break;
        }
        }

        lastRef->EndSession();
    }
}

void Bethesda::StringHandler(string command)
{
    char output[command.length()];
    char* token;
    strcpy(output, command.c_str());

    int count = 0;
    int iLastRef = 0;
    int wantsdrawn = 0;
    int weapdrawn = 0;
    char action[64];

    self->StartSession();

    if (sscanf(output, "%d", &count))
    {
        char* data = strchr(output, '(');

        int item = 0;
        int type = 0;
        float condition = 0.00;
        char worn[16];

        if (data != NULL && sscanf(data, "(%X) (%d,%f%%) - %s", &item, &type, &condition, worn))
        {
            char basestr[16];
            bool bWorn = (stricmp(worn, "Worn") == 0) ? true : false;
            snprintf(basestr, sizeof(basestr), "%08x", item);

            if (!Inventory::AddItem_Internal(string(basestr), count, type, condition, bWorn))
            {
                // Item not registered in database, send error to server
            }
        }
    }
    else if (sscanf(output, "Wants Weapon Drawn %d, Weapon Drawn %d", &wantsdrawn, &weapdrawn))
    {
        self->SetActorAlerted((bool) weapdrawn);
    }
    else if (sscanf(output, "Movement -> %s", action))
    {
        token = strtok(action, "/");

        if (stricmp(token, "FastForward") == 0)
            self->SetActorMoving(MOV_FASTFORWARD);
        else if (stricmp(token, "FastBackward") == 0)
            self->SetActorMoving(MOV_FASTBACKWARD);
        else if (stricmp(token, "FastLeft") == 0)
            self->SetActorMoving(MOV_FASTLEFT);
        else if (stricmp(token, "FastRight") == 0)
            self->SetActorMoving(MOV_FASTRIGHT);
        else if (stricmp(token, "Forward") == 0)
            self->SetActorMoving(MOV_FORWARD);
        else if (stricmp(token, "Backward") == 0)
            self->SetActorMoving(MOV_BACKWARD);
        else if (stricmp(token, "Left") == 0)
            self->SetActorMoving(MOV_LEFT);
        else if (stricmp(token, "Right") == 0)
            self->SetActorMoving(MOV_RIGHT);
        else
        {
            token = strtok(NULL, "/");

            if (token != NULL)
            {
                if (stricmp(token, "FastForward") == 0)
                    self->SetActorMoving(MOV_FASTFORWARD);
                else if (stricmp(token, "FastBackward") == 0)
                    self->SetActorMoving(MOV_FASTBACKWARD);
                else if (stricmp(token, "FastLeft") == 0)
                    self->SetActorMoving(MOV_FASTLEFT);
                else if (stricmp(token, "FastRight") == 0)
                    self->SetActorMoving(MOV_FASTRIGHT);
                else if (stricmp(token, "Forward") == 0)
                    self->SetActorMoving(MOV_FORWARD);
                else if (stricmp(token, "Backward") == 0)
                    self->SetActorMoving(MOV_BACKWARD);
                else if (stricmp(token, "Left") == 0)
                    self->SetActorMoving(MOV_LEFT);
                else if (stricmp(token, "Right") == 0)
                    self->SetActorMoving(MOV_RIGHT);
            }
        }
    }
    else if (sscanf(output, "Life State: %s", action))
    {
        if (stricmp(action, "Dead") == 0)
        {
            if (self->SetActorDead(true))
            {
                Command::StartSession();

                map<RakNetGUID, Player*> players;
                map<RakNetGUID, Player*>::iterator it;
                players = Player::GetPlayerList();

                for (it = players.begin(); it != players.end(); ++it)
                {
                    Player* player = it->second;

                    if (player == self)
                        continue;

                    player->SetReference("");
                    player->SetActorEnabled(false);
                    refqueue.push_back(player);
                }

                ParamList param_Load;
                param_Load.push_back(Data::BuildParameter(savegame));
                ParamContainer Load = ParamContainer(param_Load, &Data::AlwaysTrue);
                Command::ExecuteCommandOnce("Load", Load, 0, 150);

                ParamList param_SetName;
                param_SetName.push_back(self->GetActorRefParam());
                param_SetName.push_back(self->GetActorNameParam());
                ParamContainer SetName = ParamContainer(param_SetName, &Data::AlwaysTrue);
                Command::ExecuteCommandOnce("SetName", SetName);

                Command::EndSession();
            }
        }
        else
        {
            if (self->SetActorDead(false))
            {
                Command::StartSession();

                map<RakNetGUID, Player*> players;
                map<RakNetGUID, Player*>::iterator it;
                players = Player::GetPlayerList();

                for (it = players.begin(); it != players.end(); ++it)
                {
                    Player* player = it->second;

                    if (player == self)
                        continue;

                    string baseID = NewVegas ? "8D0E7" : "30D82";

                    ParamList param_PlaceAtMe;
                    param_PlaceAtMe.push_back(self->GetActorRefParam());
                    param_PlaceAtMe.push_back(Data::BuildParameter(baseID));
                    param_PlaceAtMe.push_back(Data::Param_True);
                    param_PlaceAtMe.push_back(Data::Param_False);
                    param_PlaceAtMe.push_back(Data::Param_False);
                    ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
                    Command::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, 1000);
                }

                Command::EndSession();
            }
        }
    }
    /*else
    {
        #ifdef VAULTMP_DEBUG
        char text[256];
        snprintf(text, sizeof(text), "String could not be processed: %s", const_cast<char*>(high.c_str()));
        debug->Print(text, true);
        #endif
    }*/

    self->EndSession();
}

void Bethesda::InitializeCommands()
{
    Command::DefineCommand("GetPos", "%0.GetPos %1");
    Command::DefineCommand("SetPos", "%0.SetPos %1 %2");
    Command::DefineCommand("GetAngle", "%0.GetAngle %1");
    Command::DefineCommand("SetAngle", "%0.SetAngle %1 %2");
    Command::DefineCommand("GetParentCell", "%0.GetParentCell");
    Command::DefineCommand("GetBaseActorValue", "%0.GetBaseActorValue %1");
    Command::DefineCommand("SetActorValue", "%0.SetActorValue %1 %2");
    Command::DefineCommand("ForceActorValue", "%0.ForceActorValue %1 %2");
    Command::DefineCommand("GetActorValue", "%0.GetActorValue %1");
    Command::DefineCommand("GetDead", "%0.GetDead");
    Command::DefineCommand("IsMoving", "%0.IsMoving");
    Command::DefineCommand("ShowAnim", "%0.ShowAnim");
    Command::DefineCommand("ShowInventory", "%0.ShowInventory");
    Command::DefineCommand("Enable", "%0.Enable %1");
    Command::DefineCommand("Disable", "%0.Disable %1");
    Command::DefineCommand("MoveTo", "%0.MoveTo %1 %2 %3 %4");
    Command::DefineCommand("SetRestrained", "%0.SetRestrained %1");
    Command::DefineCommand("PlayGroup", "%0.PlayGroup %1 %2");
    Command::DefineCommand("SetAlert", "%0.SetAlert %1");
    Command::DefineCommand("SetName", "%0.SetName %1");
    Command::DefineCommand("EquipItem", "%0.EquipItem %1 %2 %3");
    Command::DefineCommand("UnequipItem", "%0.UnequipItem %1 %2 %3");
    Command::DefineCommand("AddItem", "%0.AddItem %1 %2 %3");
    Command::DefineCommand("RemoveItem", "%0.RemoveItem %1 %2 %3");
    Command::DefineCommand("RemoveItems", "%0.RemoveAllItems %1 %2");
    Command::DefineCommand("RemoveAllItems", "%0.RemoveAllItems");
    Command::DefineCommand("ResurrectActor", "%0.ResurrectActor %1");
    Command::DefineCommand("KillActor", "%0.KillActor %1 %2 %3");
    Command::DefineCommand("Kill", "%0.KillActor");
    Command::DefineCommand("PlaceAtMe", "%0.PlaceAtMe %1 %2 %3 %4");
    Command::DefineCommand("MarkForDelete", "%0.MarkForDelete");
    Command::DefineCommand("Load", "Load %0");

    ParamList param_GetPos;
    param_GetPos.push_back(Player::Param_EnabledPlayers);
    param_GetPos.push_back(Data::Param_XYZ);
    ParamContainer GetPos = ParamContainer(param_GetPos, &Data::AlwaysTrue);
    Command::DefineNative("GetPos", GetPos);
    Command::ExecuteCommandLoop("GetPos");

    ParamList param_GetParentCell;
    param_GetParentCell.push_back(Player::Param_AllPlayers);
    ParamContainer GetParentCell = ParamContainer(param_GetParentCell, &Data::AlwaysTrue);
    Command::DefineNative("GetParentCell", GetParentCell);
    Command::ExecuteCommandLoop("GetParentCell");

    ParamList param_GetDead;
    param_GetDead.push_back(Player::Param_EnabledPlayers_NotSelf);
    ParamContainer GetDead = ParamContainer(param_GetDead, &Data::AlwaysTrue);
    Command::DefineNative("GetDead", GetDead);
    Command::ExecuteCommandLoop("GetDead", 10);

    ParamList param_GetAngle;
    param_GetAngle.push_back(self->GetActorRefParam());
    param_GetAngle.push_back(Data::Param_Z);
    ParamContainer GetAngle = ParamContainer(param_GetAngle, &Data::AlwaysTrue);
    Command::DefineNative("GetAngle", GetAngle);
    Command::ExecuteCommandLoop("GetAngle");

    ParamList param_IsMoving;
    param_IsMoving.push_back(self->GetActorRefParam());
    ParamContainer IsMoving = ParamContainer(param_IsMoving, &Data::AlwaysTrue);
    Command::DefineNative("IsMoving", IsMoving);
    Command::ExecuteCommandLoop("IsMoving");

    ParamList param_GetBaseActorValue;
    param_GetBaseActorValue.push_back(self->GetActorRefParam());
    param_GetBaseActorValue.push_back(Data::Param_BaseActorValues);
    ParamContainer GetBaseActorValue = ParamContainer(param_GetBaseActorValue, &Data::AlwaysTrue);
    Command::DefineNative("GetBaseActorValue", GetBaseActorValue);
    Command::ExecuteCommandLoop("GetBaseActorValue", 70);

    ParamList param_GetActorValue;
    param_GetActorValue.push_back(self->GetActorRefParam());
    param_GetActorValue.push_back(Data::Param_ActorValues);
    ParamContainer GetActorValue = ParamContainer(param_GetActorValue, &Data::AlwaysTrue);
    Command::DefineNative("GetActorValue", GetActorValue);
    Command::ExecuteCommandLoop("GetActorValue", 20);

    ParamList param_ShowAnim;
    param_ShowAnim.push_back(self->GetActorRefParam());
    ParamContainer ShowAnim = ParamContainer(param_ShowAnim, &Data::AlwaysTrue);
    Command::DefineNative("ShowAnim", ShowAnim);
    Command::ExecuteCommandLoop("ShowAnim");

    ParamList param_ShowInventory;
    param_ShowInventory.push_back(self->GetActorRefParam());
    ParamContainer ShowInventory = ParamContainer(param_ShowInventory, &Data::AlwaysTrue);
    Command::DefineNative("ShowInventory", ShowInventory);
    Command::ExecuteCommandLoop("ShowInventory", 10);
}

bool Bethesda::InitializeFallout(bool NewVegas)
{
    char module[32];

    switch (NewVegas)
    {
    case false:
        strcpy(module, "Fallout3.exe");
        Bethesda::NewVegas = false;
        break;
    case true:
        strcpy(module, "FalloutNV.exe");
        putenv("SteamAppID=22380"); // necessary for Steam
        Bethesda::NewVegas = true;
        break;
    }

    if (Command::lookupProgramID(module) == 0)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);

        if (CreateProcess(module, NULL, NULL, NULL, FALSE, Bethesda::NewVegas ? 0 : CREATE_SUSPENDED, NULL, NULL, &si, &pi))
        {
            if (Bethesda::NewVegas) Sleep(2000); // some decrypt whatsoever needs time

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
                data.NewVegas = Bethesda::NewVegas;

                codesize = (DWORD) InjectedEnd - (DWORD) InjectedCode;

                start = VirtualAllocEx(hProc, 0, codesize + sizeof(INJECT), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
                thread = (LPVOID) ((DWORD) start + sizeof(INJECT));

                WriteProcessMemory(hProc, start, (LPVOID) &data, sizeof(INJECT), NULL);
                WriteProcessMemory(hProc, thread, (LPVOID) InjectedCode, codesize, NULL);

                CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE) thread, start, 0, 0);

                /* Initalizing vaultmp.exe <-> Fallout3.exe / FalloutNV.exe pipe */

                HANDLE* threads;

                if (!Command::Initialize(module, &CommandHandler, &StringHandler))
                    return false;

                while (!Command::IsAvailable()) Sleep(1); // Some timeout is necessary here, or a signal

                /* Resuming Fallout3.exe */

                if (!Bethesda::NewVegas) ResumeThread(pi.hThread);

                CloseHandle(hProc);

                return true;
            }
            else
                MessageBox(NULL, "Failed opening the game process!", "Error", MB_OK | MB_ICONERROR);
        }
        else
            MessageBox(NULL, "Failed creating the game process!", "Error", MB_OK | MB_ICONERROR);
    }
    else
        MessageBox(NULL, "Either Fallout 3 or Fallout: New Vegas is already runnning!", "Error", MB_OK | MB_ICONERROR);

    return false;
}

void Bethesda::InitializeVaultMP(RakPeerInterface* peer, SystemAddress addr, string name, string pwd, bool NewVegas)
{
    Bethesda::NewVegas = NewVegas;
    initialized = false;

#ifdef VAULTMP_DEBUG
    debug = new Debug((char*) "vaultmp");

    char text[128];
    snprintf(text, sizeof(text), "Vault-Tec Multiplayer Mod client debug log (%s)", CLIENT_VERSION);
    debug->Print(text, false);
    snprintf(text, sizeof(text), "Connecting to server: %s (name: %s, password: %s, game: %s)", addr.ToString(), name.c_str(), pwd.c_str(), NewVegas ? (char*) "Fallout New Vegas" : (char*) "Fallout 3");
    debug->Print(text, false);

    debug->Print((char*) "Visit www.vaultmp.com for help and upload this log if you experience problems with the mod.", false);
    debug->Print((char*) "-----------------------------------------------------------------------------------------------------", false);
    //debug->PrintSystem();
    Command::SetDebugHandler(debug);
    Inventory::SetDebugHandler(debug);
    Actor::SetDebugHandler(debug);
    Player::SetDebugHandler(debug);
#endif

    Inventory::Initialize(Bethesda::NewVegas);
    Actor::Initialize();
    Player::Initialize();

    self = new Player(peer->GetMyGUID(), "player", NewVegas ? "8D0E7" : "30D82");
    self->SetActorName(name);

    InitializeCommands();

    Inventory localPlayerInventory;
    pActorUpdate localPlayerUpdate = self->GetActorUpdateStruct();
    pActorStateUpdate localPlayerStateUpdate = self->GetActorStateUpdateStruct();
    pActorCellUpdate localPlayerCellUpdate = self->GetActorCellUpdateStruct();
    list<pActorItemUpdate> localPlayerItemUpdate;

    localPlayerUpdate.guid = peer->GetMyGUID();
    localPlayerStateUpdate.guid = peer->GetMyGUID();
    localPlayerCellUpdate.guid = peer->GetMyGUID();

    if (peer->Connect(addr.ToString(false), addr.GetPort(), DEDICATED_VERSION, sizeof(DEDICATED_VERSION), 0, 0, 3, 500, 0) == CONNECTION_ATTEMPT_STARTED)
    {
        bool query = true;

        Packet* packet;

        while (query)
        {
            if (initialized && !Command::IsAvailable())
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
                    snprintf(text, sizeof(text), "Connection request accepted (%s)", packet->systemAddress.ToString());
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
                    snprintf(text, sizeof(text), "We were successfully authenticated (%s)", packet->systemAddress.ToString());
                    debug->Print(text, true);
                    debug->Print((char*) "Initiating vaultmp game thread...", true);
#endif

                    BitStream query;

                    initialized = InitializeFallout(NewVegas);

                    if (initialized)
                        query.Write((MessageID) ID_GAME_RUN);
                    else
                        query.Write((MessageID) ID_GAME_END);

                    peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, packet->systemAddress, false, 0);
                    break;
                }
                case ID_GAME_RUN:
                {
                    BitStream query(packet->data, packet->length, false);
                    query.IgnoreBytes(sizeof(MessageID));

                    RakString save;
                    query.Read(save);
                    query.Reset();

                    savegame = string(save.C_String());

                    Command::StartSession();

                    ParamList param_Load;
                    param_Load.push_back(Data::BuildParameter(savegame));
                    ParamContainer Load = ParamContainer(param_Load, &Data::AlwaysTrue);
                    Command::ExecuteCommandOnce("Load", Load, 0, 150);

                    ParamList param_SetName;
                    param_SetName.push_back(self->GetActorRefParam());
                    param_SetName.push_back(self->GetActorNameParam());
                    ParamContainer SetName = ParamContainer(param_SetName, &Data::AlwaysTrue);
                    Command::ExecuteCommandOnce("SetName", SetName);

                    Command::EndSession();

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
                    snprintf(text, sizeof(text), "New player on the server (name: %s, guid: %s)", name.C_String(), guid.ToString());
                    debug->Print(text, true);
#endif

                    string baseID = NewVegas ? "8D0E7" : "30D82";

                    Player* player = new Player(guid, baseID);
                    player->SetActorName(string(name.C_String()));
                    refqueue.push_back(player);

                    Command::StartSession();

                    ParamList param_PlaceAtMe;
                    param_PlaceAtMe.push_back(self->GetActorRefParam());
                    param_PlaceAtMe.push_back(Data::BuildParameter(baseID));
                    param_PlaceAtMe.push_back(Data::Param_True);
                    param_PlaceAtMe.push_back(Data::Param_False);
                    param_PlaceAtMe.push_back(Data::Param_False);
                    ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
                    Command::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, 1000);

                    Command::EndSession();
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

                    if (player == NULL)
                    {
#ifdef VAULTMP_DEBUG
                        char text[128];
                        snprintf(text, sizeof(text), "ERROR: Player could not be found (guid: %s)", guid.ToString());
                        debug->Print(text, true);
#endif
                        break;
                    }

                    string refID = player->GetReference();

#ifdef VAULTMP_DEBUG
                    char text[128];
                    snprintf(text, sizeof(text), "Player left (name: %s, guid: %s)", player->GetActorName().c_str(), guid.ToString());
                    debug->Print(text, true);
#endif

                    if (!refID.empty())
                    {
                        Command::StartSession();

                        if (player->SetActorEnabled(false))
                        {
                            ParamList param_Disable;
                            param_Disable.push_back(player->GetActorRefParam());
                            param_Disable.push_back(Data::Param_False);
                            ParamContainer Disable = ParamContainer(param_Disable, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("Disable", Disable, 0, 150);
                        }

                        ParamList param_MarkForDelete;
                        param_MarkForDelete.push_back(player->GetActorRefParam());
                        ParamContainer MarkForDelete = ParamContainer(param_MarkForDelete, &Data::AlwaysTrue);
                        Command::ExecuteCommandOnce("MarkForDelete", MarkForDelete, 0, 150);

                        Command::EndSession();
                    }

                    player->StartSession();
                    delete player;
                    player->EndSession();

                    refqueue.remove(player);
                    break;
                }
                case ID_PLAYER_UPDATE:
                {
                    pActorUpdate* update = (pActorUpdate*) packet->data;

                    if (packet->length != sizeof(pActorUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(update->guid);

                    if (player == NULL)
                    {
#ifdef VAULTMP_DEBUG
                        char text[128];
                        snprintf(text, sizeof(text), "ERROR: Player could not be found (guid: %s)", update->guid.ToString());
                        debug->Print(text, true);
#endif
                        break;
                    }

                    string refID = player->GetReference();

#ifdef VAULTMP_DEBUG
                    char text[128];
                    snprintf(text, sizeof(text), "Received player update packet (name: %s, ref: %s, guid: %s)", player->GetActorName().c_str(), refID.c_str(), update->guid.ToString());
                    debug->Print(text, true);
#endif

                    if (!refID.empty())
                    {
                        Command::StartSession();

                        char pos[16];

                        if (player->SetActorAngle(update->A) && player->GetActorEnabled())
                        {
                            snprintf(pos, sizeof(pos), "%f", update->A);

                            ParamList param_SetAngle;
                            param_SetAngle.push_back(player->GetActorRefParam());
                            param_SetAngle.push_back(Data::Param_Z);
                            param_SetAngle.push_back(Data::BuildParameter(string(pos)));
                            ParamContainer SetAngle = ParamContainer(param_SetAngle, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("SetAngle", SetAngle);
                        }

                        if (player->SetActorMoving(update->moving) && player->GetActorEnabled())
                        {
                            ParamList param_PlayGroup;
                            param_PlayGroup.push_back(player->GetActorRefParam());

                            switch (update->moving)
                            {
                            case MOV_IDLE:
                                param_PlayGroup.push_back(Data::BuildParameter("Idle"));
                                param_PlayGroup.push_back(Data::Param_False);
                                break;
                            case MOV_FASTFORWARD:
                                param_PlayGroup.push_back(Data::BuildParameter("FastForward"));
                                param_PlayGroup.push_back(Data::Param_True);
                                break;
                            case MOV_FASTBACKWARD:
                                param_PlayGroup.push_back(Data::BuildParameter("FastBackward"));
                                param_PlayGroup.push_back(Data::Param_True);
                                break;
                            case MOV_FASTLEFT:
                                param_PlayGroup.push_back(Data::BuildParameter("FastLeft"));
                                param_PlayGroup.push_back(Data::Param_True);
                                break;
                            case MOV_FASTRIGHT:
                                param_PlayGroup.push_back(Data::BuildParameter("FastRight"));
                                param_PlayGroup.push_back(Data::Param_True);
                                break;
                            case MOV_FORWARD:
                                param_PlayGroup.push_back(Data::BuildParameter("Forward"));
                                param_PlayGroup.push_back(Data::Param_True);
                                break;
                            case MOV_BACKWARD:
                                param_PlayGroup.push_back(Data::BuildParameter("Backward"));
                                param_PlayGroup.push_back(Data::Param_True);
                                break;
                            case MOV_LEFT:
                                param_PlayGroup.push_back(Data::BuildParameter("Left"));
                                param_PlayGroup.push_back(Data::Param_True);
                                break;
                            case MOV_RIGHT:
                                param_PlayGroup.push_back(Data::BuildParameter("Right"));
                                param_PlayGroup.push_back(Data::Param_True);
                                break;
                            }

                            ParamContainer PlayGroup = ParamContainer(param_PlayGroup, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("PlayGroup", PlayGroup);
                        }

                        if (!player->IsCoordinateInRange(X_AXIS, update->X, 350.0))
                        {
                            if (player->SetActorPos(X_AXIS, update->X) && player->GetActorEnabled())
                            {
                                player->ToggleNoOverride(SKIPFLAG_GETPOS, true);

                                snprintf(pos, sizeof(pos), "%f", update->X);

                                ParamList param_SetPos;
                                param_SetPos.push_back(player->GetActorRefParam());
                                param_SetPos.push_back(Data::Param_X);
                                param_SetPos.push_back(Data::BuildParameter(string(pos)));
                                ParamContainer SetPos = ParamContainer(param_SetPos, &Data::AlwaysTrue);
                                Command::ExecuteCommandOnce("SetPos", SetPos);
                            }
                        }

                        if (!player->IsCoordinateInRange(Y_AXIS, update->Y, 350.0))
                        {
                            if (player->SetActorPos(Y_AXIS, update->Y) && player->GetActorEnabled())
                            {
                                player->ToggleNoOverride(SKIPFLAG_GETPOS, true);

                                snprintf(pos, sizeof(pos), "%f", update->Y);

                                ParamList param_SetPos;
                                param_SetPos.push_back(player->GetActorRefParam());
                                param_SetPos.push_back(Data::Param_Y);
                                param_SetPos.push_back(Data::BuildParameter(string(pos)));
                                ParamContainer SetPos = ParamContainer(param_SetPos, &Data::AlwaysTrue);
                                Command::ExecuteCommandOnce("SetPos", SetPos);
                            }
                        }

                        if (!player->IsCoordinateInRange(Z_AXIS, update->Z, 200.0))
                        {
                            if (player->SetActorPos(Z_AXIS, update->Z) && player->GetActorEnabled())
                            {
                                player->ToggleNoOverride(SKIPFLAG_GETPOS, true);

                                snprintf(pos, sizeof(pos), "%f", update->Z);

                                ParamList param_SetPos;
                                param_SetPos.push_back(player->GetActorRefParam());
                                param_SetPos.push_back(Data::Param_Z);
                                param_SetPos.push_back(Data::BuildParameter(string(pos)));
                                ParamContainer SetPos = ParamContainer(param_SetPos, &Data::AlwaysTrue);
                                Command::ExecuteCommandOnce("SetPos", SetPos);
                            }
                        }

                        /*
                        if (player->GetActorMoving() == 0 && !player->IsActorNearPoint(update->X, update->Y, update->Z, 400.0) && player->GetActorEnabled())
                        {
                            if (player->SetActorPos(0, update->X))
                            {
                                player->ToggleNoOverride(SKIPFLAG_GETPOS_X, true);

                                snprintf(pos, sizeof(pos), "%f", update->X);

                                cmd = new fCommand;
                                cmd->command = "setpos X " + string(pos);
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                PUSHCMD(cmd);
                            }

                            if (player->SetActorPos(1, update->Y))
                            {
                                player->ToggleNoOverride(SKIPFLAG_GETPOS_Y, true);

                                snprintf(pos, sizeof(pos), "%f", update->Y);

                                cmd = new fCommand;
                                cmd->command = "setpos Y " + string(pos);
                                cmd->player = player;
                                cmd->forplayers = false;
                                cmd->repeat = false;
                                PUSHCMD(cmd);
                            }
                        }*/

                        if (player->SetActorAlerted(update->alerted) && player->GetActorEnabled())
                        {
                            ParamList param_SetRestrained;
                            param_SetRestrained.push_back(player->GetActorRefParam());
                            param_SetRestrained.push_back(Data::Param_False);
                            ParamContainer SetRestrained = ParamContainer(param_SetRestrained, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("SetRestrained", SetRestrained, 0, 150);

                            ParamList param_SetAlert;
                            param_SetAlert.push_back(player->GetActorRefParam());

                            switch (update->alerted)
                            {
                            case true:
                                param_SetAlert.push_back(Data::Param_True);
                                break;
                            case false:
                                param_SetAlert.push_back(Data::Param_False);
                                break;
                            }

                            ParamContainer SetAlert = ParamContainer(param_SetAlert, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("SetAlert", SetAlert, 0, 150);

                            param_SetRestrained.clear();
                            param_SetRestrained.push_back(player->GetActorRefParam());
                            param_SetRestrained.push_back(Data::Param_True);
                            SetRestrained = ParamContainer(param_SetRestrained, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("SetRestrained", SetRestrained, 0, 150);
                        }

                        Command::EndSession();
                    }
                    break;
                }
                case ID_PLAYER_STATE_UPDATE:
                {
                    pActorStateUpdate* update = (pActorStateUpdate*) packet->data;

                    if (packet->length != sizeof(pActorStateUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(update->guid);

                    if (player == NULL)
                    {
#ifdef VAULTMP_DEBUG
                        char text[128];
                        snprintf(text, sizeof(text), "ERROR: Player could not be found (guid: %s)", update->guid.ToString());
                        debug->Print(text, true);
#endif
                        break;
                    }

                    string refID = player->GetReference();

#ifdef VAULTMP_DEBUG
                    char text[128];
                    snprintf(text, sizeof(text), "Received player state update packet (name: %s, ref: %s, guid: %s)", player->GetActorName().c_str(), refID.c_str(), update->guid.ToString());
                    debug->Print(text, true);
#endif

                    if (!refID.empty())
                    {
                        Command::StartSession();

                        char pos[16];

                        if (player->SetActorBaseHealth(update->baseHealth))
                        {
                            snprintf(pos, sizeof(pos), "%i", (int) update->baseHealth);

                            ParamList param_SetActorValue;
                            param_SetActorValue.push_back(player->GetActorRefParam());
                            param_SetActorValue.push_back(Data::BuildParameter("Health"));
                            param_SetActorValue.push_back(Data::BuildParameter(pos));
                            ParamContainer SetActorValue = ParamContainer(param_SetActorValue, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("SetActorValue", SetActorValue);
                        }

                        if (player->SetActorHealth(update->health))
                        {
                            player->ToggleNoOverride(SKIPFLAG_GETACTORVALUE, true);

                            snprintf(pos, sizeof(pos), "%i", (int) update->health);

                            ParamList param_ForceActorValue;
                            param_ForceActorValue.push_back(player->GetActorRefParam());
                            param_ForceActorValue.push_back(Data::BuildParameter("Health"));
                            param_ForceActorValue.push_back(Data::BuildParameter(pos));
                            ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                        }

                        if (player->SetActorCondition(COND_PERCEPTION, update->conds[COND_PERCEPTION]))
                        {
                            snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_PERCEPTION]);

                            ParamList param_ForceActorValue;
                            param_ForceActorValue.push_back(player->GetActorRefParam());
                            param_ForceActorValue.push_back(Data::BuildParameter("PerceptionCondition"));
                            param_ForceActorValue.push_back(Data::BuildParameter(pos));
                            ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                        }

                        if (player->SetActorCondition(COND_ENDURANCE, update->conds[COND_ENDURANCE]))
                        {
                            snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_ENDURANCE]);

                            ParamList param_ForceActorValue;
                            param_ForceActorValue.push_back(player->GetActorRefParam());
                            param_ForceActorValue.push_back(Data::BuildParameter("EnduranceCondition"));
                            param_ForceActorValue.push_back(Data::BuildParameter(pos));
                            ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                        }

                        if (player->SetActorCondition(COND_LEFTATTACK, update->conds[COND_LEFTATTACK]))
                        {
                            snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_LEFTATTACK]);

                            ParamList param_ForceActorValue;
                            param_ForceActorValue.push_back(player->GetActorRefParam());
                            param_ForceActorValue.push_back(Data::BuildParameter("LeftAttackCondition"));
                            param_ForceActorValue.push_back(Data::BuildParameter(pos));
                            ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                        }

                        if (player->SetActorCondition(COND_RIGHTATTACK, update->conds[COND_RIGHTATTACK]))
                        {
                            snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_RIGHTATTACK]);

                            ParamList param_ForceActorValue;
                            param_ForceActorValue.push_back(player->GetActorRefParam());
                            param_ForceActorValue.push_back(Data::BuildParameter("RightAttackCondition"));
                            param_ForceActorValue.push_back(Data::BuildParameter(pos));
                            ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                        }

                        if (player->SetActorCondition(COND_LEFTMOBILITY, update->conds[COND_LEFTMOBILITY]))
                        {
                            snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_LEFTMOBILITY]);

                            ParamList param_ForceActorValue;
                            param_ForceActorValue.push_back(player->GetActorRefParam());
                            param_ForceActorValue.push_back(Data::BuildParameter("LeftMobilityCondition"));
                            param_ForceActorValue.push_back(Data::BuildParameter(pos));
                            ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                        }

                        if (player->SetActorCondition(COND_RIGHTMOBILITY, update->conds[COND_RIGHTMOBILITY]))
                        {
                            snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_RIGHTMOBILITY]);

                            ParamList param_ForceActorValue;
                            param_ForceActorValue.push_back(player->GetActorRefParam());
                            param_ForceActorValue.push_back(Data::BuildParameter("RightMobilityCondition"));
                            param_ForceActorValue.push_back(Data::BuildParameter(pos));
                            ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                        }

                        if (player->SetActorDead(update->dead))
                        {
                            player->ToggleNoOverride(SKIPFLAG_GETDEAD, true);

                            switch (update->dead)
                            {
                            case true:
                            {
                                ParamList param_Kill;
                                param_Kill.push_back(player->GetActorRefParam());
                                ParamContainer Kill = ParamContainer(param_Kill, &Data::AlwaysTrue);
                                Command::ExecuteCommandOnce("Kill", Kill, 0, 150);
                                break;
                            }
                            case false:
                            {
                                if (player->SetActorEnabled(false))
                                {
                                    ParamList param_Disable;
                                    param_Disable.push_back(player->GetActorRefParam());
                                    param_Disable.push_back(Data::Param_False);
                                    ParamContainer Disable = ParamContainer(param_Disable, &Data::AlwaysTrue);
                                    Command::ExecuteCommandOnce("Disable", Disable, 0, 150);
                                }

                                ParamList param_MarkForDelete;
                                param_MarkForDelete.push_back(player->GetActorRefParam());
                                ParamContainer MarkForDelete = ParamContainer(param_MarkForDelete, &Data::AlwaysTrue);
                                Command::ExecuteCommandOnce("MarkForDelete", MarkForDelete, 0, 150);

                                player->SetReference("");

                                refqueue.push_back(player);

                                string baseID = NewVegas ? "8D0E7" : "30D82";

                                ParamList param_PlaceAtMe;
                                param_PlaceAtMe.push_back(self->GetActorRefParam());
                                param_PlaceAtMe.push_back(Data::BuildParameter(baseID));
                                param_PlaceAtMe.push_back(Data::Param_True);
                                param_PlaceAtMe.push_back(Data::Param_False);
                                param_PlaceAtMe.push_back(Data::Param_False);
                                ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
                                Command::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, 1000);
                                break;
                            }
                            }
                        }

                        Command::EndSession();
                    }
                    break;
                }
                case ID_PLAYER_CELL_UPDATE:
                {
                    pActorCellUpdate* update = (pActorCellUpdate*) packet->data;

                    if (packet->length != sizeof(pActorCellUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(update->guid);

                    if (player == NULL)
                    {
#ifdef VAULTMP_DEBUG
                        char text[128];
                        snprintf(text, sizeof(text), "ERROR: Player could not be found (guid: %s)", update->guid.ToString());
                        debug->Print(text, true);
#endif
                        break;
                    }

                    string refID = player->GetReference();

#ifdef VAULTMP_DEBUG
                    char text[128];
                    snprintf(text, sizeof(text), "Received player cell update packet (name: %s, ref: %s, guid: %s)", player->GetActorName().c_str(), refID.c_str(), update->guid.ToString());
                    debug->Print(text, true);
#endif

                    if (!refID.empty())
                    {
                        player->SetActorNetworkCell(update->cell);
                    }
                    break;
                }
                case ID_PLAYER_ITEM_UPDATE:
                {
                    pActorItemUpdate* update = (pActorItemUpdate*) packet->data;

                    if (packet->length != sizeof(pActorItemUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(update->guid);

                    if (player == NULL)
                    {
#ifdef VAULTMP_DEBUG
                        char text[128];
                        snprintf(text, sizeof(text), "ERROR: Player could not be found (guid: %s)", update->guid.ToString());
                        debug->Print(text, true);
#endif
                        break;
                    }

                    string refID = player->GetReference();

#ifdef VAULTMP_DEBUG
                    char text[128];
                    snprintf(text, sizeof(text), "Received player item update packet (name: %s, ref: %s, guid: %s)", player->GetActorName().c_str(), refID.c_str(), update->guid.ToString());
                    debug->Print(text, true);
#endif
                    if (player != self)
                    {
                        player->StartSession();

                        if (update->item.count == 0)
                        {
                            if (!player->UpdateItem(string(update->baseID), update->item.condition, update->item.worn))
                            {

                            }
                        }
                        else if (update->item.count > 0)
                        {
                            if (!player->AddItem(string(update->baseID), update->item.count, update->item.type, update->item.condition, update->item.worn))
                            {

                            }
                        }
                        else if (update->item.count < 0)
                        {
                            if (!player->RemoveItem(string(update->baseID), abs(update->item.count)))
                            {

                            }
                        }

                        player->EndSession();
                    }

                    if (!refID.empty())
                    {
                        string baseID = string(update->baseID);
                        update->item.item = Inventory::GetItemReference(baseID);

                        if (update->item.item == map<const char*, const char*, str_compare>().end())
                            break;

                        /*if (player == self)
                            player->ToggleNoOverride(SKIPFLAG_ITEMUPDATE, true);*/

                        Command::StartSession();

                        if (update->item.count > 0)
                        {
                            ParamList param_AddItem;
                            param_AddItem.push_back(player->GetActorRefParam());
                            param_AddItem.push_back(Inventory::GetItemBaseParam(&update->item));
                            param_AddItem.push_back(Inventory::GetItemCountParam(&update->item));
                            param_AddItem.push_back(update->hidden ? Data::Param_True : Data::Param_False);
                            ParamContainer AddItem = ParamContainer(param_AddItem, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("AddItem", AddItem);
                        }
                        else if (update->item.count < 0)
                        {
                            update->item.count = abs(update->item.count);

                            ParamList param_RemoveItem;
                            param_RemoveItem.push_back(player->GetActorRefParam());
                            param_RemoveItem.push_back(Inventory::GetItemBaseParam(&update->item));
                            param_RemoveItem.push_back(Inventory::GetItemCountParam(&update->item));
                            param_RemoveItem.push_back(update->hidden ? Data::Param_True : Data::Param_False);
                            ParamContainer RemoveItem = ParamContainer(param_RemoveItem, &Data::AlwaysTrue);
                            Command::ExecuteCommandOnce("RemoveItem", RemoveItem);
                        }

                        if (update->item.count >= 0)
                        {
                            if (update->item.worn)
                            {
                                ParamList param_EquipItem;
                                param_EquipItem.push_back(player->GetActorRefParam());
                                param_EquipItem.push_back(Inventory::GetItemBaseParam(&update->item));
                                param_EquipItem.push_back(player != self ? Data::Param_True : Data::Param_False);
                                param_EquipItem.push_back(update->hidden ? Data::Param_True : Data::Param_False);
                                ParamContainer EquipItem = ParamContainer(param_EquipItem, &Data::AlwaysTrue);
                                Command::ExecuteCommandOnce("EquipItem", EquipItem);
                            }
                            else/* if (update->item.condition != 0.00)*/
                            {
                                ParamList param_UnequipItem;
                                param_UnequipItem.push_back(player->GetActorRefParam());
                                param_UnequipItem.push_back(Inventory::GetItemBaseParam(&update->item));
                                param_UnequipItem.push_back(player != self ? Data::Param_True : Data::Param_False);
                                param_UnequipItem.push_back(update->hidden ? Data::Param_True : Data::Param_False);
                                ParamContainer UnequipItem = ParamContainer(param_UnequipItem, &Data::AlwaysTrue);
                                Command::ExecuteCommandOnce("UnequipItem", UnequipItem);
                            }
                        }

                        Command::EndSession();
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

            if (self->UpdateActorUpdateStruct(&localPlayerUpdate))
            {
                peer->Send((char*) &localPlayerUpdate, sizeof(localPlayerUpdate), MEDIUM_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_PLAYER_UPDATE, addr, false, 0);

#ifdef VAULTMP_DEBUG
                char text[128];
                snprintf(text, sizeof(text), "Sent player update packet (%s)", addr.ToString());
                debug->Print(text, true);
#endif
            }

            if (self->UpdateActorStateUpdateStruct(&localPlayerStateUpdate))
            {
                peer->Send((char*) &localPlayerStateUpdate, sizeof(localPlayerStateUpdate), HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_PLAYER_STATE_UPDATE, addr, false, 0);

#ifdef VAULTMP_DEBUG
                char text[128];
                snprintf(text, sizeof(text), "Sent player state update packet (%s)", addr.ToString());
                debug->Print(text, true);
#endif
            }

            if (self->UpdateActorCellUpdateStruct(&localPlayerCellUpdate))
            {
                peer->Send((char*) &localPlayerCellUpdate, sizeof(localPlayerCellUpdate), HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_PLAYER_CELL_UPDATE, addr, false, 0);

#ifdef VAULTMP_DEBUG
                char text[128];
                snprintf(text, sizeof(text), "Sent player cell update packet (%s)", addr.ToString());
                debug->Print(text, true);
#endif
            }

            if (self->UpdateActorItemUpdateStruct(&localPlayerItemUpdate, &localPlayerInventory))
            {
                list<pActorItemUpdate>::iterator it;

                for (it = localPlayerItemUpdate.begin(); it != localPlayerItemUpdate.end(); ++it)
                {
                    it->guid = peer->GetMyGUID();
                    peer->Send((char*) &(*it), sizeof(pActorItemUpdate), HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_PLAYER_ITEM_UPDATE, addr, false, 0);
                }


#ifdef VAULTMP_DEBUG
                char text[128];
                snprintf(text, sizeof(text), "Sent %d player item update packets (%s)", localPlayerItemUpdate.size(), addr.ToString());
                debug->Print(text, true);
#endif
            }

            RakSleep(2);
        }
    }

    Command::Terminate();
    Player::DestroyInstances();
    Actor::DestroyInstances();
    Inventory::Cleanup();

#ifdef VAULTMP_DEBUG
    debug->Print((char*) "Network thread is going to terminate...", true);
    delete debug;
#endif
}
