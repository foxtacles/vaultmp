#include <windows.h>
#include <cstdio>
#include <vector>
#include <string>

#include "vaultmp.h"

using namespace std;

typedef void (*CallCommand)(void*, void*, void*, void*, void*, void*, void*, void*);
typedef unsigned int (*LookupForm)(unsigned int);
typedef unsigned int (*LookupFunc)(unsigned int);

static HANDLE hProc;
static PipeServer pipeServer;
static PipeClient pipeClient;
static LookupForm FormLookup;
static LookupFunc FuncLookup;

static void PatchGame(HINSTANCE& silverlock);
static void BethesdaDelegator();
static vector<void*> delegated;

static bool delegate = false;
static bool DLLerror = false;
static unsigned char game = 0x00;

static const unsigned FalloutNVpatch_PlayGroup = 0x00494D5C;
static const unsigned FalloutNVpatch_delegator_src = 0x0086B3E3;
static const unsigned FalloutNVpatch_delegator_dest = 0x0086E649;
static const unsigned FalloutNVpatch_delegatorCall_src = 0x0086E64A;
static const unsigned FalloutNVpatch_delegatorCall_dest = (unsigned) &BethesdaDelegator;

static const unsigned Fallout3patch_PlayGroup = 0x0045F704;
static const unsigned Fallout3patch_delegator_src = 0x006EEC86;
static const unsigned Fallout3patch_delegator_dest = 0x006EDBD9;
static const unsigned Fallout3patch_delegatorCall_src = 0x006EDBDA;
static const unsigned Fallout3patch_delegatorCall_dest = (unsigned) &BethesdaDelegator;

static const unsigned OblivionPatch_delegator_src = 0x0040F270;
static const unsigned OblivionPatch_delegator_dest = 0x0040F753;
static const unsigned OblivionPatch_delegator_ret_src = 0x0040F75A;
static const unsigned OblivionPatch_delegator_ret_dest = 0x0040D800;
static const unsigned OblivionPatch_delegatorCall_src = 0x0040F754;
static const unsigned OblivionPatch_delegatorCall_dest = (unsigned) &BethesdaDelegator;

// Those snippets are from FOSE, thanks

void SafeWrite8(unsigned int addr, unsigned int data)
{
    unsigned int oldProtect;

    VirtualProtect((void*) addr, 4, PAGE_EXECUTE_READWRITE, (DWORD*) &oldProtect);
    *((unsigned char*) addr) = data;
    VirtualProtect((void*) addr, 4, oldProtect, (DWORD*) &oldProtect);
}

void SafeWrite16(unsigned int addr, unsigned int data)
{
    unsigned int oldProtect;

    VirtualProtect((void*) addr, 4, PAGE_EXECUTE_READWRITE, (DWORD*) &oldProtect);
    *((unsigned short *)addr) = data;
    VirtualProtect((void*) addr, 4, oldProtect, (DWORD*) &oldProtect);
}

void SafeWrite32(unsigned int addr, unsigned int data)
{
    unsigned int oldProtect;

    VirtualProtect((void*) addr, 4, PAGE_EXECUTE_READWRITE, (DWORD*) &oldProtect);
    *((unsigned int*) addr) = data;
    VirtualProtect((void*) addr, 4, oldProtect, (DWORD*) &oldProtect);
}

void SafeWriteBuf(unsigned int addr, void * data, unsigned int len)
{
    unsigned int oldProtect;

    VirtualProtect((void*) addr, len, PAGE_EXECUTE_READWRITE, (DWORD*) &oldProtect);
    memcpy((void*) addr, data, len);
    VirtualProtect((void*) addr, len, oldProtect, (DWORD*) &oldProtect);
}

void WriteRelJump(unsigned int jumpSrc, unsigned int jumpTgt)
{
    SafeWrite8(jumpSrc, 0xE9);
    SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
}

void WriteRelCall(unsigned int jumpSrc, unsigned int jumpTgt)
{
    SafeWrite8(jumpSrc, 0xE8);
    SafeWrite32(jumpSrc + 1, jumpTgt - jumpSrc - 1 - 4);
}

void BethesdaDelegator()
{
    if (delegate)
    {
        CallCommand Call = (CallCommand) delegated.at(8);
        Call(delegated.at(0), delegated.at(1), delegated.at(2), delegated.at(3), delegated.at(4), delegated.at(5), delegated.at(6), delegated.at(7));
        delegate = false;
    }
}

bool vaultfunction(void* reference, void* result, void* args, unsigned short opcode)
{
    switch (opcode)
    {
    case 0xE001: // GetActorState - returns the actors animations, alerted / sneaking state
    {
        ZeroMemory(result, sizeof(double));
        unsigned char* data;

        // thiscall convention
        asm (
            "MOV ECX,%1\n"
            "CALL %2\n"
            "MOV %0,EAX\n"
            : "=m"(data)
            : "m"(reference), "r"((game & FALLOUT_GAMES) ? (*((unsigned int*) ((unsigned) *((unsigned int*) reference) + (unsigned) 0x01E4)))
                                                         : (*((unsigned int*) ((unsigned) *((unsigned int*) reference) + (unsigned) 0x0164))))
            : "ecx"
        );

        if (data != NULL)
        {
            unsigned char alerted, sneaking, running;

            if (game & FALLOUT_GAMES)
            {
                alerted = *(data + 0x6C) != 0xFF ? 0x01 : 0x00;
                sneaking = *(data + 0x4D) == 0x10 ? 0x01 : 0x00;
                running = *(data + 0x4E);
            }
            else
            {
                alerted = *(data + 0x5C) != 0xFF ? 0x01 : 0x00;
                sneaking = *(data + 0x3D) == 0x10 ? 0x01 : 0x00;
                running = *(data + 0x3C);
            }

            memcpy(result, &alerted, 1);
            memcpy((void*) ((unsigned) result + 1), &sneaking, 1);
            memcpy((void*) ((unsigned) result + 4), &running, 1);

            // This detection is unreliable; i.e. what when the player holds Forward/Backward/Left/Right all together for some reason?
            unsigned char* _args = (unsigned char*) args;
            if (*_args == 0x6E) // Forward, Backward, Left, Right unsigned char scan codes stored in Integer, optional
            {
                ++_args;
                unsigned char forward = *_args;
                unsigned char backward = *(_args + 1);
                unsigned char left = *(_args + 2);
                unsigned char right = *(_args + 3);

                if (!(forward && backward && left && right))
                    break;

                // MAPVK_VSC_TO_VK
                if (((GetAsyncKeyState(MapVirtualKey(forward, 1)) & 0x8000) && (GetAsyncKeyState(MapVirtualKey(left, 1)) & 0x8000))
                        || ((GetAsyncKeyState(MapVirtualKey(backward, 1)) & 0x8000) && (GetAsyncKeyState(MapVirtualKey(right, 1)) & 0x8000)))
                {
                    unsigned char type = 0x01; // that equals to a Z-angle correction of -45²
                    memcpy((void*) ((unsigned) result + 5), &type, 1);
                }
                else if (((GetAsyncKeyState(MapVirtualKey(forward, 1)) & 0x8000) && (GetAsyncKeyState(MapVirtualKey(right, 1)) & 0x8000))
                         || ((GetAsyncKeyState(MapVirtualKey(backward, 1)) & 0x8000) && (GetAsyncKeyState(MapVirtualKey(left, 1)) & 0x8000)))
                {
                    unsigned char type = 0x02; // that equals to a Z-angle correction of 45²
                    memcpy((void*) ((unsigned) result + 5), &type, 1);
                }
            }
        }
        break;
    }
    case 0xE002: // ScanContainer - Returns a containers content including baseID, amount, condition, equipped state
    {
        ZeroMemory(result, sizeof(double));

        if (!(game & FALLOUT_GAMES))
            break;

        unsigned int count;

        asm (
            "MOV ECX,%1\n"
            "PUSH 1\n"
            "PUSH 0\n"
            "CALL %2\n"
            "MOV %0,EAX\n"
            : "=m"(count)
            : "m"(reference), "r"((game & FALLOUT3) ? ITEM_COUNT_FALLOUT3 : ITEM_COUNT_NEWVEGAS)
            : "ecx"
        );

        if (count > 0)
        {
            unsigned int size = count * 20;
            vector<unsigned char> container;
            container.reserve(size);

            for (int i = 0; i < count; ++i)
            {
                unsigned int item;

                asm (
                    "MOV ECX,%2\n"
                    "PUSH 0\n"
                    "PUSH %1\n"
                    "CALL %3\n"
                    "MOV %0,EAX\n"
                    : "=m"(item)
                    : "r"(i), "m"(reference), "r"((game & FALLOUT3) ? ITEM_GET_FALLOUT3 : ITEM_GET_NEWVEGAS)
                    : "ecx"
                );

                if (item)
                {
                    unsigned int amount = *((unsigned int*) (((unsigned) item) + 0x04));
                    unsigned int baseForm = *((unsigned int*) (((unsigned) item) + 0x08));

                    if (baseForm)
                    {
                        unsigned char type = *((unsigned char*) (((unsigned) baseForm) + 0x04));
                        unsigned int baseID = *((unsigned int*) (((unsigned) baseForm) + 0x0C));

                        container.insert(container.end(), (unsigned char*) &baseID, ((unsigned char*) &baseID) + 4);
                        container.insert(container.end(), (unsigned char*) &amount, ((unsigned char*) &amount) + 4);

                        unsigned int equipped;

                        asm (
                            "MOV ECX,%1\n"
                            "PUSH 0\n"
                            "CALL %2\n"
                            "MOV %0,EAX\n"
                            : "=m"(equipped)
                            : "m"(item), "r"((game & FALLOUT3) ? ITEM_ISEQUIPPED_FALLOUT3 : ITEM_ISEQUIPPED_NEWVEGAS)
                            : "ecx"
                        );

                        equipped &= 0x00000001;

                        container.insert(container.end(), (unsigned char*) &equipped, ((unsigned char*) &equipped) + 4);

                        double condition;

                        if (type == 0x18 || type == 0x28)
                        {
                            asm (
                                "MOV ECX,%1\n"
                                "PUSH 1\n"
                                "CALL %2\n"
                                "FSTP QWORD PTR %0\n"
                                : "=m"(condition)
                                : "m"(item), "r"((game & FALLOUT3) ? ITEM_CONDITION_FALLOUT3 : ITEM_CONDITION_NEWVEGAS)
                                : "ecx"
                            );
                        }

                        container.insert(container.end(), (unsigned char*) &condition, ((unsigned char*) &condition) + 8);

                        // Kind of cleanup here? not sure what this is

                        /*if (game & FALLOUT3)
                        {
                            asm (
                                "MOV ECX,%0\n"
                                "CALL %1\n"
                                "PUSH %0\n"
                                :
                                : "m"(item), "r"(ITEM_UNK1_FALLOUT3)
                                : "ecx"
                            );

                            asm (
                                "MOV ECX,%0\n"
                                "CALL %1\n"
                                :
                                : "r"(ITEM_UNK3_FALLOUT3), "r"(ITEM_UNK2_FALLOUT3)
                                : "ecx"
                            );
                        }
                        else
                        {
                            asm (
                                "MOV ECX,%0\n"
                                "PUSH 1\n"
                                "CALL %1\n"
                                :
                                : "m"(item), "r"(ITEM_UNK1_NEWVEGAS)
                                : "ecx"
                            );
                        }*/
                    }
                    else
                        return true;
                }
            }

            size = container.size();
            unsigned char* data = new unsigned char[size];
            memcpy(data, &container[0], size);

            memcpy(result, &size, 4);
            memcpy((void*) ((unsigned) result + 4), &data, 4);
        }

        return true;
    }
    default:
        break;
    }

    return false;
}

void ExecuteCommand(vector<void*>& args, unsigned int crc, bool delegate_flag)
{
    if (args.size() != 8)
        return;

    unsigned int reference = *((unsigned int*) args.at(2));
    unsigned short opcode;
    void* _args;

    if (*((unsigned int*) args.at(1)) == 0x0001001C)
    {
        opcode = *((unsigned short*) (((unsigned) args.at(1)) + 4));
        _args = (void*) (((unsigned) args.at(1)) + 4 + 2 + 2 + 2); // skip 0001001C, opcode, unk2, numargs
    }
    else
    {
        opcode = *((unsigned short*) args.at(1));
        _args = (void*) (((unsigned) args.at(1)) + 2 + 2 + 2); // skip opcode, unk2, numargs
    }

    if (opcode == 0x00)
        return;

    if (reference != 0x00)
    {
        reference = FormLookup(reference);

        if (reference == 0x00)
            return;
    }

    void* arg4 = args.at(4);
    unsigned int base = (unsigned) arg4;

    unsigned int** param1 = (unsigned int**) (base + 0x44);
    unsigned int*** param2 = (unsigned int***) (base + 0x48);

    if (*param2 == 0x00000000)
    {
        param1 = (unsigned int**) (base + 0x40);
        param2 = (unsigned int***) (base + 0x44);
    }

    *param1 = (unsigned int*) ((unsigned) *param1 + base);
    *param2 = (unsigned int**) ((unsigned) *param2 + base);
    **param2 = (unsigned int*) ((unsigned) **param2 + base);

    unsigned int param1_ref = *((unsigned int*) (((unsigned) *param1) + 0x08));
    unsigned int param2_ref = *((unsigned int*) (((unsigned) **param2) + 0x08));

    if (param1_ref != 0x00)
    {
        param1_ref = FormLookup(param1_ref);

        if (param1_ref == 0x00)
            return;

        *((unsigned int*) (((unsigned) *param1) + 0x08)) = param1_ref;
    }

    if (param2_ref != 0x00)
    {
        param2_ref = FormLookup(param2_ref);

        if (param2_ref == 0x00)
            return;

        *((unsigned int*) (((unsigned) **param2) + 0x08)) = param2_ref;
    }

    bool bigresult = false;

    if ((opcode & VAULTFUNCTION) == VAULTFUNCTION)
        bigresult = vaultfunction((void*) reference, args.at(6), _args, opcode);
    else
    {
        unsigned int function = FuncLookup((unsigned int) opcode);

        if (function == 0x00)
            return;

        void* callAddr = (void*) *((unsigned int*) (function + 0x18));

        if (callAddr == 0x00)
            return;

        if (delegate_flag)
        {
            delegated.clear();
            delegated.reserve(9);
            delegated.push_back(args.at(0));
            delegated.push_back(args.at(1));
            delegated.push_back((void*) reference);
            delegated.push_back((void*) *((unsigned int*) args.at(3)));
            delegated.push_back(args.at(4));
            delegated.push_back((void*) &arg4);
            delegated.push_back(args.at(6));
            delegated.push_back(args.at(7));
            delegated.push_back(callAddr);
            delegate = true;

            while (delegate) Sleep(10);
        }
        else
        {
            CallCommand Call = (CallCommand) callAddr;
            Call(args.at(0), args.at(1), (void*) reference, (void*) *((unsigned int*) args.at(3)), args.at(4), (void*) &arg4, args.at(6), args.at(7));
        }
    }

    char result[PIPE_LENGTH];
    ZeroMemory(result, sizeof(result));

    *((unsigned int*) ((unsigned) result + 1)) = crc;

    if (!bigresult)
    {
        result[0] = PIPE_OP_RETURN;
        memcpy(result + 5, args.at(6), sizeof(double));
    }
    else
    {
        result[0] = PIPE_OP_RETURN_BIG;
        void* data = args.at(6);
        unsigned int size = *((unsigned int*) data);
        unsigned char* _data = (unsigned char*) *((unsigned int*) (((unsigned) data) + 4));
        if (size && size <= (PIPE_LENGTH - 9))
        {
            memcpy(result + 5, &size, 4);
            memcpy(result + 9, _data, size);
        }
        delete[] _data;
    }

    pipeClient.Send(result);
}

DWORD WINAPI vaultmp_pipe(LPVOID data)
{
    /* Loading and initalizing vaultgui.dll */

    HINSTANCE vaultgui = NULL;
    HINSTANCE silverlock = NULL;

    vaultgui = LoadLibrary("vaultgui.dll");

    if (vaultgui != NULL)
    {

    }
    /*else
        DLLerror = true;*/

    pipeClient.SetPipeAttributes("BethesdaClient", PIPE_LENGTH);
    while (!pipeClient.ConnectToServer());

    pipeServer.SetPipeAttributes("BethesdaServer", PIPE_LENGTH);
    pipeServer.CreateServer();
    pipeServer.ConnectToServer();

    char buffer[PIPE_LENGTH];
    char code;
    ZeroMemory(buffer, sizeof(buffer));

    buffer[0] = PIPE_SYS_WAKEUP;
    pipeClient.Send(buffer);

    Sleep(3000);
    PatchGame(silverlock);

    if (DLLerror)
    {
        ZeroMemory(buffer, sizeof(buffer));
        buffer[0] = PIPE_ERROR_CLOSE;
        pipeClient.Send(buffer);
    }

    while (!DLLerror)
    {
        ZeroMemory(buffer, sizeof(buffer));

        pipeServer.Receive(buffer);
        code = buffer[0];
        char* content = buffer + 1;

        switch (code)
        {
        case PIPE_OP_COMMAND:
        {
            vector<void*> args;
            args.clear();
            args.reserve(8);

            unsigned int crc = *((unsigned int*) content);
            content += 4;

            if (Utils::crc32buf(content, PIPE_LENGTH - 5) == crc)
            {
                bool delegate_flag = (bool) *content;
                content += 1;

                for (int i = 0; i < 8; i++)
                {
                    unsigned char size = *content;

                    if (size != 0)
                    {
                        char* arg = new char[size];
                        content++;

                        memcpy(arg, content, size);
                        content += size;

                        args.push_back((void*) arg);
                    }
                    else
                    {
                        buffer[0] = PIPE_ERROR_CLOSE;
                        pipeClient.Send(buffer);
                        DLLerror = true;
                    }
                }

                if (!DLLerror)
                    ExecuteCommand(args, crc, delegate_flag);

                for (int i = 0; i < args.size(); i++)
                {
                    char* arg = (char*) args[i];
                    delete[] arg;
                }
            }

            break;
        }
        case PIPE_GUI_MESSAGE:
        {
            if (vaultgui != NULL)
            {
            }

            break;
        }
        case PIPE_ERROR_CLOSE:
        {
            DLLerror = true;

            break;
        }
        }
    }

    if (vaultgui != NULL)
        FreeLibrary(vaultgui);

    if (silverlock != NULL)
        FreeLibrary(silverlock);

    return ((DWORD) data);
}

void PatchGame(HINSTANCE& silverlock)
{
    TCHAR curdir[MAX_PATH];
    ZeroMemory(curdir, sizeof(curdir));
    GetModuleFileName(NULL, (LPTSTR) curdir, MAX_PATH);

    /* Loading FOSE / NVSE */

    silverlock = NULL;

    if (strstr(curdir, "Fallout3.exe"))
    {
        game = FALLOUT3;
        silverlock = LoadLibrary("fose_1_7.dll");
    }
    else if (strstr(curdir, "FalloutNV.exe"))
    {
        game = NEWVEGAS;
        silverlock = LoadLibrary("nvse_1_1.dll");
    }
    else if (strstr(curdir, "Oblivion.exe"))
    {
        game = OBLIVION;
        silverlock = LoadLibrary("obse_1_2_416.dll");
    }

    if (silverlock == NULL)
        DLLerror = true;

    switch (game)
    {
    case FALLOUT3:
    {
        FormLookup = (LookupForm) LOOKUP_FORM_FALLOUT3;
        FuncLookup = (LookupFunc) LOOKUP_FUNC_FALLOUT3;

        SafeWrite8(Fallout3patch_delegator_dest, 0x51); // PUSH ECX
        SafeWrite8(Fallout3patch_delegatorCall_src + 5, 0x59); // POP ECX
        SafeWrite8(Fallout3patch_PlayGroup, 0xEB); // JMP SHORT

        WriteRelCall(Fallout3patch_delegatorCall_src, Fallout3patch_delegatorCall_dest);
        WriteRelCall(Fallout3patch_delegator_src, Fallout3patch_delegator_dest);

        break;
    }
    case NEWVEGAS:
    {
        FormLookup = (LookupForm) LOOKUP_FORM_NEWVEGAS;
        FuncLookup = (LookupFunc) LOOKUP_FUNC_NEWVEGAS;

        SafeWrite8(FalloutNVpatch_delegator_dest, 0x51); // PUSH ECX
        SafeWrite8(FalloutNVpatch_delegatorCall_src + 5, 0x59); // POP ECX
        SafeWrite8(FalloutNVpatch_PlayGroup, 0xEB); // JMP SHORT

        WriteRelCall(FalloutNVpatch_delegatorCall_src, FalloutNVpatch_delegatorCall_dest);
        WriteRelCall(FalloutNVpatch_delegator_src, FalloutNVpatch_delegator_dest);

        break;
    }
    case OBLIVION:
    {
        FormLookup = (LookupForm) LOOKUP_FORM_OBLIVION;
        FuncLookup = (LookupFunc) LOOKUP_FUNC_OBLIVION;

        SafeWrite8(OblivionPatch_delegator_dest, 0x51); // PUSH ECX
        SafeWrite8(OblivionPatch_delegatorCall_src + 5, 0x59); // POP ECX

        WriteRelJump(OblivionPatch_delegator_ret_src, OblivionPatch_delegator_ret_dest);
        WriteRelCall(OblivionPatch_delegatorCall_src, OblivionPatch_delegatorCall_dest);
        WriteRelCall(OblivionPatch_delegator_src, OblivionPatch_delegator_dest);

        break;
    }
    }
}

void Initialize()
{
    CreateThread(NULL, 0, vaultmp_pipe, NULL, 0, NULL);
}

static void Startup(void) __attribute__((constructor));
void Startup()
{
    Initialize();
}
