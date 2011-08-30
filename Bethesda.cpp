#include "Bethesda.h"

using namespace std;
using namespace RakNet;

typedef HINSTANCE (__stdcall *fLoadLibrary)(char*);

int Bethesda::game = 0;
bool Bethesda::initialized = false;
string Bethesda::password = "";
Savegame Bethesda::savegame;
ModList Bethesda::modfiles;

#ifdef VAULTMP_DEBUG
Debug* Bethesda::debug;
#endif

void Bethesda::CommandHandler(signed int key, vector<double> info, double result)
{
    Lockable* data = NULL;

    if (key != 0x00000000)
        data = Lockable::BlindUnlock(key);

    using namespace Values;
    unsigned short opcode = (unsigned short) info.at(0);

    switch (opcode)
    {
        case Functions::Func_PlaceAtMe:
            Game::PlaceAtMe(data, *reinterpret_cast<unsigned int*>(&result));
            break;
        case Functions::Func_GetPos:
            Game::GetPos((unsigned int) info.at(1), (unsigned char) info.at(2), result);
            break;
        case Functions::Func_SetPos:
            break;
        case Functions::Func_GetAngle:
            Game::GetAngle((unsigned int) info.at(1), (unsigned char) info.at(2), result);
            break;
        case Functions::Func_SetAngle:
            break;
        case Functions::Func_GetActorValue:
            Game::GetActorValue((unsigned int) info.at(1), false, (unsigned char) info.at(2), result);
            break;
        case Functions::Func_ForceActorValue:
            break;
        case Functions::Func_GetBaseActorValue:
            Game::GetActorValue((unsigned int) info.at(1), true, (unsigned char) info.at(2), result);
            break;
        case Functions::Func_SetActorValue:
            break;
        case Functions::Func_GetDead:
            break;
        case Functions::Func_MoveTo:
            break;
        case Functions::Func_Enable:
            break;
        case Functions::Func_Disable:
            break;
        case Functions::Func_SetRestrained:
            break;
        case Fallout::Functions::Func_IsMoving:
            break;
        case Fallout::Functions::Func_MarkForDelete:
            break;
        case Fallout3::Functions::Func_GetParentCell:
        case FalloutNV::Functions::Func_GetParentCell:
        case Oblivion::Functions::Func_GetParentCell:
            Game::GetParentCell((unsigned int) info.at(1), *reinterpret_cast<unsigned int*>(&result));
            break;
        case Fallout3::Functions::Func_Load:
        case FalloutNV::Functions::Func_Load:
        case Oblivion::Functions::Func_Load:
            Game::SetName(PLAYER_REFERENCE);
            // reload game world
            break;
        case Fallout3::Functions::Func_SetName:
        case FalloutNV::Functions::Func_SetName:
        case Oblivion::Functions::Func_SetName:
            break;
        default:
            throw VaultException("Unhandled function %hX", opcode);
    }

    /*
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

            Interface::StartSession();

            ParamList param_SetRestrained;
            param_SetRestrained.push_back(player->GetActorRefParam());
            param_SetRestrained.push_back(Data::Param_True);
            ParamContainer SetRestrained = ParamContainer(param_SetRestrained, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("SetRestrained", SetRestrained, 0, 150);

            ParamList param_SetName;
            param_SetName.push_back(player->GetActorRefParam());
            param_SetName.push_back(player->GetActorNameParam());
            ParamContainer SetName = ParamContainer(param_SetName, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("SetName", SetName);

            ParamList param_RemoveAllItems;
            param_RemoveAllItems.push_back(player->GetActorRefParam());
            ParamContainer RemoveAllItems = ParamContainer(param_RemoveAllItems, &Data::AlwaysTrue);
            Interface::ExecuteCommandOnce("RemoveAllItems", RemoveAllItems);

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
                    Interface::ExecuteCommandOnce("AddItem", AddItem);
                }

                params = player->GetItemParamList_EquipItem(true, true);

                for (it = params.begin(); it != params.end(); ++it)
                {
                    ParamList param_EquipItem;
                    param_EquipItem.push_back(player->GetActorRefParam());
                    param_EquipItem.splice(param_EquipItem.end(), *it);
                    ParamContainer EquipItem = ParamContainer(param_EquipItem, &Data::AlwaysTrue);
                    Interface::ExecuteCommandOnce("EquipItem", EquipItem);
                }
            }

            Interface::EndSession();

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

                    lastRef->SetActorGameCell(self->GetActorGameCell());debug->
                    lastRef->ToggleNoOverride(SKIPFLAG_GETPARENTCELL, true);

                    Interface::StartSession();

                    if (lastRef->SetActorEnabled(true))
                    {
                        ParamList param_Enable;
                        param_Enable.push_back(lastRef->GetActorRefParam());
                        param_Enable.push_back(Data::Param_True);
                        ParamContainer Enable = ParamContainer(param_Enable, &Data::AlwaysTrue);
                        Interface::ExecuteCommandOnce("Enable", Enable, 0, 150);
                    }

                    ParamList param_MoveTo;
                    param_MoveTo.push_back(lastRef->GetActorRefParam());
                    param_MoveTo.push_back(self->GetActorRefParam());
                    param_MoveTo.push_back(Data::Param_False);
                    param_MoveTo.push_back(Data::Param_False);
                    param_MoveTo.push_back(Data::Param_False);
                    ParamContainer MoveTo = ParamContainer(param_MoveTo, &Data::AlwaysTrue);
                    Interface::ExecuteCommandOnce("MoveTo", MoveTo, 0, 150);

                    Interface::EndSession();
                }
            }

            lastRef->SetActorGameCell(cell);

            if (lastRef != self && !lastRef->GetReference().empty())
            {
                if (lastRef->GetActorNetworkCell() != self->GetActorGameCell() && !lastRef->IsActorNearPoint(self->GetActorPos(0), self->GetActorPos(1), self->GetActorPos(2), 20000.0))
                {
                    if (lastRef->SetActorEnabled(false))
                    {
                        Interface::StartSession();

                        ParamList param_Disable;
                        param_Disable.push_back(lastRef->GetActorRefParam());
                        param_Disable.push_back(Data::Param_False);
                        ParamContainer Disable = ParamContainer(param_Disable, &Data::AlwaysTrue);
                        Interface::ExecuteCommandOnce("Disable", Disable, 0, 150);

                        Interface::EndSession();
                    }
                }
                else
                {
                    if (lastRef->SetActorEnabled(true))
                    {
                        Interface::StartSession();

                        ParamList param_Enable;
                        param_Enable.push_back(lastRef->GetActorRefParam());
                        param_Enable.push_back(Data::Param_True);
                        ParamContainer Enable = ParamContainer(param_Enable, &Data::AlwaysTrue);
                        Interface::ExecuteCommandOnce("Enable", Enable, 0, 150);

                        Interface::EndSession();
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
    }*/
}

/*void Bethesda::StringHandler(string command)
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
                Interface::StartSession();

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
                Interface::ExecuteCommandOnce("Load", Load, 0, 150);

                ParamList param_SetName;
                param_SetName.push_back(self->GetActorRefParam());
                param_SetName.push_back(self->GetActorNameParam());
                ParamContainer SetName = ParamContainer(param_SetName, &Data::AlwaysTrue);
                Interface::ExecuteCommandOnce("SetName", SetName);

                Interface::EndSession();
            }
        }
        else
        {
            if (self->SetActorDead(false))
            {
                Interface::StartSession();

                map<RakNetGUID, Player*> players;
                map<RakNetGUID, Player*>::iterator it;
                players = Player::GetPlayerList();

                for (it = players.begin(); it != players.end(); ++it)
                {
                    Player* player = it->second;

                    if (player == self)
                        continue;

                    string baseID = game == 1 ? "8D0E7" : game == 0 ? "30D82" : "35EBE";

                    ParamList param_PlaceAtMe;
                    param_PlaceAtMe.push_back(self->GetActorRefParam());
                    param_PlaceAtMe.push_back(Data::BuildParameter(baseID));
                    param_PlaceAtMe.push_back(Data::Param_True);
                    param_PlaceAtMe.push_back(Data::Param_False);
                    param_PlaceAtMe.push_back(Data::Param_False);
                    ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
                    Interface::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, 1000);
                }

                Interface::EndSession();
            }
        }
    }
    else
    {
        #ifdef VAULTMP_DEBUG
        char text[256];
        snprintf(text, sizeof(text), "String could not be processed: %s", const_cast<char*>(high.c_str()));
        debug->Print(text, true);
        #endif
    }

    self->EndSession();
}*/

void Bethesda::Initialize()
{
    char module[32];

    switch (Bethesda::game = game)
    {
    case FALLOUT3:
        strcpy(module, "Fallout3.exe");
        break;
    case NEWVEGAS:
        SetEnvironmentVariable("SteamAppID", "22380");
        strcpy(module, "FalloutNV.exe");
        break;
    case OBLIVION:
        SetEnvironmentVariable("SteamAppID", "22330");
        strcpy(module, "Oblivion.exe");
        break;
    default:
        throw VaultException("Bad game ID %08X", Bethesda::game);
    }

    TCHAR savefile[MAX_PATH];
    ZeroMemory(savefile, sizeof(savefile));
    SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, savefile); // SHGFP_TYPE_CURRENT

    switch (Bethesda::game)
    {
    case FALLOUT3:
        strcat(savefile, "\\My Games\\Fallout3\\Saves\\");
        break;
    case NEWVEGAS:
        strcat(savefile, "\\My Games\\FalloutNV\\Saves\\");
        break;
    case OBLIVION:
        strcat(savefile, "\\My Games\\Oblivion\\Saves\\");
        break;
    }

    strcat(savefile, Utils::FileOnly(Bethesda::savegame.first.c_str()));
    unsigned int crc;

    if (!Utils::crc32file(savefile, &crc))
        throw VaultException("Could not find savegame file:\n\n%s\n\nAsk the server owner to send you the file or try to Synchronize with the server.", savefile);

    if (crc != Bethesda::savegame.second)
        throw VaultException("Savegame differs from the server version:\n\n%s\n\nAsk the server owner to send you the file or try to Synchronize with the server.", savefile);

    TCHAR curdir[MAX_PATH];
    ZeroMemory(curdir, sizeof(curdir));
    GetModuleFileName(GetModuleHandle(NULL), (LPTSTR) curdir, MAX_PATH);
    PathRemoveFileSpec(curdir);

    strcat(curdir, "\\Data\\");

    for (ModList::iterator it = modfiles.begin(); it != modfiles.end(); ++it)
    {
        TCHAR modfile[MAX_PATH];
        ZeroMemory(modfile, sizeof(modfile));
        strcat(modfile, curdir);
        strcat(modfile, it->first.c_str());

        if (!Utils::crc32file(modfile, &crc))
            throw VaultException("Could not find modification file:\n\n%s\n\nAsk the server owner to send you the file or try to Synchronize with the server.", modfile);

        if (crc != it->second)
            throw VaultException("Modfile differs from the server version:\n\n%s\n\nAsk the server owner to send you the file or try to Synchronize with the server.", modfile);
    }

    ZeroMemory(savefile, sizeof(savefile));
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, savefile); // SHGFP_TYPE_CURRENT

    switch (Bethesda::game)
    {
    case FALLOUT3:
        strcat(savefile, "\\Fallout3\\plugins.txt");
        break;
    case NEWVEGAS:
        strcat(savefile, "\\FalloutNV\\plugins.txt");
        break;
    case OBLIVION:
        strcat(savefile, "\\Oblivion\\Plugins.txt");
        break;
    }

    FILE* plugins = fopen(savefile, "w");

    switch (Bethesda::game)
    {
    case FALLOUT3:
    {
        char esm[] = "Fallout3.esm\n";
        fwrite(esm, sizeof(char), strlen(esm), plugins);
        break;
    }
    case NEWVEGAS:
    {
        char esm[] = "FalloutNV.esm\n";
        fwrite(esm, sizeof(char), strlen(esm), plugins);
        break;
    }
    }

    for (ModList::iterator it = modfiles.begin(); it != modfiles.end(); ++it)
    {
        fwrite(it->first.c_str(), sizeof(char), it->first.length(), plugins);
        fwrite("\n", sizeof(char), 1, plugins);
    }

    fclose(plugins);

    if (Interface::lookupProgramID(module) == 0)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);

        if (CreateProcess(module, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
        {
            HANDLE hProc;

            hProc = OpenProcess(PROCESS_ALL_ACCESS, false, pi.dwProcessId);

            if (hProc)
            {
                GetModuleFileName(GetModuleHandle(NULL), (LPTSTR) curdir, MAX_PATH);
                PathRemoveFileSpec(curdir);
                strcat(curdir, "\\vaultmp.dll");
                unsigned int size = strlen(curdir) + 16;

                HINSTANCE hDll = LoadLibrary("kernel32.dll");
                fLoadLibrary LoadLibrary = (fLoadLibrary) GetProcAddress(hDll, "LoadLibraryA");

                LPVOID remote = VirtualAllocEx(hProc, 0, size, MEM_COMMIT, PAGE_READWRITE);
                WriteProcessMemory(hProc, remote, curdir, size, NULL);
                CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE) LoadLibrary, remote, 0, 0);

                try
                {
                    Interface::Initialize(module, &CommandHandler, Bethesda::game);

                    for (int i = 0; i < 10 && !Interface::IsAvailable(); i++)
                        Sleep(50);

                    if (!Interface::IsAvailable())
                        throw VaultException("Failed connecting to vaultmp interface!");
                }
                catch (...)
                {
                    CloseHandle(hProc);
                    throw;
                }

                ResumeThread(pi.hThread);

                Sleep(5000);

                initialized = true;

                CloseHandle(hProc);
            }
            else
                throw VaultException("Failed opening the game process!");
        }
        else
            throw VaultException("Failed creating the game process!");
    }
    else
        throw VaultException("Either Fallout 3, Fallout: New Vegas or TES: Oblivion is already runnning!");
}

void Bethesda::InitializeVaultMP(RakPeerInterface* peer, SystemAddress server, string name, string pwd, int game)
{
    Bethesda::game = game;
    Bethesda::password = pwd;
    Bethesda::savegame = Savegame();
    Bethesda::modfiles.clear();
    Game::game = game;
    initialized = false;

#ifdef VAULTMP_DEBUG
    debug = new Debug((char*) "vaultmp");
    debug->PrintFormat("Vault-Tec Multiplayer Mod client debug log (%s)", false, CLIENT_VERSION);
    debug->PrintFormat("Connecting to server: %s (name: %s, password: %s, game: %s)", false, server.ToString(), name.c_str(), pwd.c_str(), game == FALLOUT3 ? (char*) "Fallout 3" : game == NEWVEGAS ? (char*) "Fallout New Vegas" : (char*) "TES Oblivion");
    debug->Print("Visit www.vaultmp.com for help and upload this log if you experience problems with the mod.", false);
    debug->Print("-----------------------------------------------------------------------------------------------------", false);
    //debug->PrintSystem();
    VaultException::SetDebugHandler(debug);
    NetworkClient::SetDebugHandler(debug);
    Interface::SetDebugHandler(debug);
    Lockable::SetDebugHandler(debug);
    Object::SetDebugHandler(debug);
    Item::SetDebugHandler(debug);
    Container::SetDebugHandler(debug);
    Actor::SetDebugHandler(debug);
    Player::SetDebugHandler(debug);
    GameFactory::SetDebugHandler(debug);
#endif

    API::Initialize(game);
    Container::Initialize(game);

    Player* self = (Player*) GameFactory::CreateInstance(ID_PLAYER, PLAYER_REFERENCE, PLAYER_BASE);
    self->SetEnabled(true);
    self->SetName(name);
    GameFactory::LeaveReference(self);
    self = NULL; // lets make sure that we dont use this by accident somewhere (old version code did so), I prefer a crash over a bug hard to track

    Network::Flush();

    try
    {
        if (peer->Connect(server.ToString(false), server.GetPort(), DEDICATED_VERSION, sizeof(DEDICATED_VERSION), 0, 0, 3, 500, 0) == CONNECTION_ATTEMPT_STARTED)
        {
            bool query = true;
            Packet* packet;

            while (query)
            {
                NetworkResponse response;

                while ((response = Network::Next()).size())
                    Network::Dispatch(peer, response);

                for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
                {
                    if (packet->data[0] == ID_DISCONNECTION_NOTIFICATION)
                        query = false;
                    else if (packet->data[0] == ID_CONNECTION_REQUEST_ACCEPTED)
                        Game::server = peer->GetGuidFromSystemAddress(server);

                    try
                    {
                        response = NetworkClient::ProcessPacket(packet);
                        Network::Dispatch(peer, response);
                    }
                    catch (...)
                    {
                        peer->DeallocatePacket(packet);
                        response = NetworkClient::ProcessEvent(ID_EVENT_CLIENT_ERROR);
                        Network::Dispatch(peer, response);
                        peer->CloseConnection(server, true, CHANNEL_SYSTEM, HIGH_PRIORITY);
                        throw;
                    }
                }

                if (initialized && !Interface::IsAvailable())
                {
                    NetworkResponse response = NetworkClient::ProcessEvent(ID_EVENT_INTERFACE_LOST);
                    Network::Dispatch(peer, response);
                    peer->CloseConnection(server, true, CHANNEL_SYSTEM, HIGH_PRIORITY);
                    throw VaultException("Lost connection to interface");
                }


                switch (packet->data[0])
                {
                    /*
                                        case ID_GAME_RUN:
                                        {
                       BitStream query(packet->data, packet->length, false);
                       query.IgnoreBytes(sizeof(MessageID));

                       RakString save;
                       query.Read(save);
                       query.Reset();

                       savegame = string(save.C_String());

                       Interface::StartSession();

                       ParamList param_Load;
                       param_Load.push_back(Data::BuildParameter(savegame));
                       ParamContainer Load = ParamContainer(param_Load, &Data::AlwaysTrue);
                       Interface::ExecuteCommandOnce("Load", Load, 0, 150);

                       ParamList param_SetName;
                       param_SetName.push_back(self->GetActorRefParam());
                       param_SetName.push_back(self->GetActorNameParam());
                       ParamContainer SetName = ParamContainer(param_SetName, &Data::AlwaysTrue);
                       Interface::ExecuteCommandOnce("SetName", SetName);

                       Interface::EndSession();

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
                       debug->PrintFormat("New player on the server (name: %s, guid: %s)", true, name.C_String(), guid.ToString());
                        #endif

                       string baseID = game == NEWVEGAS ? "8D0E7" : game == FALLOUT3 ? "30D82" : "35EBE";

                       Player* player = new Player(guid, baseID);
                       player->SetActorName(string(name.C_String()));
                       refqueue.push_back(player);

                       Interface::StartSession();

                       ParamList param_PlaceAtMe;
                       param_PlaceAtMe.push_back(self->GetActorRefParam());
                       param_PlaceAtMe.push_back(Data::BuildParameter(baseID));
                       param_PlaceAtMe.push_back(Data::Param_True);
                       param_PlaceAtMe.push_back(Data::Param_False);
                       param_PlaceAtMe.push_back(Data::Param_False);
                       ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
                       Interface::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, 1000);

                       Interface::EndSession();
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
                           debug->PrintFormat("ERROR: Player could not be found (guid: %s)", true, guid.ToString());
                        #endif
                           break;
                       }

                       string refID = player->GetReference();

                        #ifdef VAULTMP_DEBUG
                       debug->PrintFormat("Player left (name: %s, guid: %s)", true, player->GetActorName().c_str(), guid.ToString());
                        #endif

                       if (!refID.empty())
                       {
                           Interface::StartSession();

                           if (player->SetActorEnabled(false))
                           {
                               ParamList param_Disable;
                               param_Disable.push_back(player->GetActorRefParam());
                               param_Disable.push_back(Data::Param_False);
                               ParamContainer Disable = ParamContainer(param_Disable, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("Disable", Disable, 0, 150);
                           }

                           ParamList param_MarkForDelete;
                           param_MarkForDelete.push_back(player->GetActorRefParam());
                           ParamContainer MarkForDelete = ParamContainer(param_MarkForDelete, &Data::AlwaysTrue);
                           Interface::ExecuteCommandOnce("MarkForDelete", MarkForDelete, 0, 150);

                           Interface::EndSession();
                       }

                       player->StartSession();
                       delete player;
                       player->EndSession();

                       refqueue.remove(player);
                       break;
                                        }
                                        /*case ID_PLAYER_UPDATE:
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
                           Interface::StartSession();

                           char pos[16];

                           if (player->SetActorAngle(update->A) && player->GetActorEnabled())
                           {
                               snprintf(pos, sizeof(pos), "%f", update->A);

                               ParamList param_SetAngle;
                               param_SetAngle.push_back(player->GetActorRefParam());
                               param_SetAngle.push_back(Data::Param_Z);
                               param_SetAngle.push_back(Data::BuildParameter(string(pos)));
                               ParamContainer SetAngle = ParamContainer(param_SetAngle, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("SetAngle", SetAngle);
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
                               Interface::ExecuteCommandOnce("PlayGroup", PlayGroup);
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
                                   Interface::ExecuteCommandOnce("SetPos", SetPos);
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
                                   Interface::ExecuteCommandOnce("SetPos", SetPos);
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
                                   Interface::ExecuteCommandOnce("SetPos", SetPos);
                               }
                           }


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
                           }

                           if (player->SetActorAlerted(update->alerted) && player->GetActorEnabled())
                           {
                               ParamList param_SetRestrained;
                               param_SetRestrained.push_back(player->GetActorRefParam());
                               param_SetRestrained.push_back(Data::Param_False);
                               ParamContainer SetRestrained = ParamContainer(param_SetRestrained, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("SetRestrained", SetRestrained, 0, 150);

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
                               Interface::ExecuteCommandOnce("SetAlert", SetAlert, 0, 150);

                               param_SetRestrained.clear();
                               param_SetRestrained.push_back(player->GetActorRefParam());
                               param_SetRestrained.push_back(Data::Param_True);
                               SetRestrained = ParamContainer(param_SetRestrained, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("SetRestrained", SetRestrained, 0, 150);
                           }

                           Interface::EndSession();
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
                           Interface::StartSession();

                           char pos[16];

                           if (player->SetActorBaseHealth(update->baseHealth))
                           {
                               snprintf(pos, sizeof(pos), "%i", (int) update->baseHealth);

                               ParamList param_SetActorValue;
                               param_SetActorValue.push_back(player->GetActorRefParam());
                               param_SetActorValue.push_back(Data::BuildParameter("Health"));
                               param_SetActorValue.push_back(Data::BuildParameter(pos));
                               ParamContainer SetActorValue = ParamContainer(param_SetActorValue, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("SetActorValue", SetActorValue);
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
                               Interface::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                           }

                           if (player->SetActorCondition(COND_PERCEPTION, update->conds[COND_PERCEPTION]))
                           {
                               snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_PERCEPTION]);

                               ParamList param_ForceActorValue;
                               param_ForceActorValue.push_back(player->GetActorRefParam());
                               param_ForceActorValue.push_back(Data::BuildParameter("PerceptionCondition"));
                               param_ForceActorValue.push_back(Data::BuildParameter(pos));
                               ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                           }

                           if (player->SetActorCondition(COND_ENDURANCE, update->conds[COND_ENDURANCE]))
                           {
                               snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_ENDURANCE]);

                               ParamList param_ForceActorValue;
                               param_ForceActorValue.push_back(player->GetActorRefParam());
                               param_ForceActorValue.push_back(Data::BuildParameter("EnduranceCondition"));
                               param_ForceActorValue.push_back(Data::BuildParameter(pos));
                               ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                           }

                           if (player->SetActorCondition(COND_LEFTATTACK, update->conds[COND_LEFTATTACK]))
                           {
                               snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_LEFTATTACK]);

                               ParamList param_ForceActorValue;
                               param_ForceActorValue.push_back(player->GetActorRefParam());
                               param_ForceActorValue.push_back(Data::BuildParameter("LeftAttackCondition"));
                               param_ForceActorValue.push_back(Data::BuildParameter(pos));
                               ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                           }

                           if (player->SetActorCondition(COND_RIGHTATTACK, update->conds[COND_RIGHTATTACK]))
                           {
                               snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_RIGHTATTACK]);

                               ParamList param_ForceActorValue;
                               param_ForceActorValue.push_back(player->GetActorRefParam());
                               param_ForceActorValue.push_back(Data::BuildParameter("RightAttackCondition"));
                               param_ForceActorValue.push_back(Data::BuildParameter(pos));
                               ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                           }

                           if (player->SetActorCondition(COND_LEFTMOBILITY, update->conds[COND_LEFTMOBILITY]))
                           {
                               snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_LEFTMOBILITY]);

                               ParamList param_ForceActorValue;
                               param_ForceActorValue.push_back(player->GetActorRefParam());
                               param_ForceActorValue.push_back(Data::BuildParameter("LeftMobilityCondition"));
                               param_ForceActorValue.push_back(Data::BuildParameter(pos));
                               ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
                           }

                           if (player->SetActorCondition(COND_RIGHTMOBILITY, update->conds[COND_RIGHTMOBILITY]))
                           {
                               snprintf(pos, sizeof(pos), "%i", (int) update->conds[COND_RIGHTMOBILITY]);

                               ParamList param_ForceActorValue;
                               param_ForceActorValue.push_back(player->GetActorRefParam());
                               param_ForceActorValue.push_back(Data::BuildParameter("RightMobilityCondition"));
                               param_ForceActorValue.push_back(Data::BuildParameter(pos));
                               ParamContainer ForceActorValue = ParamContainer(param_ForceActorValue, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("ForceActorValue", ForceActorValue);
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
                                   Interface::ExecuteCommandOnce("Kill", Kill, 0, 150);
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
                                       Interface::ExecuteCommandOnce("Disable", Disable, 0, 150);
                                   }

                                   ParamList param_MarkForDelete;
                                   param_MarkForDelete.push_back(player->GetActorRefParam());
                                   ParamContainer MarkForDelete = ParamContainer(param_MarkForDelete, &Data::AlwaysTrue);
                                   Interface::ExecuteCommandOnce("MarkForDelete", MarkForDelete, 0, 150);

                                   player->SetReference("");

                                   refqueue.push_back(player);

                                   string baseID = game == NEWVEGAS ? "8D0E7" : game == FALLOUT3 ? "30D82" : "35EBE";

                                   ParamList param_PlaceAtMe;
                                   param_PlaceAtMe.push_back(self->GetActorRefParam());
                                   param_PlaceAtMe.push_back(Data::BuildParameter(baseID));
                                   param_PlaceAtMe.push_back(Data::Param_True);
                                   param_PlaceAtMe.push_back(Data::Param_False);
                                   param_PlaceAtMe.push_back(Data::Param_False);
                                   ParamContainer PlaceAtMe = ParamContainer(param_PlaceAtMe, &Data::AlwaysTrue);
                                   Interface::ExecuteCommandOnce("PlaceAtMe", PlaceAtMe, 0, 1000);
                                   break;
                               }
                               }
                           }

                           Interface::EndSession();
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

                           if (player == self)
                               player->ToggleNoOverride(SKIPFLAG_ITEMUPDATE, true);

                           Interface::StartSession();

                           if (update->item.count > 0)
                           {
                               ParamList param_AddItem;
                               param_AddItem.push_back(player->GetActorRefParam());
                               param_AddItem.push_back(Inventory::GetItemBaseParam(&update->item));
                               param_AddItem.push_back(Inventory::GetItemCountParam(&update->item));
                               param_AddItem.push_back(update->hidden ? Data::Param_True : Data::Param_False);
                               ParamContainer AddItem = ParamContainer(param_AddItem, &Data::AlwaysTrue);
                               Interface::ExecuteCommandOnce("AddItem", AddItem);
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
                               Interface::ExecuteCommandOnce("RemoveItem", RemoveItem);
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
                                   Interface::ExecuteCommandOnce("EquipItem", EquipItem);
                               }
                               else if (update->item.condition != 0.00)
                               {
                                   ParamList param_UnequipItem;
                                   param_UnequipItem.push_back(player->GetActorRefParam());
                                   param_UnequipItem.push_back(Inventory::GetItemBaseParam(&update->item));
                                   param_UnequipItem.push_back(player != self ? Data::Param_True : Data::Param_False);
                                   param_UnequipItem.push_back(update->hidden ? Data::Param_True : Data::Param_False);
                                   ParamContainer UnequipItem = ParamContainer(param_UnequipItem, &Data::AlwaysTrue);
                                   Interface::ExecuteCommandOnce("UnequipItem", UnequipItem);
                               }
                           }

                           Interface::EndSession();
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
                       break;*/
                }


                /*if (self->UpdateActorUpdateStruct(&localPlayerUpdate))
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
                }*/

                RakSleep(2);
            }
        }
        else
            throw VaultException("Could not establish connection to server");
    }
    catch (...)
    {
        Sleep(200);
        Packet* packet = NULL;
        while (packet = peer->Receive()) peer->DeallocatePacket(packet); // disconnection notification might still arrive

        Interface::Terminate();
        GameFactory::DestroyAllInstances();
        Container::Cleanup();
        API::Terminate();

#ifdef VAULTMP_DEBUG
        debug->Print("Network thread is going to terminate (ERROR)", true);
#endif
        throw;
    }

    Interface::Terminate();
    GameFactory::DestroyAllInstances();
    Container::Cleanup();
    API::Terminate();

#ifdef VAULTMP_DEBUG
    debug->Print("Network thread is going to terminate (no error occured)", true);
#endif
}
