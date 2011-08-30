#include <windows.h>
#include <stdio.h>
#include <vector>
#include <string>

#include "vaultmp.h"

using namespace std;

typedef void (*CallCommand)(void*, void*, void*, void*, void*, void*, void*, void*);
typedef unsigned int (*LookupForm)(unsigned int);
typedef unsigned int (*LookupFunc)(unsigned int);

typedef HANDLE (*fInitialize)(void);
typedef void (*fMessage)(string);

HANDLE hProc;
PipeServer pipeServer;
PipeClient pipeClient;
LookupForm FormLookup;
LookupFunc FuncLookup;

void PatchGame(HINSTANCE& silverlock);
void BethesdaDelegator();
vector<void*> delegated;

bool delegate = false;
bool DLLerror = false;
int game = 0;

const unsigned FalloutNVpatch_VATS_src = 0x009428AE;
const unsigned FalloutNVpatch_VATS_dest = 0x00942BE8;
const unsigned FalloutNVpatch_delegator_src = 0x0086B3E3;
const unsigned FalloutNVpatch_delegator_dest = 0x0086E649;
const unsigned FalloutNVpatch_delegatorCall_src = 0x0086E64A;
const unsigned FalloutNVpatch_delegatorCall_dest = (unsigned) &BethesdaDelegator;

const unsigned Fallout3patch_VATS_src = 0x0078A27D;
const unsigned Fallout3patch_VATS_dest = 0x0078A40A;
const unsigned Fallout3patch_delegator_src = 0x006EEC86;
const unsigned Fallout3patch_delegator_dest = 0x006EDBD9;
const unsigned Fallout3patch_delegatorCall_src = 0x006EDBDA;
const unsigned Fallout3patch_delegatorCall_dest = (unsigned) &BethesdaDelegator;

const unsigned OblivionPatch_delegator_src = 0x0040F270;
const unsigned OblivionPatch_delegator_dest = 0x0040F753;
const unsigned OblivionPatch_delegator_ret_src = 0x0040F75A;
const unsigned OblivionPatch_delegator_ret_dest = 0x0040D800;
const unsigned OblivionPatch_delegatorCall_src = 0x0040F754;
const unsigned OblivionPatch_delegatorCall_dest = (unsigned) &BethesdaDelegator;

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

void ExecuteCommand(const vector<void*> args, unsigned int crc, signed int key, bool delegate_flag)
{
    if (args.size() != 8)
        return;

    unsigned int reference = *((unsigned int*) args.at(2));
    unsigned int reference_old = reference;
    unsigned short opcode;

    if (*((unsigned int*) args.at(1)) == 0x0001001C)
        opcode = *((unsigned short*) (((unsigned) args.at(1)) + 4));
    else
        opcode = *((unsigned short*) args.at(1));

    if (opcode == 0x00)
        return;

    unsigned int function = FuncLookup((unsigned int) opcode);

    if (function == 0x00)
        return;

    void* callAddr = (void*) *((unsigned int*) (function + 0x18));

    if (callAddr == 0x00)
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

    char result[PIPE_LENGTH];
    ZeroMemory(result, sizeof(result));
    result[0] = PIPE_OP_RETURN;
    *((unsigned int*) ((unsigned) result + 1)) = crc;
    *((signed int*) ((unsigned) result + 5)) = key;

    memcpy(result + 9, args.at(6), sizeof(double));

    pipeClient.Send(result);
}

DWORD WINAPI vaultmp_pipe(LPVOID data)
{
    /* Loading and initalizing vaultgui.dll */

    HINSTANCE vaultgui = NULL;
    HINSTANCE silverlock = NULL;
    HANDLE guiThread;

    vaultgui = LoadLibrary("vaultgui.dll");

    if (vaultgui != NULL)
    {
        fInitialize init;
        init = (fInitialize) GetProcAddress(vaultgui, "Initialize");
        guiThread = init();
    }
    /*else
        DLLerror = true;*/

    fMessage GUI_msg;

    if (vaultgui != NULL)
    {
        GUI_msg = (fMessage) GetProcAddress(vaultgui, "Message");
    }

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
                signed int key = *((signed int*) content);
                content += 4;
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
                    ExecuteCommand(args, crc, key, delegate_flag);

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
                GUI_msg(string(content));
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
    TCHAR curdir[MAX_PATH]; ZeroMemory(curdir, sizeof(curdir));
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

        WriteRelJump(Fallout3patch_VATS_src, Fallout3patch_VATS_dest);
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

        WriteRelJump(FalloutNVpatch_VATS_src, FalloutNVpatch_VATS_dest);
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
