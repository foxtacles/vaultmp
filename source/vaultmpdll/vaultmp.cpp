#include <winsock2.h>
#include <cstdio>
#include <vector>
#include <string>

#include "vaultmp.h"

using namespace std;

// FOSE
enum eEmotion
{
	happy	= 0,
	sad		= 1,
	neutral = 2,
	pain	= 3
};

typedef void (*CallCommand)(void*, void*, void*, void*, void*, void*, void*, void*);
typedef bool (*QueueUIMessage_Fallout3)(const char* msg, unsigned int emotion, const char* ddsPath, const char* soundName, float msgTime);
typedef bool (*QueueUIMessage_FalloutNV)(const char* msg, unsigned int emotion, const char* ddsPath, const char* soundName, float msgTime, char unk);
typedef unsigned int (*LookupForm)(unsigned int);
typedef unsigned int (*LookupFunc)(unsigned int);
typedef void (*Chatbox_AddToChat)(const char*);
typedef const char* (*Chatbox_GetQueue)();

static HANDLE hProc;
static PipeServer pipeServer;
static PipeClient pipeClient;
static LookupForm FormLookup;
static LookupFunc FuncLookup;
static Chatbox_AddToChat AddToChat;
static Chatbox_GetQueue GetQueue;
static QueueUIMessage_Fallout3 QueueMessage_Fallout3;;
static QueueUIMessage_FalloutNV QueueMessage_FalloutNV;

static void PatchGame(HINSTANCE& silverlock);
static void BethesdaDelegator();
static void ToggleRespawn();
static void RespawnDetour();
static void AnimDetour_F3();
static void PlayIdleDetour_F3();
static void AnimDetour_FNV();
static void PlayIdleDetour_FNV();
static void AVFix_F3();
static void AVFix_FNV();
static vector<void*> delegated;

typedef void (__stdcall * _GetSystemTimeAsFileTime)(LPFILETIME * fileTime);
static _GetSystemTimeAsFileTime GetSystemTimeAsFileTime_Original = NULL;
static _GetSystemTimeAsFileTime * _GetSystemTimeAsFileTime_IAT = NULL;
static HINSTANCE silverlock = NULL;
static HINSTANCE vaultgui = NULL;

static bool delegate = false;
static bool respawn = true;
static bool DLLerror = false;
static unsigned int anim = 0x00;
static unsigned int* _anim = NULL;
static unsigned char game = 0x00;


//Game ready hook address
static const unsigned FalloutNVPatch_gamereadyhook = 0x403e05;
static const unsigned FalloutNVPatch_gamereadyvariable = 0x401015;


static const unsigned FalloutNVpatch_disableNAM = 0x01018814;
static const unsigned FalloutNVpatch_pluginsVMP = 0x0108282D;
static const unsigned FalloutNVpatch_PlayGroup = 0x00494D5C;
static const unsigned FalloutNVpatch_delegator_src = 0x0086B3E3;
static const unsigned FalloutNVpatch_delegator_dest = 0x0086E649;
static const unsigned FalloutNVpatch_delegatorCall_src = 0x0086E64A;
static const unsigned FalloutNVpatch_delegatorCall_dest = (unsigned)& BethesdaDelegator;
static const unsigned FalloutNVpatch_noRespawn_NOP = 0x00851304; // 2x NOP
static const unsigned FalloutNVpatch_noRespawn_jmp = 0x0093FF83;
static const unsigned FalloutNVpatch_noRespawn_call_src = 0x0093FF85;
static const unsigned FalloutNVpatch_noRespawn_call_dest = 0x007D0A70;
static const unsigned FalloutNVpatch_noRespawn_call_detour = (unsigned)& RespawnDetour;
static const unsigned FalloutNVpatch_playIdle_call_src = 0x008D96BA;
static const unsigned FalloutNVpatch_playIdle_call_dest = (unsigned)& AnimDetour_FNV;
static const unsigned FalloutNVpatch_playIdle_fix_src = 0x005CB4F1;
static const unsigned FalloutNVpatch_playIdle_fix_dest = (unsigned)& PlayIdleDetour_FNV;
static const unsigned FalloutNVpatch_matchRace_NOP1 = 0x005DA85D;
static const unsigned FalloutNVpatch_matchRace_NOP2 = 0x005DA8A8;
static const unsigned FalloutNVpatch_matchRace_patch = 0x005DA8C4;
static const unsigned FalloutNVpatch_matchRace_param = 0x0118D5C8;
static const unsigned FalloutNVpatch_Lock = 0x005CC08B;
static unsigned FalloutNVpatch_playIdle_fix_ret = 0x005CB4F6;
static unsigned FalloutNVpatch_AVFix_src = 0x004AEC57;
static unsigned FalloutNVpatch_AVFix_dest = (unsigned)& AVFix_FNV;
static unsigned FalloutNVpatch_AVFix_ret = 0x004AEC5C;
static unsigned FalloutNVpatch_AVFix_call = 0x0084E3A0;
static unsigned FalloutNVpatch_AVFix_term = 0x004AEDCB;

static const unsigned Fallout3patch_pluginsVMP = 0x00E10FF1;
static const unsigned Fallout3patch_PlayGroup = 0x0045F704;
static const unsigned Fallout3patch_delegator_src = 0x006EEC86;
static const unsigned Fallout3patch_delegator_dest = 0x006EDBD9;
static const unsigned Fallout3patch_delegatorCall_src = 0x006EDBDA;
static const unsigned Fallout3patch_delegatorCall_dest = (unsigned)& BethesdaDelegator;
static const unsigned Fallout3patch_noRespawn_NOP = 0x006D5965; // 2x NOP
static const unsigned Fallout3patch_noRespawn_jmp_src = 0x0078B230;
static const unsigned Fallout3patch_noRespawn_jmp_dest = 0x0078B2B9;
static const unsigned Fallout3patch_noRespawn_jmp_detour = (unsigned)& RespawnDetour;
static const unsigned Fallout3patch_playIdle_call_src = 0x0073BB20;
static const unsigned Fallout3patch_playIdle_call_dest = (unsigned)& AnimDetour_F3;
static const unsigned Fallout3patch_playIdle_fix_src = 0x00534D8D;
static const unsigned Fallout3patch_playIdle_fix_dest = (unsigned)& PlayIdleDetour_F3;
static const unsigned Fallout3patch_matchRace_NOP1 = 0x0052F4DD;
static const unsigned Fallout3patch_matchRace_NOP2 = 0x0052F50F;
static const unsigned Fallout3patch_matchRace_patch = 0x0052F513;
static const unsigned Fallout3patch_matchRace_param = 0x00F51ADC;
static const unsigned Fallout3patch_Lock = 0x00527F33;
static unsigned Fallout3patch_AVFix_src = 0x00473D35;
static unsigned Fallout3patch_AVFix_dest = (unsigned)& AVFix_F3;
static unsigned Fallout3patch_AVFix_ret = 0x00473D3B;
static unsigned Fallout3patch_AVFix_term = 0x00473E85;

// Those snippets / functions are from FOSE / NVSE, thanks

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
	*((unsigned short*)addr) = data;
	VirtualProtect((void*) addr, 4, oldProtect, (DWORD*) &oldProtect);
}

void SafeWrite32(unsigned int addr, unsigned int data)
{
	unsigned int oldProtect;

	VirtualProtect((void*) addr, 4, PAGE_EXECUTE_READWRITE, (DWORD*) &oldProtect);
	*((unsigned int*) addr) = data;
	VirtualProtect((void*) addr, 4, oldProtect, (DWORD*) &oldProtect);
}

void SafeWriteBuf(unsigned int addr, void* data, unsigned int len)
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

void * GetIATAddr(unsigned char * base, const char * searchDllName, const char * searchImportName)
{
	IMAGE_DOS_HEADER		* dosHeader = (IMAGE_DOS_HEADER *)base;
	IMAGE_NT_HEADERS		* ntHeader = (IMAGE_NT_HEADERS *)(base + dosHeader->e_lfanew);
	IMAGE_IMPORT_DESCRIPTOR	* importTable =
		(IMAGE_IMPORT_DESCRIPTOR *)(base + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	for(; importTable->Characteristics; ++importTable)
	{
		const char	* dllName = (const char *)(base + importTable->Name);

		if(!_stricmp(dllName, searchDllName))
		{
			// found the dll

			IMAGE_THUNK_DATA	* thunkData = (IMAGE_THUNK_DATA *)(base + importTable->OriginalFirstThunk);
			unsigned int				* iat = (unsigned int *)(base + importTable->FirstThunk);

			for(; thunkData->u1.Ordinal; ++thunkData, ++iat)
			{
				if(!IMAGE_SNAP_BY_ORDINAL(thunkData->u1.Ordinal))
				{
					IMAGE_IMPORT_BY_NAME	* importInfo = (IMAGE_IMPORT_BY_NAME *)(base + thunkData->u1.AddressOfData);

					if(!_stricmp((char *)importInfo->Name, searchImportName))
					{
						// found the import
						return iat;
					}
				}
			}

			return NULL;
		}
	}

	return NULL;
}

void __stdcall GetSystemTimeAsFileTime_Hook(LPFILETIME * fileTime)
{
	PatchGame(silverlock);

	GetSystemTimeAsFileTime_Original(fileTime);
}

void BethesdaDelegator()
{
	if (delegate)
	{
		CallCommand Call = (CallCommand) delegated[8];
		Call(delegated[0], delegated[1], delegated[2], delegated[3], delegated[4], delegated[5], delegated[6], delegated[7]);
		delegate = false;
	}
}

void RespawnDetour()
{
	ToggleRespawn();

	if (game == NEWVEGAS)
		reinterpret_cast<void(*)()>(FalloutNVpatch_noRespawn_call_dest)();
}

void AnimDetour_F3()
{
	asm volatile(
		"PUSH EAX\n"
		"LEA EAX,[ECX+0x414]\n"
		"CMP EAX,%2\n"
		"JNE _nopatch\n"
		"MOV EAX,%1\n"
		"MOV %0,0\n"
		"JMP _patch\n"

		"_nopatch:\n"
		"XOR EAX,EAX\n"

		"_patch:\n"
		"MOV [ECX+0x414],EAX\n"
		"POP EAX\n"
		: "=m"(anim)
		: "m"(anim), "m"(_anim)
		:
	);
}

void PlayIdleDetour_F3()
{
	asm volatile(
		"CMP DWORD PTR [EBP+0xC],0x14\n"
		"JNE _push1\n"
		"MOV %0,0x80\n"
		"MOV %1,ECX\n"
		"ADD %1,0x414\n"

		"_push1:\n"
		"PUSH 0x80\n"
		"CALL EAX\n"
		: "=m"(anim), "=m"(_anim)
		:
		:
	);
}

void AnimDetour_FNV()
{
	asm volatile(
		"PUSH ECX\n"
		"LEA ECX,[EAX+0x424]\n"
		"CMP ECX,%2\n"
		"JNE _nopatch2\n"

		"MOV ECX,%1\n"
		"TEST ECX,ECX\n"
		"JE _store\n"
		"CMP ECX,0x100\n"
		"JE _first\n"
		"MOV %0,0\n"
		"JMP _store\n"

		"_first:\n"
		"SHR ECX,1\n"
		"MOV %0,ECX\n"
		"JMP _store\n"

		"_nopatch2:\n"
		"XOR ECX,ECX\n"

		"_store:\n"
		"MOV [EAX+0x424],ECX\n"
		"POP ECX\n"
		: "=m"(anim)
		: "m"(anim), "m"(_anim)
		:
	);
}

void PlayIdleDetour_FNV()
{
	asm volatile(
		"CMP DWORD PTR [ECX+0xC],0x14\n"
		"JNE _push2\n"
		"MOV %0,0x100\n"
		"MOV ECX,[EBP-0x220]\n"
		"MOV %1,ECX\n"
		"ADD %1,0x424\n"

		"_push2:\n"
		"PUSH 0x80\n"
		"JMP %2\n"
		: "=m"(anim), "=m"(_anim)
		: "m"(FalloutNVpatch_playIdle_fix_ret)
		:
	);
}

void AVFix_F3()
{
	asm volatile(
		"TEST ECX,ECX\n"
		"JNE _doit\n"
		"JMP %1\n"

		"_doit:\n"
		"MOV EDX,[ECX]\n"
		"MOV [ESP+0x34],EAX\n"
		"JMP %0\n"
		:
		:  "m"(Fallout3patch_AVFix_ret), "m"(Fallout3patch_AVFix_term)
		:
	);
}

void GameReady_NV()
{
    asm volatile(
        "mov [ebp-8],ecx\n"
        "cmp ecx,0x0105bd68\n" //0101c524
        "jne _notready\n"
        "mov dword ptr [0x401015],0\n"
        "_notready:\n"
        "push 0x403e11\n"
        "ret\n"
    );
}

void AVFix_FNV()
{
	asm volatile(
		"TEST ECX,ECX\n"
		"JNE _doit2\n"
		"JMP %2\n"

		"_doit2:\n"
		"CALL %0\n"
		"JMP %1\n"
		:
		: "m"(FalloutNVpatch_AVFix_call), "m"(FalloutNVpatch_AVFix_ret), "m"(FalloutNVpatch_AVFix_term)
		:
	);
}

bool vaultfunction(void* reference, void* result, void* args, unsigned short opcode)
{
	switch (opcode)
	{
		case 0x0001 | VAULTFUNCTION: // GetActorState - returns the actors animations, alerted / sneaking state
		{
			ZeroMemory(result, sizeof(double));
			unsigned char* data;

			// thiscall convention
			asm(
				"MOV ECX,%1\n"
				"CALL %2\n"
				"MOV %0,EAX\n"
				: "=m"(data)
				: "m"(reference), "r"(*((unsigned int*)((unsigned) *((unsigned int*) reference) + (unsigned) 0x01E4)))
				: "eax", "ecx"
			);

			if (data != NULL)
			{
				unsigned char sneaking, moving, weapon;
				unsigned int idle = 0x00000000;
				unsigned char* anim = data;

				moving = *(anim + 0x4E);
				weapon = *(anim + 0x54);

				asm(
					"MOV ECX,%1\n"
					"CALL %2\n"
					"MOV %0,EAX\n"
					: "=m"(data)
					: "m"(reference), "r"((game & FALLOUT3) ? SNEAKING_STATE_FALLOUT3 : SNEAKING_STATE_NEWVEGAS)
					: "eax", "ecx", "edx"
				);

				sneaking = (game & FALLOUT3 ? ((bool)((unsigned) data & 0x00000400)) : ((unsigned) data & 0x00000001));

				unsigned int data = *(unsigned int*) (anim + (game & FALLOUT3 ? 0x118 : 0x124));

				if (data)
				{
					data = *(unsigned int*)(data + 0x2C);
					idle = *(unsigned int*)(data + 0x0C);
				}

				// This detection is unreliable; i.e. what when the player holds Forward/Backward/Left/Right all together for some reason?
				unsigned char* _args = (unsigned char*) args;
				unsigned char flags = 0x00;

				if (*_args == 0x6E)   // Forward, Backward, Left, Right unsigned char scan codes stored in Integer, optional
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
						flags |= 0x01; // that equals to a Z-angle correction of -45²
					}
					else if (((GetAsyncKeyState(MapVirtualKey(forward, 1)) & 0x8000) && (GetAsyncKeyState(MapVirtualKey(right, 1)) & 0x8000))
							 || ((GetAsyncKeyState(MapVirtualKey(backward, 1)) & 0x8000) && (GetAsyncKeyState(MapVirtualKey(left, 1)) & 0x8000)))
					{
						flags |= 0x02; // that equals to a Z-angle correction of 45²
					}
				}

				flags |= (GetAsyncKeyState(0x54) & 0x8000) ? 0x04 : 0x00; // T
				flags |= (GetAsyncKeyState(VK_ESCAPE) & 0x8000) ? 0x08 : 0x00;
				flags |= (GetAsyncKeyState(VK_RETURN) & 0x8000) ? 0x10 : 0x00;

				memcpy(result, &idle, 4);
				memcpy((void*)((unsigned) result + 4), &moving, 1);
				memcpy((void*)((unsigned) result + 5), &flags, 1);
				memcpy((void*)((unsigned) result + 6), &weapon, 1);
				memcpy((void*)((unsigned) result + 7), &sneaking, 1);
			}

			break;
		}

		case 0x0002 | VAULTFUNCTION: // Chat - Print chat message
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			AddToChat(data);
			break;
		}

		case 0x0003 | VAULTFUNCTION: // ScanContainer - Returns a containers content including baseID, amount, condition, equipped state
		case 0x0005 | VAULTFUNCTION: // RemoveAllItemsEx - same functionality, client will use the information for RemoveItem
		{
			ZeroMemory(result, sizeof(double));

			unsigned int count;

			asm(
				"MOV ECX,%1\n"
				"PUSH 1\n"
				"PUSH 0\n"
				"CALL %2\n"
				"MOV %0,EAX\n"
				: "=m"(count)
				: "m"(reference), "r"((game & FALLOUT3) ? ITEM_COUNT_FALLOUT3 : ITEM_COUNT_NEWVEGAS)
				: "eax", "ecx"
			);

			if (count > 0)
			{
				unsigned int size = count * 20;
				vector<unsigned char> container;
				container.reserve(size);

				for (unsigned int i = 0; i < count; ++i)
				{
					unsigned int item;

					// dynamic allocation here

					asm(
						"MOV ECX,%2\n"
						"PUSH 0\n"
						"PUSH %1\n"
						"CALL %3\n"
						"MOV %0,EAX\n"
						: "=m"(item)
						: "r"(i), "m"(reference), "r"((game & FALLOUT3) ? ITEM_GET_FALLOUT3 : ITEM_GET_NEWVEGAS)
						: "eax", "ecx"
					);

					if (item)
					{
						unsigned int amount = *((unsigned int*)(((unsigned) item) + 0x04));
						unsigned int baseForm = *((unsigned int*)(((unsigned) item) + 0x08));

						if (baseForm)
						{
							unsigned char type = *((unsigned char*)(((unsigned) baseForm) + 0x04));
							unsigned int baseID = *((unsigned int*)(((unsigned) baseForm) + 0x0C));

							container.insert(container.end(), (unsigned char*) &baseID, ((unsigned char*) &baseID) + 4);
							container.insert(container.end(), (unsigned char*) &amount, ((unsigned char*) &amount) + 4);

							unsigned int equipped;

							asm(
								"MOV ECX,%1\n"
								"PUSH 0\n"
								"CALL %2\n"
								"MOV %0,EAX\n"
								: "=m"(equipped)
								: "m"(item), "r"((game & FALLOUT3) ? ITEM_ISEQUIPPED_FALLOUT3 : ITEM_ISEQUIPPED_NEWVEGAS)
								: "eax", "ecx"
							);

							equipped &= 0x00000001;

							container.insert(container.end(), (unsigned char*) &equipped, ((unsigned char*) &equipped) + 4);

							double condition;

							// there's also a way to get the absolute health value

							if (type == 0x18 || type == 0x28)
							{
								asm(
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

							if (game & FALLOUT3)
							{
							    asm (
							        "MOV ECX,%0\n"
							        "CALL %1\n"
							        "PUSH %0\n"
							        :
							        : "m"(item), "r"(ITEM_UNK1_FALLOUT3)
							        : "ecx"
							    );

								// the following is probably a free function

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
							}
						}
						else
							return true;
					}
				}

				size = container.size();
				unsigned char* data = new unsigned char[size];
				memcpy(data, &container[0], size);

				memcpy(result, &size, 4);
				memcpy((void*)((unsigned) result + 4), &data, 4);
			}

			return true;
		}

		case 0x0004 | VAULTFUNCTION: // UIMessage - Queue UI message
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length

			unsigned int emoticon = *(unsigned int*)(data + strlen(data) + 2);

			if (game & FALLOUT3)
				QueueMessage_Fallout3(data, emoticon, NULL, NULL, 2.0); // add more later
			else
				QueueMessage_FalloutNV(data, emoticon, NULL, NULL, 2.0, 0); // add more later

			break;
		}

		case 0x0006 | VAULTFUNCTION: // ForceRespawn - returns to the main screen
		{
			ToggleRespawn();

			while (respawn)
				Sleep(1);
			break;
		}

		case 0x0007 | VAULTFUNCTION: // SetGlobalValue - sets a global value
		{
			if (!reference)
				return false;

			unsigned char* global_value = (unsigned char*) reference + 0x24;
			unsigned char* _args = (unsigned char*) args;

			if (*_args == 0x6E) // just using int (stored as 4-byte float) so far
				*(float*) global_value = (float) *(signed int*) (_args + 1);
		}

		default:
			break;
	}

	return false;
}

void ExecuteCommand(vector<void*>& args, unsigned int r, bool delegate_flag)
{
	if (args.size() != 8)
		return;

	unsigned int reference = *((unsigned int*) args[2]);
	unsigned short opcode;
	void* _args;

	if (*((unsigned int*) args[1]) == 0x0001001C)
	{
		opcode = *((unsigned short*)(((unsigned) args[1]) + 4));
		_args = (void*)(((unsigned) args[1]) + 4 + 2 + 2 + 2);            // skip 0001001C, opcode, unk2, numargs
	}
	else
	{
		opcode = *((unsigned short*) args[1]);
		_args = (void*)(((unsigned) args[1]) + 2 + 2 + 2);            // skip opcode, unk2, numargs
	}

	if (opcode == 0x00)
		return;

	if (reference != 0x00)
	{
		reference = FormLookup(reference);

		if (reference == 0x00)
			return;
	}

	void* arg4 = args[4];
	unsigned int base = (unsigned) arg4;

	unsigned int** param1 = (unsigned int**)(base + 0x44);
	unsigned int*** param2 = (unsigned int***)(base + 0x48);

	if (*param2 == 0x00000000)
	{
		param1 = (unsigned int**)(base + 0x40);
		param2 = (unsigned int***)(base + 0x44);
	}

	*param1 = (unsigned int*)((unsigned) *param1 + base);
	*param2 = (unsigned int**)((unsigned) *param2 + base);
	**param2 = (unsigned int*)((unsigned) **param2 + base);

	unsigned int param1_ref = *((unsigned int*)(((unsigned) *param1) + 0x08));
	unsigned int param2_ref = *((unsigned int*)(((unsigned) **param2) + 0x08));

	if (param1_ref != 0x00)
	{
		param1_ref = FormLookup(param1_ref);

		if (param1_ref == 0x00)
			return;

		*((unsigned int*)(((unsigned) *param1) + 0x08)) = param1_ref;
	}

	if (param2_ref != 0x00)
	{
		param2_ref = FormLookup(param2_ref);

		if (param2_ref == 0x00)
			return;

		*((unsigned int*)(((unsigned)** param2) + 0x08)) = param2_ref;
	}

	bool bigresult = false;

	if ((opcode & VAULTFUNCTION) == VAULTFUNCTION)
		bigresult = vaultfunction((void*) reference, args[6], _args, opcode);
	else
	{
		unsigned int function = FuncLookup((unsigned int) opcode);

		if (function == 0x00)
			return;

		void* callAddr = (void*) *((unsigned int*)(function + 0x18));

		if (callAddr == 0x00)
			return;

		if (delegate_flag)
		{
			delegated.clear();
			delegated.reserve(9);
			delegated.push_back(args[0]);
			delegated.push_back(args[1]);
			delegated.push_back((void*) reference);
			delegated.push_back((void*) *((unsigned int*) args[3]));
			delegated.push_back(args[4]);
			delegated.push_back((void*) &arg4);
			delegated.push_back(args[6]);
			delegated.push_back(args[7]);
			delegated.push_back(callAddr);
			delegate = true;

			while (delegate)
				Sleep(1);
		}
		else
		{
			CallCommand Call = (CallCommand) callAddr;
			Call(args[0], args[1], (void*) reference, (void*) * ((unsigned int*) args[3]), args[4], (void*) &arg4, args[6], args[7]);
		}
	}

	unsigned char result[PIPE_LENGTH];
	ZeroMemory(result, sizeof(result));

	*((unsigned int*)((unsigned) result + 1)) = r;

	if (!bigresult)
	{
		result[0] = PIPE_OP_RETURN;
		memcpy(result + 5, args[6], sizeof(double));
	}
	else
	{
		result[0] = PIPE_OP_RETURN_BIG;
		void* data = args[6];
		unsigned int size = *((unsigned int*) data);
		unsigned char* _data = (unsigned char*) * ((unsigned int*)(((unsigned) data) + 4));

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

	vaultgui = LoadLibrary("vaultgui.dll");

	if (vaultgui == NULL)
		DLLerror = true;
	else
	{
		LoadLibrary("CEGUIBase.dll");
		LoadLibrary("CEGUIDevILImageCodec.dll");
		LoadLibrary("CEGUIDirect3D9Renderer.dll");
		LoadLibrary("CEGUIExpatParser.dll");
		LoadLibrary("CEGUIFalagardWRBase.dll");
		LoadLibrary("CEGUIFreeImageImageCodec.dll");
		LoadLibrary("CEGUIOpenGLRenderer.dll");
		LoadLibrary("CEGUISILLYImageCodec.dll");
		LoadLibrary("CEGUISTBImageCodec.dll");
		LoadLibrary("CEGUITGAImageCodec.dll");

		AddToChat = reinterpret_cast<Chatbox_AddToChat>(GetProcAddress(vaultgui, "Chatbox_AddToChat"));
		GetQueue = reinterpret_cast<Chatbox_GetQueue>(GetProcAddress(vaultgui, "Chatbox_GetQueue"));

		if (!AddToChat || !GetQueue)
			DLLerror = true;
	}

	pipeClient.SetPipeAttributes("BethesdaClient", PIPE_LENGTH);

	while (!pipeClient.ConnectToServer());

	pipeServer.SetPipeAttributes("BethesdaServer", PIPE_LENGTH);
	pipeServer.CreateServer();
	pipeServer.ConnectToServer();

	unsigned char buffer[PIPE_LENGTH];
	pipeClient.Receive(buffer);

	// The special Steam hook by NVSE
	if (buffer[0])
	{
		unsigned int oldProtect;
		_GetSystemTimeAsFileTime_IAT = (_GetSystemTimeAsFileTime *)GetIATAddr((unsigned char *)GetModuleHandle(NULL), "kernel32.dll", "GetSystemTimeAsFileTime");
		if(_GetSystemTimeAsFileTime_IAT)
		{
			VirtualProtect((void *)_GetSystemTimeAsFileTime_IAT, 4, PAGE_EXECUTE_READWRITE, (DWORD*) &oldProtect);
			GetSystemTimeAsFileTime_Original = *_GetSystemTimeAsFileTime_IAT;
			*_GetSystemTimeAsFileTime_IAT = GetSystemTimeAsFileTime_Hook;
			unsigned int junk;
			VirtualProtect((void *)_GetSystemTimeAsFileTime_IAT, 4, oldProtect, (DWORD*) &junk);
		}
		else
			DLLerror = true;
	}
	else
		PatchGame(silverlock);

	SetCurrentDirectory("..");

	buffer[0] = PIPE_SYS_WAKEUP;
	pipeClient.Send(buffer);

	Sleep(3000);

	while (!DLLerror)
	{
		ZeroMemory(buffer, sizeof(buffer));

		pipeServer.Receive(buffer);
		unsigned char code = buffer[0];
		unsigned char* content = buffer + 1;

		switch (code)
		{
			case PIPE_OP_COMMAND:
			{
				vector<void*> args;
				args.reserve(8);

				unsigned int r = *((unsigned int*) content);
				content += 4;

				bool delegate_flag = (bool) *content;
				content += 1;

				for (unsigned int i = 0; i < 8 && !DLLerror; ++i)
				{
					unsigned char size = *content;

					if (size)
					{
						++content;
						args.push_back((void*) content);
						content += size;
					}
					else
						DLLerror = true;
				}

				if (!DLLerror)
					ExecuteCommand(args, r, delegate_flag);

				break;
			}

			case PIPE_ERROR_CLOSE:
			{
				DLLerror = true;

				break;
			}
		}

		string chat(GetQueue());

		if (!chat.empty())
		{
			buffer[0] = PIPE_OP_RETURN_RAW;
			*reinterpret_cast<unsigned int*>(buffer + 1) = 0x0002 | VAULTFUNCTION;
			*reinterpret_cast<unsigned int*>(buffer + 5) = chat.length();
			memcpy(buffer + 9, chat.c_str(), chat.length());

			pipeClient.Send(buffer);
		}
	}

	buffer[0] = PIPE_ERROR_CLOSE;
	pipeClient.Send(buffer);

	return ((DWORD) 0);
}

void ToggleRespawn()
{
	if (respawn)
	{
		respawn = false;

		switch (game)
		{
			case FALLOUT3:
				SafeWrite8(Fallout3patch_noRespawn_NOP, 0x90);  // NOP
				SafeWrite8(Fallout3patch_noRespawn_NOP + 1, 0x90);  // NOP
				WriteRelJump(Fallout3patch_noRespawn_jmp_src, Fallout3patch_noRespawn_jmp_dest);
				break;
			case NEWVEGAS:
				SafeWrite8(FalloutNVpatch_noRespawn_NOP, 0x90);  // NOP
				SafeWrite8(FalloutNVpatch_noRespawn_NOP + 1, 0x90);  // NOP
				SafeWrite8(FalloutNVpatch_noRespawn_jmp, 0xEB);   // JMP SHORT
				SafeWrite8(FalloutNVpatch_noRespawn_jmp + 1, 0x05);   // JMP SHORT
				WriteRelCall(FalloutNVpatch_noRespawn_call_src, FalloutNVpatch_noRespawn_call_dest);
				break;
		}
	}
	else
	{
		respawn = true;

		switch (game)
		{
			case FALLOUT3:
				SafeWrite8(Fallout3patch_noRespawn_NOP, 0x75);  // JNZ
				SafeWrite8(Fallout3patch_noRespawn_NOP + 1, 0x03);
				SafeWrite8(Fallout3patch_noRespawn_jmp_src + 5, 0x90);  // NOP (original JNZ instruction is 6 bytes, our CALL/JMP only 5. fix required for call return)
				WriteRelCall(Fallout3patch_noRespawn_jmp_src, Fallout3patch_noRespawn_jmp_detour);
				break;
			case NEWVEGAS:
				SafeWrite8(FalloutNVpatch_noRespawn_NOP, 0x75);  // JNZ
				SafeWrite8(FalloutNVpatch_noRespawn_NOP + 1, 0x04);
				SafeWrite8(FalloutNVpatch_noRespawn_jmp, 0x90);   // NOP
				SafeWrite8(FalloutNVpatch_noRespawn_jmp + 1, 0x90);   // NOP
				WriteRelCall(FalloutNVpatch_noRespawn_call_src, FalloutNVpatch_noRespawn_call_detour);
				break;
		}
	}
}

void PatchGame(HINSTANCE& silverlock)
{
	TCHAR curdir[MAX_PATH+1];
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
		silverlock = LoadLibrary("nvse_1_4.dll");
	}

	if (silverlock == NULL)
		DLLerror = true;
	else
	{
		// FOSE authors thought it was a smart move to prevent disabling ESC and console key
		if (game == FALLOUT3)
		{
			unsigned int codebase = (DWORD) silverlock + 0x1000;
			unsigned char NOP[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

			// patch inlined ShouldIgnoreKey
			SafeWriteBuf(codebase + 0x14222, NOP, sizeof(NOP));
			SafeWriteBuf(codebase + 0x14260, NOP, sizeof(NOP));
		}

		// NVSE: "whatever, mods can be malicious in easier ways"
	}

	switch (game)
	{
		case FALLOUT3:
		{
			FormLookup = (LookupForm) LOOKUP_FORM_FALLOUT3;
			FuncLookup = (LookupFunc) LOOKUP_FUNC_FALLOUT3;
			QueueMessage_Fallout3 = (QueueUIMessage_Fallout3) QUEUE_UI_MESSAGE_FALLOUT3;

			SafeWrite8(Fallout3patch_delegator_dest, 0x51);   // PUSH ECX
			SafeWrite8(Fallout3patch_delegatorCall_src + 5, 0x59);   // POP ECX
			SafeWrite8(Fallout3patch_PlayGroup, 0xEB);   // JMP SHORT
			SafeWrite16(Fallout3patch_playIdle_fix_src + 5, 0x9090); // NOP NOP
			SafeWrite16(Fallout3patch_Lock, 0x9090); // NOP NOP

			unsigned char NOP[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
			SafeWriteBuf(Fallout3patch_matchRace_NOP1, NOP, sizeof(NOP));
			SafeWriteBuf(Fallout3patch_matchRace_NOP2, NOP, 3);
			SafeWrite8(Fallout3patch_matchRace_patch + 1, 0xF1);
			SafeWriteBuf(Fallout3patch_matchRace_patch + 2, NOP, 4);
			SafeWrite8(Fallout3patch_matchRace_param, 0x0F);

			WriteRelCall(Fallout3patch_delegatorCall_src, Fallout3patch_delegatorCall_dest);
			WriteRelCall(Fallout3patch_delegator_src, Fallout3patch_delegator_dest);
			WriteRelCall(Fallout3patch_playIdle_fix_src, Fallout3patch_playIdle_fix_dest);
			WriteRelJump(Fallout3patch_playIdle_call_src, Fallout3patch_playIdle_call_dest);
			WriteRelJump(Fallout3patch_AVFix_src, Fallout3patch_AVFix_dest);

			SafeWrite32(Fallout3patch_pluginsVMP, *(DWORD*)".vmp"); // redirect Plugins.txt

			break;
		}

		case NEWVEGAS:
		{
			FormLookup = (LookupForm) LOOKUP_FORM_NEWVEGAS;
			FuncLookup = (LookupFunc) LOOKUP_FUNC_NEWVEGAS;
			QueueMessage_FalloutNV = (QueueUIMessage_FalloutNV) QUEUE_UI_MESSAGE_NEWVEGAS;

			SafeWrite8(FalloutNVpatch_delegator_dest, 0x51);   // PUSH ECX
			SafeWrite8(FalloutNVpatch_delegatorCall_src + 5, 0x59);   // POP ECX
			SafeWrite8(FalloutNVpatch_PlayGroup, 0xEB);   // JMP SHORT

			unsigned char NOP[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
			SafeWriteBuf(FalloutNVpatch_playIdle_call_src + 5, NOP, 5); // 5x NOP
			SafeWrite16(FalloutNVpatch_Lock, 0x9090); // NOP NOP

			SafeWriteBuf(FalloutNVpatch_matchRace_NOP1, NOP, sizeof(NOP));
			SafeWriteBuf(FalloutNVpatch_matchRace_NOP2, NOP, 11);
			SafeWrite8(FalloutNVpatch_matchRace_patch + 1, 0x45);
			SafeWrite8(FalloutNVpatch_matchRace_patch + 2, 0xFC);
			SafeWriteBuf(FalloutNVpatch_matchRace_patch + 3, NOP, 11);
			SafeWrite8(FalloutNVpatch_matchRace_param, 0x0F);

			WriteRelCall(FalloutNVpatch_delegatorCall_src, FalloutNVpatch_delegatorCall_dest);
			WriteRelCall(FalloutNVpatch_delegator_src, FalloutNVpatch_delegator_dest);
			WriteRelCall(FalloutNVpatch_playIdle_call_src, FalloutNVpatch_playIdle_call_dest);
			WriteRelJump(FalloutNVpatch_playIdle_fix_src, FalloutNVpatch_playIdle_fix_dest);
			WriteRelJump(FalloutNVpatch_AVFix_src, FalloutNVpatch_AVFix_dest);

			SafeWrite32(FalloutNVpatch_disableNAM, *(DWORD*)".|||"); // disable .NAM files
			SafeWrite32(FalloutNVpatch_pluginsVMP, *(DWORD*)".vmp"); // redirect Plugins.txt
/*
            unsigned int oldProtect;
            VirtualProtect((void*) FalloutNVPatch_gamereadyvariable, 4, PAGE_EXECUTE_READWRITE, (DWORD*) &oldProtect);
            WriteRelJump(FalloutNVPatch_gamereadyhook, (unsigned)&GameReady_NV);
*/
			break;
		}
	}

	ToggleRespawn();
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
