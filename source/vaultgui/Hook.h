#include <windows.h>
#include <fstream>
#include <d3d9.h>
#include <conio.h>

//#define TARGET_FUNCTION "GetModuleHandleW"
#define IMPORT_TABLE_OFFSET 1

#define JMP32_SZ 5 // the size of JMP

#define NOP 0x90 // opcode for NOP
#define JMP 0xE9 // opcode for JUMP
#define CAL 0xE8 // opcode for JUMP



void HookJmp(BYTE*,BYTE*,int);
void HookCall(BYTE*,BYTE*,int);
