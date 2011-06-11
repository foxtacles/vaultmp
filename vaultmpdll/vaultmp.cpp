#include <windows.h>
#include <stdio.h>
#include <string>

#include "vaultmp.h"

using namespace std;
using namespace pipe;

HANDLE hProc;
PipeServer pipeServer;
PipeClient pipeClient;
unsigned Fallout3opTrigger = 0x00;           // 0x00 = trigger disabled. 0x0A = trigger enabled.
unsigned Fallout3refid = 0x00;               // RefID storage
char Fallout3output[MAX_OUTPUT_LENGTH];      // Command output storage
char Fallout3input[MAX_INPUT_LENGTH];        // Command input storage
bool Fallout3mutex = false;                  // Command queue mutex

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

void Fallout3toggleTrigger(bool toggle) {
    Fallout3opTrigger = toggle ? 0x0A : 0x00;
}

void Fallout3sendOp(string op) {

     while (Fallout3mutex) Sleep(2);

     Fallout3mutex = true;

     strcpy(Fallout3input, op.c_str());
}

void Fallout3commandNotify() {

    char format[MAX_OUTPUT_LENGTH + 3];
    strcat(format, "op:");
    strcat(format, Fallout3output);
    string output(format);
    pipeClient.Send(&output);
}

void Fallout3refidNotify() {

    char format[16]; char refid[16];
    sprintf(refid, "%x", Fallout3refid);
    strcat(format, "re:");
    strcat(format, refid);
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

    Fallout3toggleTrigger(true);

    do
    {
        recv.clear(); low.clear(); high.clear();
        recv = pipeServer.Recv();
        low = recv.substr(0, 3);
        high = recv.substr(3);

        if (low.compare("op:") == 0)
            Fallout3sendOp(high);
        else if (low.compare("ce:") == 0)
        {
            if (vaultgui != NULL)
                GUI_msg(high);
        }
    } while (low.compare("ca:") != 0);

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

            /* 0071A823   90               NOP
             * 0071A824   90               NOP

             * 0071A849   90               NOP
             * 0071A84A   90               NOP
             * 0071A84B   90               NOP
             * 0071A84C   90               NOP
             * 0071A84D   90               NOP
             * 0071A84E   90               NOP

             * 0071A949   90               NOP
             * 0071A94A   90               NOP
             * 0071A94B   90               NOP
             * 0071A94C   90               NOP
             * 0071A94D   90               NOP
             * 0071A94E   90               NOP

             * 0071AA91   90               NOP
             * 0071AA92   90               NOP
             * 0071AA93   90               NOP
             * 0071AA94   90               NOP
             * 0071AA95   90               NOP
             * 0071AA96   90               NOP
             */

            bytestream[0] = 0x90; bytestream[1] = 0x90;
            for (int i = 0; i < 2; i++) WriteProcessMemory(hProc, (LPVOID) (0x0071A823 + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0x90; bytestream[1] = 0x90; bytestream[2] = 0x90; bytestream[3] = 0x90; bytestream[4] = 0x90; bytestream[5] = 0x90;
            for (int i = 0; i < 6; i++) WriteProcessMemory(hProc, (LPVOID) (0x0071A849 + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0x90; bytestream[1] = 0x90; bytestream[2] = 0x90; bytestream[3] = 0x90; bytestream[4] = 0x90; bytestream[5] = 0x90;
            for (int i = 0; i < 6; i++) WriteProcessMemory(hProc, (LPVOID) (0x0071A949 + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0x83; bytestream[1] = 0xFB; bytestream[2] = 0x0A; bytestream[3] = 0x90; bytestream[4] = 0x90; bytestream[5] = 0x90;
            for (int i = 0; i < 6; i++) WriteProcessMemory(hProc, (LPVOID) (0x0071AA91 + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            /* Disabling VATS system */

            /* 0093FFAE   E9 5A020000      JMP FalloutN.0094020D
             * 0093FFB3   90               NOP
             */

            bytes = 0;
            rw = 0;

            bytestream[0] = 0xE9; bytestream[1] = 0x5A; bytestream[2] = 0x02; bytestream[3] = 0x00; bytestream[4] = 0x00; bytestream[5] = 0x90;
            for (int i = 0; i < 6; i++) WriteProcessMemory(hProc, (LPVOID) (0x0093FFAE + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            /* Writing FalloutNV command trigger TOTAL BYTES TO RESERVE: 19 */

            /* XXXXXXXX   A1 XXXXXXXX      MOV EAX,[XXXXXXXX]
             * XXXXXXXX   83F8 0A          CMP EAX,A
             * XXXXXXXX  -0F84 XXXXXXXX    JE FalloutN.0070D6EA
             * XXXXXXXX  -E9 XXXXXXXX      JMP FalloutN.0070D7AF
             *
             * 0070D6E5  -E9 XXXXXXXX      JMP XXXXXXXX
             * 0070D6EA   90               NOP
             *
             * 0070D704   90               NOP
             * 0070D705   90               NOP
             * 0070D706   90               NOP
             * 0070D707   90               NOP
             * 0070D708   90               NOP
             * 0070D709   90               NOP
             *
             * 00703224   EB 0C            JMP SHORT FalloutN.00703232
             */

            bytes = 0;
            rw = 0;

            LPVOID Fallout3triggerASM = VirtualAllocEx(hProc, 0, 19, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            tmp = (unsigned) &Fallout3opTrigger;
            bytestream[0] = 0xA1; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x83; bytestream[1] = 0xF8; bytestream[2] = 0x0A;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((((unsigned) Fallout3triggerASM) + bytes), (unsigned) 0x0070D6EA, 6);
            bytestream[0] = 0x0F; bytestream[1] = 0x84;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((((unsigned) Fallout3triggerASM) + bytes), (unsigned) 0x0070D7AF, 5);
            bytestream[0] = 0xE9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((unsigned) 0x0070D6E5, (unsigned) Fallout3triggerASM, 5);
            bytestream[0] = 0xE9; bytes = 0; WriteProcessMemory(hProc, (LPVOID) (0x0070D6E5 + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (0x0070D6E5 + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;
            bytestream[0] = 0x90; WriteProcessMemory(hProc, (LPVOID) (0x0070D6E5 + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;

            bytestream[0] = 0x90; bytestream[1] = 0x90; bytestream[2] = 0x90; bytestream[3] = 0x90; bytestream[4] = 0x90; bytestream[5] = 0x90;
            for (int i = 0; i < 6; i++) WriteProcessMemory(hProc, (LPVOID) (0x0070D704 + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0xEB; bytestream[1] = 0x0C;
            for (int i = 0; i < 2; i++) WriteProcessMemory(hProc, (LPVOID) (0x00703224 + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            /* Writing FalloutNV command INPUT detour TOTAL BYTES TO RESERVE: 68 */

            /* XXXXXXXX   50               PUSH EAX
             * XXXXXXXX   51               PUSH ECX
             * XXXXXXXX   52               PUSH EDX
             * XXXXXXXX   8A10             MOV DL,BYTE PTR DS:[EAX]
             * XXXXXXXX   80FA 00          CMP DL,0
             * XXXXXXXX   74 0E            JE SHORT XXXXXXXX
             * XXXXXXXX   5A               POP EDX
             * XXXXXXXX   59               POP ECX
             * XXXXXXXX   58               POP EAX
             * XXXXXXXX   8B8D 58D7FFFF    MOV ECX,DWORD PTR SS:[EBP-28A8]
             * XXXXXXXX  -E9 XXXXXXXX      JMP FalloutN.0071AACE
             * XXXXXXXX   B9 XXXXXXXX      MOV ECX,XXXXXXXX
             * XXXXXXXX   8A11             MOV DL,BYTE PTR DS:[ECX]
             * XXXXXXXX   80FA 00          CMP DL,0
             * XXXXXXXX  ^74 E6            JE SHORT XXXXXXXX
             * XXXXXXXX   8810             MOV BYTE PTR DS:[EAX],DL
             * XXXXXXXX   C601 00          MOV BYTE PTR DS:[ECX],0
             * XXXXXXXX   83C1 01          ADD ECX,1
             * XXXXXXXX   83C0 01          ADD EAX,1
             * XXXXXXXX   8A11             MOV DL,BYTE PTR DS:[ECX]
             * XXXXXXXX   80FA 00          CMP DL,0
             * XXXXXXXX   74 02            JE SHORT XXXXXXXX
             * XXXXXXXX  ^EB EC            JMP SHORT XXXXXXXX
             * XXXXXXXX   C605 XXXXXXXX 00 MOV BYTE PTR DS:[XXXXXXXX],0
             * XXXXXXXX   C600 00          MOV BYTE PTR DS:[EAX],0
             * XXXXXXXX  ^EB C6            JMP SHORT XXXXXXXX
             *
             * 0071AAC9  -E9 XXXXXXXX      JMP XXXXXXXX
             * 0071AACE   50               PUSH EAX
             * 0071AACF   90               NOP
             */

            bytes = 0;
            rw = 0;

            LPVOID Fallout3inputASM = VirtualAllocEx(hProc, 0, 68, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            bytestream[0] = 0x50; bytestream[1] = 0x51; bytestream[2] = 0x52;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x8A; bytestream[1] = 0x10;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x80; bytestream[1] = 0xFA; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x74; bytestream[1] = 0x0E;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x5A; bytestream[1] = 0x59; bytestream[2] = 0x58;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x8B; bytestream[1] = 0x8D; bytestream[2] = 0x58; bytestream[3] = 0xD7; bytestream[4] = 0xFF; bytestream[5] = 0xFF;
            for (int i = 0; i < 6; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((((unsigned) Fallout3inputASM) + bytes), (unsigned) 0x0071AACE, 5);
            bytestream[0] = 0xE9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = (unsigned) &Fallout3input;
            bytestream[0] = 0xB9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x8A; bytestream[1] = 0x11;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x80; bytestream[1] = 0xFA; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x74; bytestream[1] = 0xE6;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x88; bytestream[1] = 0x10;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xC6; bytestream[1] = 0x01; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x83; bytestream[1] = 0xC1; bytestream[2] = 0x01;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x83; bytestream[1] = 0xC0; bytestream[2] = 0x01;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x8A; bytestream[1] = 0x11;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x80; bytestream[1] = 0xFA; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x74; bytestream[1] = 0x02;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xEB; bytestream[1] = 0xEC;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = (unsigned) &Fallout3mutex;
            bytestream[0] = 0xC6; bytestream[1] = 0x05;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;
            bytestream[0] = 0x00; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;

            bytestream[0] = 0xC6; bytestream[1] = 0x00; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xEB; bytestream[1] = 0xC6;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((unsigned) 0x0071AAC9, ((unsigned) Fallout3inputASM), 5);
            bytestream[0] = 0xE9; bytes = 0; WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x0071AAC9 + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x0071AAC9 + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x50; bytestream[1] = 0x90;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x0071AAC9 + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            /* Writing FalloutNV command OUTPUT detour TOTAL BYTES TO RESERVE: 52 */

            /* XXXXXXXX   50               PUSH EAX
             * XXXXXXXX   51               PUSH ECX
             * XXXXXXXX   52               PUSH EDX
             * XXXXXXXX   B9 XXXXXXXX      MOV ECX,XXXXXXXX
             * XXXXXXXX   83F8 00          CMP EAX,0
             * XXXXXXXX   74 19            JE SHORT XXXXXXXX
             * XXXXXXXX   8A10             MOV DL,BYTE PTR DS:[EAX]
             * XXXXXXXX   80FA 00          CMP DL,0
             * XXXXXXXX   74 0A            JE SHORT XXXXXXXX
             * XXXXXXXX   8811             MOV BYTE PTR DS:[ECX],DL
             * XXXXXXXX   83C0 01          ADD EAX,1
             * XXXXXXXX   83C1 01          ADD ECX,1
             * XXXXXXXX  ^EB EF            JMP SHORT XXXXXXXX
             * XXXXXXXX   C601 00          MOV BYTE PTR DS:[ECX],0
             * XXXXXXXX   E8 XXXXXXXX      CALL vaultmp.XXXXXXXX
             * XXXXXXXX   5A               POP EDX
             * XXXXXXXX   59               POP ECX
             * XXXXXXXX   58               POP EAX
             * XXXXXXXX   8B8D B4F7FFFF    MOV ECX,DWORD PTR SS:[EBP-84C]
             * XXXXXXXX  -E9 XXXXXXXX      JMP Fallout3.0071C6EB
             *
             * 0071C6E6  -E9 XXXXXXXX      JMP XXXXXXXX
             * 0071C6EB   50               PUSH EAX
             * 0071C6EC   90               NOP
             */

            bytes = 0;
            rw = 0;

            LPVOID Fallout3outputASM = VirtualAllocEx(hProc, 0, 52, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            bytestream[0] = 0x50; bytestream[1] = 0x51; bytestream[2] = 0x52;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = (unsigned) &Fallout3output;
            bytestream[0] = 0xB9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x83; bytestream[1] = 0xF8; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x74; bytestream[1] = 0x19;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x8A; bytestream[1] = 0x10;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x80; bytestream[1] = 0xFA; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x74; bytestream[1] = 0x0A;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x88; bytestream[1] = 0x11;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x83; bytestream[1] = 0xC0; bytestream[2] = 0x01;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x83; bytestream[1] = 0xC1; bytestream[2] = 0x01;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xEB; bytestream[1] = 0xEF;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xC6; bytestream[1] = 0x01; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((((unsigned) Fallout3outputASM) + bytes), ((unsigned) &Fallout3commandNotify), 5);
            bytestream[0] = 0xE8; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x5A; bytestream[1] = 0x59; bytestream[2] = 0x58;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x8B; bytestream[1] = 0x8D; bytestream[2] = 0xB4; bytestream[3] = 0xF7; bytestream[4] = 0xFF; bytestream[5] = 0xFF;
            for (int i = 0; i < 6; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((((unsigned) Fallout3outputASM) + bytes), (unsigned) 0x0071C6EB, 5);
            bytestream[0] = 0xE9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((unsigned) 0x0071C6E6, ((unsigned) Fallout3outputASM), 5);
            bytestream[0] = 0xE9; bytes = 0; WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x0071C6E6 + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x0071C6E6 + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x50; bytestream[1] = 0x90;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x0071C6E6 + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            /* Writing FalloutNV RefID detour TOTAL BYTES TO RESERVE: 21 */

            /* XXXXXXXX   A3 XXXXXXXX      MOV DWORD PTR DS:[XXXXXXXX],EAX
             * XXXXXXXX   E8 XXXXXXXX      CALL vaultmp.XXXXXXXX
             * XXXXXXXX   8945 E8          MOV DWORD PTR SS:[EBP-18],EAX
             * XXXXXXXX   8B45 20          MOV EAX,DWORD PTR SS:[EBP+20]
             * XXXXXXXX   -E9 XXXXXXXX     JMP Fallout3.005C49B0
             *
             * 005C49AB   -E9 XXXXXXXX     JMP XXXXXXXX
             * 005C49B0   90               NOP
             */

            bytes = 0;
            rw = 0;

            LPVOID Fallout3refidASM = VirtualAllocEx(hProc, 0, 21, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            tmp = (unsigned) &Fallout3refid;
            bytestream[0] = 0xA3; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((((unsigned) Fallout3refidASM) + bytes), (unsigned) &Fallout3refidNotify, 5);
            bytestream[0] = 0xE8; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x89; bytestream[1] = 0x45; bytestream[2] = 0xE8;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x8B; bytestream[1] = 0x45; bytestream[2] = 0x20;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((((unsigned) Fallout3refidASM) + bytes), (unsigned) 0x005C49B0, 5);
            bytestream[0] = 0xE9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((unsigned) 0x005C49AB, ((unsigned) Fallout3refidASM), 5);
            bytestream[0] = 0xE9; bytes = 0; WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x005C49AB + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x005C49AB + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;
            bytestream[0] = 0x90; WriteProcessMemory(hProc, (LPVOID) (0x005C49AB + bytes), &bytestream[0], sizeof(bytestream[0]), &rw);

            break;
        }
        case false:
        {
            /* Patching key event and console */

            /* 0062B663   83FB 0A          CMP EBX,A
             * 0062B666   90               NOP
             *
             * 0062B66B   0F85 BE150000    JNZ Fallout3.0062CC2F
             * 00627A84   EB 6E            JMP SHORT Fallout3.00627AF4
             * 0062B67E   EB 1B            JMP SHORT Fallout3.0062B69B
             *
             * 00627AFF   E9 990D0000      JMP Fallout3.0062889D
             * 00627B04   90               NOP

             * 0062B69B   90               NOP
             * 0062B69C   90               NOP
             * 0062B69D   90               NOP
             * 0062B69E   90               NOP
             * 0062B69F   90               NOP
             * 0062B6A0   90               NOP
             * 0062B6A1   90               NOP
             *
             * 0062B744   83FB 0A          CMP EBX,A
             * 0062B747   90               NOP
             * 0062B748   90               NOP
             * 0062B749   90               NOP
             *
             * 006195E8   90               NOP
             * 006195E9   90               NOP
             * 006195EA   90               NOP
             * 006195EB   90               NOP
             * 006195EC   90               NOP
             * 006195ED   90               NOP
             * 006195EE   90               NOP
             * 006195EF   90               NOP
             * 006195F0   90               NOP
             * 006195F1   90               NOP
             * 006195F2   90               NOP
             * 006195F3   90               NOP
             * 006195F4   90               NOP
             * 006195F5   90               NOP
             * 006195F6   90               NOP
             */

            bytestream[0] = 0x83; bytestream[1] = 0xFB; bytestream[2] = 0x0A; bytestream[3] = 0x90;
            for (int i = 0; i < 4; i++) WriteProcessMemory(hProc, (LPVOID) (0x0062B663 + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0x0F; bytestream[1] = 0x85; bytestream[2] = 0xBE; bytestream[3] = 0x15; bytestream[4] = 0x00; bytestream[5] = 0x00;
            for (int i = 0; i < 6; i++) WriteProcessMemory(hProc, (LPVOID) (0x0062B66B + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0xEB; bytestream[1] = 0x6E;
            for (int i = 0; i < 2; i++) WriteProcessMemory(hProc, (LPVOID) (0x00627A84 + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0xEB; bytestream[1] = 0x1B;
            for (int i = 0; i < 2; i++) WriteProcessMemory(hProc, (LPVOID) (0x0062B67E + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0xE9; bytestream[1] = 0x99; bytestream[2] = 0x0D; bytestream[3] = 0x00; bytestream[4] = 0x00; bytestream[5] = 0x90;
            for (int i = 0; i < 6; i++) WriteProcessMemory(hProc, (LPVOID) (0x00627AFF + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0x90; bytestream[1] = 0x90; bytestream[2] = 0x90; bytestream[3] = 0x90; bytestream[4] = 0x90; bytestream[5] = 0x90; bytestream[6] = 0x90;
            for (int i = 0; i < 7; i++) WriteProcessMemory(hProc, (LPVOID) (0x0062B69B + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0x83; bytestream[1] = 0xFB; bytestream[2] = 0x0A; bytestream[3] = 0x90; bytestream[4] = 0x90; bytestream[5] = 0x90;
            for (int i = 0; i < 6; i++) WriteProcessMemory(hProc, (LPVOID) (0x0062B744 + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            bytestream[0] = 0x90; bytestream[1] = 0x90; bytestream[2] = 0x90; bytestream[3] = 0x90; bytestream[4] = 0x90; bytestream[5] = 0x90; bytestream[6] = 0x90; bytestream[7] = 0x90;
            bytestream[8] = 0x90; bytestream[9] = 0x90; bytestream[10] = 0x90; bytestream[11] = 0x90; bytestream[12] = 0x90; bytestream[13] = 0x90; bytestream[14] = 0x90;
            for (int i = 0; i < 15; i++) WriteProcessMemory(hProc, (LPVOID) (0x006195E8 + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            /* Disabling VATS system */

            /* 0078A27D   E9 88010000      JMP Fallout3.0078A40A
             * 0078A282   90               NOP
             */

            bytes = 0;
            rw = 0;

            bytestream[0] = 0xE9; bytestream[1] = 0x88; bytestream[2] = 0x01; bytestream[3] = 0x00; bytestream[4] = 0x00; bytestream[5] = 0x90;
            for (int i = 0; i < 6; i++) WriteProcessMemory(hProc, (LPVOID) (0x0078A27D + i), &bytestream[i], sizeof(bytestream[i]), &rw);

            /* Writing Fallout3 command trigger TOTAL BYTES TO RESERVE: 20 */

            /* XXXXXXXX   8B1D XXXXXXXX    MOV EBX,[XXXXXXXX]
             * XXXXXXXX   83FB 0A          CMP EBX,A
             * XXXXXXXX  -0F84 XXXXXXXX    JE Fallout3.006288CA
             * XXXXXXXX  -E9 XXXXXXXX      JMP Fallout3.0062897A
             *
             * 006288C4  -E9 XXXXXXXX      JMP XXXXXXXX
             * 006288C9   90               NOP
             */

            bytes = 0;
            rw = 0;

            LPVOID Fallout3triggerASM = VirtualAllocEx(hProc, 0, 20, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            tmp = (unsigned) &Fallout3opTrigger;
            bytestream[0] = 0x8B; bytestream[1] = 0x1D;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x83; bytestream[1] = 0xFB; bytestream[2] = 0x0A;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((((unsigned) Fallout3triggerASM) + bytes), (unsigned) 0x006288CA, 6);
            bytestream[0] = 0x0F; bytestream[1] = 0x84;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((((unsigned) Fallout3triggerASM) + bytes), (unsigned) 0x0062897A, 5);
            bytestream[0] = 0xE9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3triggerASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((unsigned) 0x006288C4, (unsigned) Fallout3triggerASM, 5);
            bytestream[0] = 0xE9; bytes = 0; WriteProcessMemory(hProc, (LPVOID) (0x006288C4 + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (0x006288C4 + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;
            bytestream[0] = 0x90; WriteProcessMemory(hProc, (LPVOID) (0x006288C4 + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;

            /* Writing Fallout3 command INPUT detour TOTAL BYTES TO RESERVE: 64 */

            /* XXXXXXXX   50               PUSH EAX
             * XXXXXXXX   51               PUSH ECX
             * XXXXXXXX   56               PUSH ESI
             * XXXXXXXX   8A06             MOV AL,BYTE PTR DS:[ESI]
             * XXXXXXXX   3C 00            CMP AL,0
             * XXXXXXXX   74 0D            JE SHORT XXXXXXXX
             * XXXXXXXX   5E               POP ESI
             * XXXXXXXX   59               POP ECX
             * XXXXXXXX   58               POP EAX
             * XXXXXXXX   BA FFFEFE7E      MOV EDX,7EFEFEFF
             * XXXXXXXX  -E9 XXXXXXXX      JMP Fallout3.00C075B4
             * XXXXXXXX   B9 XXXXXXXX      MOV ECX,XXXXXXXX
             * XXXXXXXX   8A01             MOV AL,BYTE PTR DS:[ECX]
             * XXXXXXXX   3C 00            CMP AL,0
             * XXXXXXXX  ^74 E8            JE SHORT XXXXXXXX
             * XXXXXXXX   8806             MOV BYTE PTR DS:[ESI],AL
             * XXXXXXXX   C601 00          MOV BYTE PTR DS:[ECX],0
             * XXXXXXXX   83C1 01          ADD ECX,1
             * XXXXXXXX   83C6 01          ADD ESI,1
             * XXXXXXXX   8A01             MOV AL,BYTE PTR DS:[ECX]
             * XXXXXXXX   3C 00            CMP AL,0
             * XXXXXXXX   74 02            JE SHORT XXXXXXXX
             * XXXXXXXX  ^EB ED            JMP SHORT XXXXXXXX
             * XXXXXXXX   C605 XXXXXXXX 00 MOV BYTE PTR DS:[XXXXXXXX],0
             * XXXXXXXX   C606 00          MOV BYTE PTR DS:[ESI],0
             * XXXXXXXX  ^EB C9            JMP SHORT XXXXXXXX
             *
             * 00C075AF   -E9 XXXXXXXX     JMP XXXXXXXX
             */

            bytes = 0;
            rw = 0;

            LPVOID Fallout3inputASM = VirtualAllocEx(hProc, 0, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            bytestream[0] = 0x50; bytestream[1] = 0x51; bytestream[2] = 0x56;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x8A; bytestream[1] = 0x06;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x3C; bytestream[1] = 0x00;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x74; bytestream[1] = 0x0D;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x5E; bytestream[1] = 0x59; bytestream[2] = 0x58;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xBA; bytestream[1] = 0xFF; bytestream[2] = 0xFE; bytestream[3] = 0xFE; bytestream[4] = 0x7E;
            for (int i = 0; i < 5; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((((unsigned) Fallout3inputASM) + bytes), (unsigned) 0x00C075B4, 5);
            bytestream[0] = 0xE9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = (unsigned) &Fallout3input;
            bytestream[0] = 0xB9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x8A; bytestream[1] = 0x01;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x3C; bytestream[1] = 0x00;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x74; bytestream[1] = 0xE8;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x88; bytestream[1] = 0x06;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xC6; bytestream[1] = 0x01; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x83; bytestream[1] = 0xC1; bytestream[2] = 0x01;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x83; bytestream[1] = 0xC6; bytestream[2] = 0x01;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x8A; bytestream[1] = 0x01;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x3C; bytestream[1] = 0x00;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x74; bytestream[1] = 0x02;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xEB; bytestream[1] = 0xED;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = (unsigned) &Fallout3mutex;
            bytestream[0] = 0xC6; bytestream[1] = 0x05;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;
            bytestream[0] = 0x00; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;

            bytestream[0] = 0xC6; bytestream[1] = 0x06; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xEB; bytestream[1] = 0xC9;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3inputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((unsigned) 0x00C075AF, ((unsigned) Fallout3inputASM), 5);
            bytestream[0] = 0xE9; bytes = 0; WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x00C075AF + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x00C075AF + bytes), &tmp, sizeof(tmp), &rw);

            /* Writing Fallout3 command OUTPUT detour TOTAL BYTES TO RESERVE: 46 */

            /* XXXXXXXX   50               PUSH EAX
             * XXXXXXXX   51               PUSH ECX
             * XXXXXXXX   52               PUSH EDX
             * XXXXXXXX   B9 XXXXXXXX      MOV ECX,XXXXXXXX
             * XXXXXXXX   8A10             MOV DL,BYTE PTR DS:[EAX]
             * XXXXXXXX   80FA 00          CMP DL,0
             * XXXXXXXX   74 0A            JE SHORT XXXXXXXX
             * XXXXXXXX   8811             MOV BYTE PTR DS:[ECX],DL
             * XXXXXXXX   83C0 01          ADD EAX,1
             * XXXXXXXX   83C1 01          ADD ECX,1
             * XXXXXXXX  ^EB EF            JMP SHORT XXXXXXXX
             * XXXXXXXX   C601 00          MOV BYTE PTR DS:[ECX],0
             * XXXXXXXX   E8 XXXXXXXX      CALL vaultmp.XXXXXXXX
             * XXXXXXXX   5A               POP EDX
             * XXXXXXXX   59               POP ECX
             * XXXXXXXX   58               POP EAX
             * XXXXXXXX   E8 XXXXXXXX      CALL Fallout3.0062A800
             * XXXXXXXX  -E9 XXXXXXXX      JMP Fallout3.0062B230
             *
             * 0062B22B   -E9 XXXXXXXX     JMP XXXXXXXX
             */

            bytes = 0;
            rw = 0;

            LPVOID Fallout3outputASM = VirtualAllocEx(hProc, 0, 46, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            bytestream[0] = 0x50; bytestream[1] = 0x51; bytestream[2] = 0x52;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = (unsigned) &Fallout3output;
            bytestream[0] = 0xB9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x8A; bytestream[1] = 0x10;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x80; bytestream[1] = 0xFA; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x74; bytestream[1] = 0x0A;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x88; bytestream[1] = 0x11;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x83; bytestream[1] = 0xC0; bytestream[2] = 0x01;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0x83; bytestream[1] = 0xC1; bytestream[2] = 0x01;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xEB; bytestream[1] = 0xEF;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            bytestream[0] = 0xC6; bytestream[1] = 0x01; bytestream[2] = 0x00;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((((unsigned) Fallout3outputASM) + bytes), ((unsigned) &Fallout3commandNotify), 5);
            bytestream[0] = 0xE8; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            bytestream[0] = 0x5A; bytestream[1] = 0x59; bytestream[2] = 0x58;
            for (int i = 0; i < 3; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }

            tmp = offset((((unsigned) Fallout3outputASM) + bytes), (unsigned) 0x0062A800, 5);
            bytestream[0] = 0xE8; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((((unsigned) Fallout3outputASM) + bytes), (unsigned) 0x0062B230, 5);
            bytestream[0] = 0xE9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3outputASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((unsigned) 0x0062B22B, ((unsigned) Fallout3outputASM), 5);
            bytestream[0] = 0xE9; bytes = 0; WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x0062B22B + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x0062B22B + bytes), &tmp, sizeof(tmp), &rw);

            /* Writing Fallout3 RefID detour TOTAL BYTES TO RESERVE: 21 */

            /* XXXXXXXX   8915 XXXXXXXX    MOV DWORD PTR DS:[XXXXXXXX],EDX
             * XXXXXXXX   E8 XXXXXXXX      CALL vaultmp.XXXXXXXX
             * XXXXXXXX   E8 XXXXXXXX      CALL Fallout3.00516790
             * XXXXXXXX   -E9 XXXXXXXX     JMP Fallout3.0053CACF
             *
             * 0053CAC6   -E9 XXXXXXXX     JMP XXXXXXXX
             */

            bytes = 0;
            rw = 0;

            LPVOID Fallout3refidASM = VirtualAllocEx(hProc, 0, 21, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            tmp = (unsigned) &Fallout3refid;
            bytestream[0] = 0x89; bytestream[1] = 0x15;
            for (int i = 0; i < 2; i++) { WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &bytestream[i], sizeof(bytestream[i]), &rw); bytes += rw; }
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((((unsigned) Fallout3refidASM) + bytes), (unsigned) &Fallout3refidNotify, 5);
            bytestream[0] = 0xE8; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((((unsigned) Fallout3refidASM) + bytes), (unsigned) 0x00516790, 5);
            bytestream[0] = 0xE8; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((((unsigned) Fallout3refidASM) + bytes), (unsigned) 0x0053CACF, 5);
            bytestream[0] = 0xE9; WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) (((unsigned) Fallout3refidASM) + bytes), &tmp, sizeof(tmp), &rw); bytes += rw;

            tmp = offset((unsigned) 0x0053CAC6, ((unsigned) Fallout3refidASM), 5);
            bytestream[0] = 0xE9; bytes = 0; WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x0053CAC6 + bytes), &bytestream[0], sizeof(bytestream[0]), &rw); bytes += rw;
            WriteProcessMemory(hProc, (LPVOID) ((unsigned) 0x0053CAC6 + bytes), &tmp, sizeof(tmp), &rw);

            break;
        }
    }

    CloseHandle(hProc);

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
