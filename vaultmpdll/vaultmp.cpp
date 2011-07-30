#include <windows.h>
#include <stdio.h>
#include <string>

#include "vaultmp.h"

using namespace std;
using namespace pipe;

HANDLE hProc;
PipeServer pipeServer;
PipeClient pipeClient;
char Fallout3output[MAX_OUTPUT_LENGTH];      // Command output storage
char Fallout3input[MAX_INPUT_LENGTH];        // Command input storage
bool Fallout3mutex = false;                  // Command queue mutex
bool Fallout3_output = false;
unsigned long long Fallout3_result = 0x00;
DWORD Fallout3_opcode = 0x00;
DWORD Fallout3_refID = 0x00;
DWORD Fallout3_newRefID = 0x00;
unsigned char Fallout3_coord = 0x00;
unsigned char Fallout3_setcoord = 0x00;
unsigned char Fallout3_valcoord = 0x00;
bool DLLerror = false;

void Fallout3commandNotify();
void Fallout3commandHandler();

/* Fallout: New Vegas version 1.4 */

static unsigned FalloutNVpatch_cmd1patchAddr = 0x0071B253;
static unsigned FalloutNVpatch_cmd2patchAddr = 0x0071B279;
static unsigned FalloutNVpatch_cmd3patchAddr = 0x0071B379;
static unsigned FalloutNVpatch_cmd4patchAddr = 0x0071B4C1;
static unsigned FalloutNVpatch_cmd5patchAddr = 0x0070E075;
static unsigned FalloutNVpatch_cmd6patchAddr = 0x0070E094;
static unsigned FalloutNVpatch_cmd7patchAddr = 0x00703C44;
static unsigned FalloutNVpatch_cmd8patchAddr = 0x0070CC7E;
static unsigned FalloutNVpatch_cmd9patchAddr = 0x0071B266;
static char FalloutNVpatch_cmd1[] = {0x90, 0x90};
static char FalloutNVpatch_cmd2[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
static char FalloutNVpatch_cmd3[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
static char FalloutNVpatch_cmd4[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
static char FalloutNVpatch_cmd5[] = {0xEB, 0x03, 0x90, 0x90, 0x90, 0x90};
static char FalloutNVpatch_cmd6[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
static char FalloutNVpatch_cmd7[] = {0xEB, 0x0C};
static char FalloutNVpatch_cmd8[] = {0x0F, 0x85, 0x15, 0x02, 0x00, 0x00};
static char FalloutNVpatch_cmd9[] = {0xEB, 0x07};

static unsigned FalloutNVpatch_VATSpatchAddr = 0x009428AE;
static char FalloutNVpatch_VATS[] = {0xE9, 0x35, 0x03, 0x00, 0x00, 0x90};

static LPVOID FalloutNVpatch_input1patchAddr = 0x00000000;
static unsigned FalloutNVpatch_input2patchAddr = 0x0071B4F9;
static char FalloutNVpatch_input1_1[] = {0x50, 0x51, 0x52, 0x8A, 0x10, 0x80, 0xFA, 0x00, 0x74, 0x0E, 0x5A, 0x59, 0x58, 0x8B, 0x8D, 0x58, 0xD7, 0xFF, 0xFF, 0xE9};
static unsigned FalloutNVpatch_input1_2 = 0x00000000;
static unsigned FalloutNVpatch_input1_2jmp = 0x0071B4FE;
static char FalloutNVpatch_input1_3[] = {0xB9};
static unsigned FalloutNVpatch_input1_4 = (unsigned) &Fallout3input;
static char FalloutNVpatch_input1_5[] = {0x8A, 0x11, 0x80, 0xFA, 0x00, 0x74, 0xE6, 0x88, 0x10, 0xC6, 0x01, 0x00, 0x83, 0xC1, 0x01, 0x83, 0xC0, 0x01, 0x8A, 0x11, 0x80, 0xFA, 0x00, 0x74, 0x02, 0xEB, 0x0EC, 0xC6, 0x05};
static unsigned FalloutNVpatch_input1_6 = (unsigned) &Fallout3mutex;
static char FalloutNVpatch_input1_7[] = {0x00, 0xC6, 0x00, 0x00, 0xEB, 0xC6};
static char FalloutNVpatch_input2_1[] = {0xE9};
static unsigned FalloutNVpatch_input2_2 = 0x00000000;
static char FalloutNVpatch_input2_3[] = {0x50, 0x90};
static unsigned FalloutNVpatch_input1size = sizeof(FalloutNVpatch_input1_1) + sizeof(FalloutNVpatch_input1_2) + sizeof(FalloutNVpatch_input1_3) + sizeof(FalloutNVpatch_input1_4) + sizeof(FalloutNVpatch_input1_5) +
                                            sizeof(FalloutNVpatch_input1_6) + sizeof(FalloutNVpatch_input1_7);

static LPVOID FalloutNVpatch_output1patchAddr = 0x00000000;
static unsigned FalloutNVpatch_output2patchAddr = 0x0071D116;
static char FalloutNVpatch_output1_1[] = {0x50, 0x51, 0x52, 0x8A, 0x0D};
static unsigned FalloutNVpatch_output1_2 = (unsigned) &Fallout3_output;
static char FalloutNVpatch_output1_3[] = {0x84, 0xC9, 0x74, 0x23, 0xB9};
static unsigned FalloutNVpatch_output1_4 = (unsigned) &Fallout3output;
static char FalloutNVpatch_output1_5[] = {0x83, 0xF8, 0x00, 0x74, 0x19, 0x8A, 0x10, 0x80, 0xFA, 0x00, 0x74, 0x0A, 0x88, 0x11, 0x83, 0xC0, 0x01, 0x83, 0xC1, 0x01, 0xEB, 0xEF, 0xC6, 0x01, 0x00, 0xE8};
static unsigned FalloutNVpatch_output1_6 = 0x00000000;
static unsigned FalloutNVpatch_output1_6cll = (unsigned) &Fallout3commandNotify;
static char FalloutNVpatch_output1_7[] = {0x5A, 0x59, 0x58, 0x8B, 0x8D, 0xB4, 0xF7, 0xFF, 0xFF, 0xE9};
static unsigned FalloutNVpatch_output1_8 = 0x00000000;
static unsigned FalloutNVpatch_output1_8jmp = 0x0071D11B;
static char FalloutNVpatch_output2_1[] = {0xE9};
static unsigned FalloutNVpatch_output2_2 = 0x00000000;
static char FalloutNVpatch_output2_3[] = {0x50, 0x90};
static unsigned FalloutNVpatch_output1size = sizeof(FalloutNVpatch_output1_1) + sizeof(FalloutNVpatch_output1_2) + sizeof(FalloutNVpatch_output1_3) + sizeof(FalloutNVpatch_output1_4) + sizeof(FalloutNVpatch_output1_5) +
                                             sizeof(FalloutNVpatch_output1_6) + sizeof(FalloutNVpatch_output1_7) + sizeof(FalloutNVpatch_output1_8);

static LPVOID FalloutNVpatch_handler1patchAddr = 0x00000000;
static unsigned FalloutNVpatch_handler2patchAddr = 0x005E2345;
static char FalloutNVpatch_handler1_1[] = {0x81, 0x7D, 0x0C, 0x14, 0x01, 0x00, 0x00, 0x74, 0x0B, 0x81, 0x7D, 0x0C, 0x9B, 0x01, 0x00, 0x00, 0x74, 0x02, 0xEB, 0x0A, 0x50, 0xB8};
static unsigned FalloutNVpatch_handler1_2 = (unsigned) &Fallout3_output;
static char FalloutNVpatch_handler1_3[] = {0xC6, 0x00, 0x01, 0x58, 0xFF, 0x95, 0x48, 0xF1, 0xFF, 0xFF,0x50, 0x56, 0xB8};
static unsigned FalloutNVpatch_handler1_4 = (unsigned) &Fallout3_output;
static char FalloutNVpatch_handler1_5[] = {0xC6, 0x00, 0x00, 0x83, 0xC4, 0x08, 0xBE};
static unsigned FalloutNVpatch_handler1_6 = (unsigned) &Fallout3_result;
static char FalloutNVpatch_handler1_7[] = {0x8B, 0x44, 0x24, 0x18, 0x8B, 0x00, 0x89, 0x06, 0x83, 0xEC, 0x08, 0x56, 0xBE};
static unsigned FalloutNVpatch_handler1_8 = (unsigned) &Fallout3_newRefID;
static char FalloutNVpatch_handler1_9[] = {0x89, 0x06, 0x5E, 0x83, 0xC4, 0x08, 0x8B, 0x44, 0x24, 0x18, 0x8B, 0x40, 0x04, 0x89, 0x46, 0x04, 0xBE};
static unsigned FalloutNVpatch_handler1_10 = (unsigned) &Fallout3_opcode;
static char FalloutNVpatch_handler1_11[] = {0x8B, 0x45, 0x0C, 0x89, 0x06, 0xBE};
static unsigned FalloutNVpatch_handler1_12 = (unsigned) &Fallout3_refID;
static char FalloutNVpatch_handler1_13[] = {0x8B, 0x45, 0x10, 0x85, 0xC0, 0x74, 0x05, 0x8B, 0x40, 0x0C, 0x89, 0x06, 0xBE};
static unsigned FalloutNVpatch_handler1_14 = (unsigned) &Fallout3_coord;
static char FalloutNVpatch_handler1_15[] = {0x8A, 0x44, 0x24, 0xE8, 0x88, 0x06, 0xBE};
static unsigned FalloutNVpatch_handler1_16 = (unsigned) &Fallout3_setcoord;
static char FalloutNVpatch_handler1_17[] = {0x8A, 0x44, 0x24, 0xE4, 0x88, 0x06, 0xBE};
static unsigned FalloutNVpatch_handler1_18 = (unsigned) &Fallout3_valcoord;
static char FalloutNVpatch_handler1_19[] = {0x8A, 0x44, 0x24, 0xF0, 0x88, 0x06, 0x83, 0xEC, 0x08, 0x5E, 0x58, 0xE8};
static unsigned FalloutNVpatch_handler1_20 = 0x00000000;
static unsigned FalloutNVpatch_handler1_20cll = (unsigned) &Fallout3commandHandler;
static char FalloutNVpatch_handler1_21[] = {0xE9};
static unsigned FalloutNVpatch_handler1_22 = 0x00000000;
static unsigned FalloutNVpatch_handler1_22jmp = 0x005E234B;
static char FalloutNVpatch_handler2_1[] = {0xE9};
static unsigned FalloutNVpatch_handler2_2 = 0x00000000;
static char FalloutNVpatch_handler2_3[] = {0x90};
static unsigned FalloutNVpatch_handler1size = sizeof(FalloutNVpatch_handler1_1) + sizeof(FalloutNVpatch_handler1_2) + sizeof(FalloutNVpatch_handler1_3) + sizeof(FalloutNVpatch_handler1_4) + sizeof(FalloutNVpatch_handler1_5) +
                                             sizeof(FalloutNVpatch_handler1_6) + sizeof(FalloutNVpatch_handler1_7) + sizeof(FalloutNVpatch_handler1_8) + sizeof(FalloutNVpatch_handler1_9) + sizeof(FalloutNVpatch_handler1_10) +
                                             sizeof(FalloutNVpatch_handler1_11) + sizeof(FalloutNVpatch_handler1_12) + sizeof(FalloutNVpatch_handler1_13) + sizeof(FalloutNVpatch_handler1_14) + sizeof(FalloutNVpatch_handler1_15) +
                                             sizeof(FalloutNVpatch_handler1_16) + sizeof(FalloutNVpatch_handler1_17) + sizeof(FalloutNVpatch_handler1_18) + sizeof(FalloutNVpatch_handler1_19) + sizeof(FalloutNVpatch_handler1_20) +
                                             sizeof(FalloutNVpatch_handler1_21) + sizeof(FalloutNVpatch_handler1_22);

/* Fallout 3 version 1.7 */

static unsigned Fallout3patch_cmd1patchAddr = 0x0062B663;
static unsigned Fallout3patch_cmd2patchAddr = 0x0062B66B;
static unsigned Fallout3patch_cmd3patchAddr = 0x00627A84;
static unsigned Fallout3patch_cmd4patchAddr = 0x0062B67E;
static unsigned Fallout3patch_cmd5patchAddr = 0x00627AFF;
static unsigned Fallout3patch_cmd6patchAddr = 0x0062B69B;
static unsigned Fallout3patch_cmd7patchAddr = 0x0062B744;
static unsigned Fallout3patch_cmd8patchAddr = 0x006195E8;
static unsigned Fallout3patch_cmd9patchAddr = 0x006288C4;
static char Fallout3patch_cmd1[] = {0x83, 0xFB, 0x0A, 0x90};
static char Fallout3patch_cmd2[] = {0x0F, 0x85, 0xBE, 0x15, 0x00, 0x00};
static char Fallout3patch_cmd3[] = {0xEB, 0x6E};
static char Fallout3patch_cmd4[] = {0xEB, 0x1B};
static char Fallout3patch_cmd5[] = {0xE9, 0x99, 0x0D, 0x00, 0x00, 0x90};
static char Fallout3patch_cmd6[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
static char Fallout3patch_cmd7[] = {0x83, 0xFB, 0x0A, 0x90, 0x90, 0x90};
static char Fallout3patch_cmd8[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
static char Fallout3patch_cmd9[] = {0xBB, 0x0A, 0x00, 0x00, 0x00, 0x90};

static unsigned Fallout3patch_VATSpatchAddr = 0x0078A27D;
static char Fallout3patch_VATS[] = {0xE9, 0x88, 0x01, 0x00, 0x00, 0x90};

static LPVOID Fallout3patch_input1patchAddr = 0x00000000;
static unsigned Fallout3patch_input2patchAddr = 0x00C075AF;
static char Fallout3patch_input1_1[] = {0x50, 0x51, 0x56, 0x8A, 0x06, 0x3C, 0x00, 0x74, 0x0D, 0x5E, 0x59, 0x58, 0xBA, 0xFF, 0xFE, 0xFE, 0x7E, 0xE9};
static unsigned Fallout3patch_input1_2 = 0x00000000;
static unsigned Fallout3patch_input1_2jmp = 0x00C075B4;
static char Fallout3patch_input1_3[] = {0xB9};
static unsigned Fallout3patch_input1_4 = (unsigned) &Fallout3input;
static char Fallout3patch_input1_5[] = {0x8A, 0x01, 0x3C, 0x00, 0x74, 0xE8, 0x88, 0x06, 0xC6, 0x01, 0x00, 0x83, 0xC1, 0x01, 0x83, 0xC6, 0x01, 0x8A, 0x01, 0x3C, 0x00, 0x74, 0x02, 0xEB, 0xED, 0xC6, 0x05};
static unsigned Fallout3patch_input1_6 = (unsigned) &Fallout3mutex;
static char Fallout3patch_input1_7[] = {0x00, 0xC6, 0x06, 0x00, 0xEB, 0xC9};
static char Fallout3patch_input2_1[] = {0xE9};
static unsigned Fallout3patch_input2_2 = 0x00000000;
static unsigned Fallout3patch_input1size = sizeof(Fallout3patch_input1_1) + sizeof(Fallout3patch_input1_2) + sizeof(Fallout3patch_input1_3) + sizeof(Fallout3patch_input1_4) + sizeof(Fallout3patch_input1_5) +
                                           sizeof(Fallout3patch_input1_6) + sizeof(Fallout3patch_input1_7);

static LPVOID Fallout3patch_output1patchAddr = 0x00000000;
static unsigned Fallout3patch_output2patchAddr = 0x0062B22B;
static char Fallout3patch_output1_1[] = {0x50, 0x51, 0x52, 0x8A, 0x0D};
static unsigned Fallout3patch_output1_2 = (unsigned) &Fallout3_output;
static char Fallout3patch_output1_3[] = {0x84, 0xC9, 0x74, 0x1E, 0xB9};
static unsigned Fallout3patch_output1_4 = (unsigned) &Fallout3output;
static char Fallout3patch_output1_5[] = {0x8A, 0x10, 0x80, 0xFA, 0x00, 0x74, 0x0A, 0x88, 0x11, 0x83, 0xC0, 0x01, 0x83, 0xC1, 0x01, 0xEB, 0xEF, 0xC6, 0x01, 0x00, 0xE8};
static unsigned Fallout3patch_output1_6 = 0x00000000;
static unsigned Fallout3patch_output1_6cll = (unsigned) &Fallout3commandNotify;
static char Fallout3patch_output1_7[] = {0x5A, 0x59, 0x58, 0xE8};
static unsigned Fallout3patch_output1_8 = 0x00000000;
static unsigned Fallout3patch_output1_8cll = 0x0062A800;
static char Fallout3patch_output1_9[] = {0xE9};
static unsigned Fallout3patch_output1_10 = 0x00000000;
static unsigned Fallout3patch_output1_10jmp = 0x0062B230;
static char Fallout3patch_output2_1[] = {0xE9};
static unsigned Fallout3patch_output2_2 = 0x00000000;
static unsigned Fallout3patch_output1size = sizeof(Fallout3patch_output1_1) + sizeof(Fallout3patch_output1_2) + sizeof(Fallout3patch_output1_3) + sizeof(Fallout3patch_output1_4) + sizeof(Fallout3patch_output1_5) +
                                            sizeof(Fallout3patch_output1_6) + sizeof(Fallout3patch_output1_7) + sizeof(Fallout3patch_output1_8) + sizeof(Fallout3patch_output1_9) + sizeof(Fallout3patch_output1_10);

static LPVOID Fallout3patch_handler1patchAddr = 0x00000000;
static unsigned Fallout3patch_handler2patchAddr = 0x00540BDE;
static char Fallout3patch_handler1_1[] = {0x81, 0x7D, 0x0C, 0x14, 0x01, 0x00, 0x00, 0x74, 0x0B, 0x81, 0x7D, 0x0C, 0x9C, 0x01, 0x00, 0x00, 0x74, 0x02, 0xEB, 0x0A, 0x50, 0xB8};
static unsigned Fallout3patch_handler1_2 = (unsigned) &Fallout3_output;
static char Fallout3patch_handler1_3[] = {0xC6, 0x00, 0x01, 0x58, 0x52, 0x57, 0x51, 0xFF, 0xD0, 0x50, 0x56, 0xB8};
static unsigned Fallout3patch_handler1_4 = (unsigned) &Fallout3_output;
static char Fallout3patch_handler1_5[] = {0xC6, 0x00, 0x00, 0x83, 0xC4, 0x08, 0xBE};
static unsigned Fallout3patch_handler1_6 = (unsigned) &Fallout3_result;
static char Fallout3patch_handler1_7[] = {0x8B, 0x44, 0x24, 0x18, 0x85, 0xC0, 0x74, 0x0E, 0x8B, 0x00, 0x89, 0x06, 0x8B, 0x44, 0x24, 0x18, 0x8B, 0x40, 0x04, 0x89, 0x46, 0x04, 0xBE};
static unsigned Fallout3patch_handler1_8 = (unsigned) &Fallout3_opcode;
static char Fallout3patch_handler1_9[] = {0x8B, 0x45, 0x0C, 0x89, 0x06, 0xBE};
static unsigned Fallout3patch_handler1_10 = (unsigned) &Fallout3_refID;
static char Fallout3patch_handler1_11[] = {0x8B, 0x45, 0x10, 0x85, 0xC0, 0x74, 0x05, 0x8B, 0x40, 0x0C, 0x89, 0x06, 0xBE};
static unsigned Fallout3patch_handler1_12 = (unsigned) &Fallout3_newRefID;
static char Fallout3patch_handler1_13[] = {0x89, 0x0E, 0xBE};
static unsigned Fallout3patch_handler1_14 = (unsigned) &Fallout3_coord;
static char Fallout3patch_handler1_15[] = {0x8A, 0x44, 0x24, 0xE8, 0x88, 0x06, 0xBE};
static unsigned Fallout3patch_handler1_16 = (unsigned) &Fallout3_setcoord;
static char Fallout3patch_handler1_17[] = {0x8A, 0x44, 0x24, 0xE0, 0x88, 0x06, 0xBE};
static unsigned Fallout3patch_handler1_18 = (unsigned) &Fallout3_valcoord;
static char Fallout3patch_handler1_19[] = {0x8A, 0x44, 0x24, 0xEC, 0x88, 0x06, 0x83, 0xEC, 0x08, 0x5E, 0x58, 0xE8};
static unsigned Fallout3patch_handler1_20 = 0x00000000;
static unsigned Fallout3patch_handler1_20cll = (unsigned) &Fallout3commandHandler;
static char Fallout3patch_handler1_21[] = {0xE9};
static unsigned Fallout3patch_handler1_22 = 0x00000000;
static unsigned Fallout3patch_handler1_22jmp = 0x00540BE3;
static char Fallout3patch_handler2_1[] = {0xE9};
static unsigned Fallout3patch_handler2_2 = 0x00000000;
static unsigned Fallout3patch_handler1size = sizeof(Fallout3patch_handler1_1) + sizeof(Fallout3patch_handler1_2) + sizeof(Fallout3patch_handler1_3) + sizeof(Fallout3patch_handler1_4) + sizeof(Fallout3patch_handler1_5) +
                                             sizeof(Fallout3patch_handler1_6) + sizeof(Fallout3patch_handler1_7) + sizeof(Fallout3patch_handler1_8) + sizeof(Fallout3patch_handler1_9) + sizeof(Fallout3patch_handler1_10) +
                                             sizeof(Fallout3patch_handler1_11) + sizeof(Fallout3patch_handler1_12) + sizeof(Fallout3patch_handler1_13) + sizeof(Fallout3patch_handler1_14) + sizeof(Fallout3patch_handler1_15) +
                                             sizeof(Fallout3patch_handler1_16) + sizeof(Fallout3patch_handler1_17) + sizeof(Fallout3patch_handler1_18) + sizeof(Fallout3patch_handler1_19) + sizeof(Fallout3patch_handler1_20) +
                                             sizeof(Fallout3patch_handler1_21) + sizeof(Fallout3patch_handler1_22);

typedef HANDLE (*fDLLjump)(void);
typedef void (*fDLLend)(void);
typedef void (*fMessage)(string);

void seDebugPrivilege()
{
	TOKEN_PRIVILEGES priv;
	HANDLE hThis, hToken;
	LUID luid;
	hThis = GetCurrentProcess();
	OpenProcessToken(hThis, TOKEN_ADJUST_PRIVILEGES, &hToken);
	LookupPrivilegeValue(0, "seDebugPrivilege", &luid);
	priv.PrivilegeCount = 1;
	priv.Privileges[0].Luid = luid;
	priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, false, &priv, 0, 0, 0);
	CloseHandle(hToken);
	CloseHandle(hThis);
}

unsigned offset(unsigned src, unsigned dest, unsigned jmpbytes) {
        unsigned way;
        src += jmpbytes;
        way = dest - src;
        return way;
     }

void Fallout3sendOp(string op) {

     while (Fallout3mutex) Sleep(2);

     Fallout3mutex = true;

     strcpy(Fallout3input, op.c_str());

}

void Fallout3commandNotify() {

    char format[MAX_OUTPUT_LENGTH + 3];
    ZeroMemory(format, sizeof(format));
    snprintf(format, sizeof(format), "st:%s", Fallout3output);
    string output(format);
    pipeClient.Send(&output);
}

void Fallout3commandHandler() {

    char format[MAX_OUTPUT_LENGTH + 3];
    ZeroMemory(format, sizeof(format));
    snprintf(format, sizeof(format), "op:%x %x %llx %x %x %x %x", Fallout3_opcode, Fallout3_refID, Fallout3_result, Fallout3_coord, Fallout3_setcoord, Fallout3_valcoord, Fallout3_newRefID);
    string output(format);
    pipeClient.Send(&output);
}

DWORD WINAPI Fallout3pipe(LPVOID data) {

    HINSTANCE vaultgui = (HINSTANCE) data;

    fDLLend GUI_end;
    fMessage GUI_msg;

    if (vaultgui != NULL)
    {
        GUI_end = (fDLLend)  GetProcAddress(vaultgui, "DLLend");
        GUI_msg = (fMessage) GetProcAddress(vaultgui, "Message");
    }

    pipeClient.SetPipeAttributes("Fallout3client", 4096);
    while (!pipeClient.ConnectToServer());

    pipeServer.SetPipeAttributes("Fallout3server", 4096);
    pipeServer.CreateServer();
    pipeServer.ConnectToServer();

    string send;
    string recv;
    string low;
    string high;

    send = "up:";
    pipeClient.Send(&send);

    if (DLLerror)
    {
        send = "ca:";
        pipeClient.Send(&send);
    }

    while (!DLLerror)
    {
        recv.clear(); low.clear(); high.clear();
        recv = pipeServer.Recv();
        low = recv.substr(0, 3);
        high = recv.substr(3);

        if (low.compare("op:") == 0)
        {
            Fallout3sendOp(high);
        }
        else if (low.compare("ce:") == 0)
        {
            if (vaultgui != NULL)
            {
                GUI_msg(high);
            }
        }
        else if (low.compare("ca:") == 0)
        {
            DLLerror = true;
        }
    }

    if (vaultgui != NULL)
        GUI_end();

    return ((DWORD) data);
}

extern "C" void __declspec(dllexport) DLLjump(bool NewVegas)
{
    seDebugPrivilege();

    hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
    DWORD rw = 0;
    unsigned bytes = 0;
    unsigned tmp = 0;

    char bytestream[16];

    switch (NewVegas)
    {
        case true:
        {
            /* Patching key event and console */

            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_cmd1patchAddr, &FalloutNVpatch_cmd1, sizeof(FalloutNVpatch_cmd1), &rw);
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_cmd2patchAddr, &FalloutNVpatch_cmd2, sizeof(FalloutNVpatch_cmd2), &rw);
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_cmd3patchAddr, &FalloutNVpatch_cmd3, sizeof(FalloutNVpatch_cmd3), &rw);
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_cmd4patchAddr, &FalloutNVpatch_cmd4, sizeof(FalloutNVpatch_cmd4), &rw);
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_cmd5patchAddr, &FalloutNVpatch_cmd5, sizeof(FalloutNVpatch_cmd5), &rw);
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_cmd6patchAddr, &FalloutNVpatch_cmd6, sizeof(FalloutNVpatch_cmd6), &rw);
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_cmd7patchAddr, &FalloutNVpatch_cmd7, sizeof(FalloutNVpatch_cmd7), &rw);
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_cmd8patchAddr, &FalloutNVpatch_cmd8, sizeof(FalloutNVpatch_cmd8), &rw);
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_cmd9patchAddr, &FalloutNVpatch_cmd9, sizeof(FalloutNVpatch_cmd9), &rw);

            /* Disabling VATS system */

            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_VATSpatchAddr, &FalloutNVpatch_VATS, sizeof(FalloutNVpatch_VATS), &rw);

            /* Writing FalloutNV command INPUT detour */

            bytes = 0;

            LPVOID FalloutNVpatch_input1patchAddr = VirtualAllocEx(hProc, 0, FalloutNVpatch_input1size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_input1patchAddr, &FalloutNVpatch_input1_1, sizeof(FalloutNVpatch_input1_1), &rw); bytes += rw;
            FalloutNVpatch_input1_2 = offset((((unsigned) FalloutNVpatch_input1patchAddr) + bytes - 1), (unsigned) FalloutNVpatch_input1_2jmp, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_input1patchAddr) + bytes), &FalloutNVpatch_input1_2, sizeof(FalloutNVpatch_input1_2), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_input1patchAddr) + bytes), &FalloutNVpatch_input1_3, sizeof(FalloutNVpatch_input1_3), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_input1patchAddr) + bytes), &FalloutNVpatch_input1_4, sizeof(FalloutNVpatch_input1_4), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_input1patchAddr) + bytes), &FalloutNVpatch_input1_5, sizeof(FalloutNVpatch_input1_5), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_input1patchAddr) + bytes), &FalloutNVpatch_input1_6, sizeof(FalloutNVpatch_input1_6), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_input1patchAddr) + bytes), &FalloutNVpatch_input1_7, sizeof(FalloutNVpatch_input1_7), &rw); bytes += rw;

            bytes = 0;
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_input2patchAddr, &FalloutNVpatch_input2_1, sizeof(FalloutNVpatch_input2_1), &rw); bytes += rw;
            FalloutNVpatch_input2_2 = offset((((unsigned) FalloutNVpatch_input2patchAddr) + bytes - 1), (unsigned) FalloutNVpatch_input1patchAddr, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_input2patchAddr) + bytes), &FalloutNVpatch_input2_2, sizeof(FalloutNVpatch_input2_2), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_input2patchAddr) + bytes), &FalloutNVpatch_input2_3, sizeof(FalloutNVpatch_input2_3), &rw); bytes += rw;

            /* Writing FalloutNV command OUTPUT detour */

            bytes = 0;

            LPVOID FalloutNVpatch_output1patchAddr = VirtualAllocEx(hProc, 0, FalloutNVpatch_output1size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_output1patchAddr, &FalloutNVpatch_output1_1, sizeof(FalloutNVpatch_output1_1), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_output1patchAddr) + bytes), &FalloutNVpatch_output1_2, sizeof(FalloutNVpatch_output1_2), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_output1patchAddr) + bytes), &FalloutNVpatch_output1_3, sizeof(FalloutNVpatch_output1_3), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_output1patchAddr) + bytes), &FalloutNVpatch_output1_4, sizeof(FalloutNVpatch_output1_4), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_output1patchAddr) + bytes), &FalloutNVpatch_output1_5, sizeof(FalloutNVpatch_output1_5), &rw); bytes += rw;
            FalloutNVpatch_output1_6 = offset((((unsigned) FalloutNVpatch_output1patchAddr) + bytes - 1), (unsigned) FalloutNVpatch_output1_6cll, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_output1patchAddr) + bytes), &FalloutNVpatch_output1_6, sizeof(FalloutNVpatch_output1_6), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_output1patchAddr) + bytes), &FalloutNVpatch_output1_7, sizeof(FalloutNVpatch_output1_7), &rw); bytes += rw;
            FalloutNVpatch_output1_8 = offset((((unsigned) FalloutNVpatch_output1patchAddr) + bytes - 1), (unsigned) FalloutNVpatch_output1_8jmp, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_output1patchAddr) + bytes), &FalloutNVpatch_output1_8, sizeof(FalloutNVpatch_output1_8), &rw); bytes += rw;

            bytes = 0;
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_output2patchAddr, &FalloutNVpatch_output2_1, sizeof(FalloutNVpatch_output2_1), &rw); bytes += rw;
            FalloutNVpatch_output2_2 = offset((((unsigned) FalloutNVpatch_output2patchAddr) + bytes - 1), (unsigned) FalloutNVpatch_output1patchAddr, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_output2patchAddr) + bytes), &FalloutNVpatch_output2_2, sizeof(FalloutNVpatch_output2_2), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_output2patchAddr) + bytes), &FalloutNVpatch_output2_3, sizeof(FalloutNVpatch_output2_3), &rw); bytes += rw;

            /* Writing FalloutNV command handler */

            bytes = 0;

            LPVOID FalloutNVpatch_handler1patchAddr = VirtualAllocEx(hProc, 0, FalloutNVpatch_handler1size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_handler1patchAddr, &FalloutNVpatch_handler1_1, sizeof(FalloutNVpatch_handler1_1), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_2, sizeof(FalloutNVpatch_handler1_2), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_3, sizeof(FalloutNVpatch_handler1_3), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_4, sizeof(FalloutNVpatch_handler1_4), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_5, sizeof(FalloutNVpatch_handler1_5), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_6, sizeof(FalloutNVpatch_handler1_6), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_7, sizeof(FalloutNVpatch_handler1_7), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_8, sizeof(FalloutNVpatch_handler1_8), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_9, sizeof(FalloutNVpatch_handler1_9), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_10, sizeof(FalloutNVpatch_handler1_10), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_11, sizeof(FalloutNVpatch_handler1_11), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_12, sizeof(FalloutNVpatch_handler1_12), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_13, sizeof(FalloutNVpatch_handler1_13), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_14, sizeof(FalloutNVpatch_handler1_14), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_15, sizeof(FalloutNVpatch_handler1_15), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_16, sizeof(FalloutNVpatch_handler1_16), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_17, sizeof(FalloutNVpatch_handler1_17), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_18, sizeof(FalloutNVpatch_handler1_18), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_19, sizeof(FalloutNVpatch_handler1_19), &rw); bytes += rw;
            FalloutNVpatch_handler1_20 = offset((((unsigned) FalloutNVpatch_handler1patchAddr) + bytes - 1), (unsigned) FalloutNVpatch_handler1_20cll, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_20, sizeof(FalloutNVpatch_handler1_20), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_21, sizeof(FalloutNVpatch_handler1_21), &rw); bytes += rw;
            FalloutNVpatch_handler1_22 = offset((((unsigned) FalloutNVpatch_handler1patchAddr) + bytes - 1), (unsigned) FalloutNVpatch_handler1_22jmp, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler1patchAddr) + bytes), &FalloutNVpatch_handler1_22, sizeof(FalloutNVpatch_handler1_22), &rw); bytes += rw;

            bytes = 0;
            WriteProcessMemory(hProc, (LPVOID) FalloutNVpatch_handler2patchAddr, &FalloutNVpatch_handler2_1, sizeof(FalloutNVpatch_handler2_1), &rw); bytes += rw;
            FalloutNVpatch_handler2_2 = offset((((unsigned) FalloutNVpatch_handler2patchAddr) + bytes - 1), (unsigned) FalloutNVpatch_handler1patchAddr, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler2patchAddr) + bytes), &FalloutNVpatch_handler2_2, sizeof(FalloutNVpatch_handler2_2), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) FalloutNVpatch_handler2patchAddr) + bytes), &FalloutNVpatch_handler2_3, sizeof(FalloutNVpatch_handler2_3), &rw); bytes += rw;

            break;
        }
        case false:
        {
            /* Patching key event and console */

            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_cmd1patchAddr, &Fallout3patch_cmd1, sizeof(Fallout3patch_cmd1), &rw);
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_cmd2patchAddr, &Fallout3patch_cmd2, sizeof(Fallout3patch_cmd2), &rw);
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_cmd3patchAddr, &Fallout3patch_cmd3, sizeof(Fallout3patch_cmd3), &rw);
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_cmd4patchAddr, &Fallout3patch_cmd4, sizeof(Fallout3patch_cmd4), &rw);
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_cmd5patchAddr, &Fallout3patch_cmd5, sizeof(Fallout3patch_cmd5), &rw);
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_cmd6patchAddr, &Fallout3patch_cmd6, sizeof(Fallout3patch_cmd6), &rw);
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_cmd7patchAddr, &Fallout3patch_cmd7, sizeof(Fallout3patch_cmd7), &rw);
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_cmd8patchAddr, &Fallout3patch_cmd8, sizeof(Fallout3patch_cmd8), &rw);
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_cmd9patchAddr, &Fallout3patch_cmd9, sizeof(Fallout3patch_cmd9), &rw);

            /* Disabling VATS system */

            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_VATSpatchAddr, &Fallout3patch_VATS, sizeof(Fallout3patch_VATS), &rw);

            /* Writing Fallout3 command INPUT detour */

            bytes = 0;

            LPVOID Fallout3patch_input1patchAddr = VirtualAllocEx(hProc, 0, Fallout3patch_input1size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_input1patchAddr, &Fallout3patch_input1_1, sizeof(Fallout3patch_input1_1), &rw); bytes += rw;
            Fallout3patch_input1_2 = offset((((unsigned) Fallout3patch_input1patchAddr) + bytes - 1), (unsigned) Fallout3patch_input1_2jmp, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_input1patchAddr) + bytes), &Fallout3patch_input1_2, sizeof(Fallout3patch_input1_2), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_input1patchAddr) + bytes), &Fallout3patch_input1_3, sizeof(Fallout3patch_input1_3), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_input1patchAddr) + bytes), &Fallout3patch_input1_4, sizeof(Fallout3patch_input1_4), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_input1patchAddr) + bytes), &Fallout3patch_input1_5, sizeof(Fallout3patch_input1_5), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_input1patchAddr) + bytes), &Fallout3patch_input1_6, sizeof(Fallout3patch_input1_6), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_input1patchAddr) + bytes), &Fallout3patch_input1_7, sizeof(Fallout3patch_input1_7), &rw); bytes += rw;

            bytes = 0;
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_input2patchAddr, &Fallout3patch_input2_1, sizeof(Fallout3patch_input2_1), &rw); bytes += rw;
            Fallout3patch_input2_2 = offset((((unsigned) Fallout3patch_input2patchAddr) + bytes - 1), (unsigned) Fallout3patch_input1patchAddr, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_input2patchAddr) + bytes), &Fallout3patch_input2_2, sizeof(Fallout3patch_input2_2), &rw); bytes += rw;

            /* Writing Fallout3 command OUTPUT detour */

            bytes = 0;

            LPVOID Fallout3patch_output1patchAddr = VirtualAllocEx(hProc, 0, Fallout3patch_output1size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_output1patchAddr, &Fallout3patch_output1_1, sizeof(Fallout3patch_output1_1), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_output1patchAddr) + bytes), &Fallout3patch_output1_2, sizeof(Fallout3patch_output1_2), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_output1patchAddr) + bytes), &Fallout3patch_output1_3, sizeof(Fallout3patch_output1_3), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_output1patchAddr) + bytes), &Fallout3patch_output1_4, sizeof(Fallout3patch_output1_4), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_output1patchAddr) + bytes), &Fallout3patch_output1_5, sizeof(Fallout3patch_output1_5), &rw); bytes += rw;
            Fallout3patch_output1_6 = offset((((unsigned) Fallout3patch_output1patchAddr) + bytes - 1), (unsigned) Fallout3patch_output1_6cll, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_output1patchAddr) + bytes), &Fallout3patch_output1_6, sizeof(Fallout3patch_output1_6), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_output1patchAddr) + bytes), &Fallout3patch_output1_7, sizeof(Fallout3patch_output1_7), &rw); bytes += rw;
            Fallout3patch_output1_8 = offset((((unsigned) Fallout3patch_output1patchAddr) + bytes - 1), (unsigned) Fallout3patch_output1_8cll, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_output1patchAddr) + bytes), &Fallout3patch_output1_8, sizeof(Fallout3patch_output1_8), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_output1patchAddr) + bytes), &Fallout3patch_output1_9, sizeof(Fallout3patch_output1_9), &rw); bytes += rw;
            Fallout3patch_output1_10 = offset((((unsigned) Fallout3patch_output1patchAddr) + bytes - 1), (unsigned) Fallout3patch_output1_10jmp, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_output1patchAddr) + bytes), &Fallout3patch_output1_10, sizeof(Fallout3patch_output1_10), &rw); bytes += rw;

            bytes = 0;
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_output2patchAddr, &Fallout3patch_output2_1, sizeof(Fallout3patch_output2_1), &rw); bytes += rw;
            Fallout3patch_output2_2 = offset((((unsigned) Fallout3patch_output2patchAddr) + bytes - 1), (unsigned) Fallout3patch_output1patchAddr, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_output2patchAddr) + bytes), &Fallout3patch_output2_2, sizeof(Fallout3patch_output2_2), &rw); bytes += rw;

            /* Writing Fallout3 command handler */

            bytes = 0;

            LPVOID Fallout3patch_handler1patchAddr = VirtualAllocEx(hProc, 0, Fallout3patch_handler1size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_handler1patchAddr, &Fallout3patch_handler1_1, sizeof(Fallout3patch_handler1_1), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_2, sizeof(Fallout3patch_handler1_2), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_3, sizeof(Fallout3patch_handler1_3), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_4, sizeof(Fallout3patch_handler1_4), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_5, sizeof(Fallout3patch_handler1_5), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_6, sizeof(Fallout3patch_handler1_6), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_7, sizeof(Fallout3patch_handler1_7), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_8, sizeof(Fallout3patch_handler1_8), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_9, sizeof(Fallout3patch_handler1_9), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_10, sizeof(Fallout3patch_handler1_10), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_11, sizeof(Fallout3patch_handler1_11), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_12, sizeof(Fallout3patch_handler1_12), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_13, sizeof(Fallout3patch_handler1_13), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_14, sizeof(Fallout3patch_handler1_14), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_15, sizeof(Fallout3patch_handler1_15), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_16, sizeof(Fallout3patch_handler1_16), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_17, sizeof(Fallout3patch_handler1_17), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_18, sizeof(Fallout3patch_handler1_18), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_19, sizeof(Fallout3patch_handler1_19), &rw); bytes += rw;
            Fallout3patch_handler1_20 = offset((((unsigned) Fallout3patch_handler1patchAddr) + bytes - 1), (unsigned) Fallout3patch_handler1_20cll, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_20, sizeof(Fallout3patch_handler1_20), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_21, sizeof(Fallout3patch_handler1_21), &rw); bytes += rw;
            Fallout3patch_handler1_22 = offset((((unsigned) Fallout3patch_handler1patchAddr) + bytes - 1), (unsigned) Fallout3patch_handler1_22jmp, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler1patchAddr) + bytes), &Fallout3patch_handler1_22, sizeof(Fallout3patch_handler1_22), &rw); bytes += rw;

            bytes = 0;
            WriteProcessMemory(hProc, (LPVOID) Fallout3patch_handler2patchAddr, &Fallout3patch_handler2_1, sizeof(Fallout3patch_handler2_1), &rw); bytes += rw;
            Fallout3patch_handler2_2 = offset((((unsigned) Fallout3patch_handler2patchAddr) + bytes - 1), (unsigned) Fallout3patch_handler1patchAddr, 5);
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3patch_handler2patchAddr) + bytes), &Fallout3patch_handler2_2, sizeof(Fallout3patch_handler2_2), &rw); bytes += rw;

            break;
        }
    }

    CloseHandle(hProc);

    /* Loading FOSE / NVSE */

    HINSTANCE fose = NULL;

    if (NewVegas)
        fose = LoadLibrary("nvse_1_1.dll");
    else
        fose = LoadLibrary("fose_1_7.dll");

    if (fose == NULL)
        DLLerror = true;

    /* Loading and initalizing vaultgui.dll */

    HINSTANCE vaultgui = NULL;
    HANDLE guiThread;

    vaultgui = LoadLibrary("vaultgui.dll");

    if (vaultgui != NULL)
    {
        fDLLjump init;
        init = (fDLLjump) GetProcAddress(vaultgui, "DLLjump");
        guiThread = init();
    }
    /*else
        DLLerror = true;*/

    /* Initalizing vaultmp.exe <-> Fallout3.exe pipe */

    HANDLE PipeThread;
    DWORD Fallout3pipeID;

    PipeThread = CreateThread(NULL, 0, Fallout3pipe, (LPVOID) vaultgui, 0, &Fallout3pipeID);

    if (guiThread != NULL)
    {
        HANDLE threads[2];
        threads[0] = PipeThread;
        threads[1] = guiThread;
        WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    }
    else
        WaitForSingleObject(PipeThread, INFINITE);

    if (vaultgui != NULL)
        FreeLibrary(vaultgui);

    if (guiThread != NULL)
        CloseHandle(guiThread);

    CloseHandle(PipeThread);
}

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
   return TRUE;
}
