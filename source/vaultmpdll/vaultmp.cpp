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
typedef bool (*QueueUIMessage)(const char* msg, unsigned int emotion, const char* ddsPath, const char* soundName, float msgTime);
typedef unsigned int (*LookupForm)(unsigned int);
typedef unsigned int (*LookupFunc)(unsigned int);
typedef void (*Chatbox_AddToChat)(const char*);
typedef const char* (*Chatbox_GetQueue)();
typedef void (*Chatbox_HideChatbox)(bool);
typedef void (*Chatbox_LockChatbox)(bool);
typedef void (*Chatbox_SetChatboxPos)(float, float);
typedef void (*Chatbox_SetChatboxSize)(float, float);

static HANDLE hProc;
static PipeServer pipeServer;
static PipeClient pipeClient;
static LookupForm FormLookup;
static LookupFunc FuncLookup;
static Chatbox_AddToChat AddToChat;
static Chatbox_GetQueue GetQueue;
static Chatbox_HideChatbox HideChatbox;
static Chatbox_LockChatbox LockChatbox;
static Chatbox_SetChatboxPos SetChatboxPos;
static Chatbox_SetChatboxSize SetChatboxSize;
static QueueUIMessage QueueMessage;

static void PatchGame(HINSTANCE& silverlock);
static void BethesdaDelegator();
static void ToggleRespawn();
static void RespawnDetour();
static void AnimDetour();
static void PlayIdleDetour();
static void AVFix();
static vector<void*> delegated;

static HINSTANCE silverlock = NULL;
static HINSTANCE vaultgui = NULL;

static bool delegate = false;
static bool respawn = true;
static bool DLLerror = false;
static unsigned int anim = 0x00;
static unsigned int* _anim = NULL;

static const unsigned pluginsVMP = 0x00E10FF1;
static const unsigned PlayGroup = 0x0045F704;
static const unsigned delegator_src = 0x006EEC86;
static const unsigned delegator_dest = 0x006EDBD9;
static const unsigned delegatorCall_src = 0x006EDBDA;
static const unsigned delegatorCall_dest = (unsigned)& BethesdaDelegator;
static const unsigned noRespawn_NOP = 0x006D5965; // 2x NOP
static const unsigned noRespawn_jmp_src = 0x0078B230;
static const unsigned noRespawn_jmp_dest = 0x0078B2B9;
static const unsigned noRespawn_jmp_detour = (unsigned)& RespawnDetour;
static const unsigned playIdle_call_src = 0x0073BB20;
static const unsigned playIdle_call_dest = (unsigned)& AnimDetour;
static const unsigned playIdle_fix_src = 0x00534D8D;
static const unsigned playIdle_fix_dest = (unsigned)& PlayIdleDetour;
static const unsigned matchRace_NOP1 = 0x0052F4DD;
static const unsigned matchRace_NOP2 = 0x0052F50F;
static const unsigned matchRace_patch = 0x0052F513;
static const unsigned matchRace_param = 0x00F51ADC;
static const unsigned LockFix = 0x00527F33;
static const unsigned aiFix1 = 0x0072051E;
static const unsigned aiFix2 = 0x006FAEE8;
static const unsigned aiFix3 = 0x006FAF19;
static const unsigned aiFix4 = 0x0042FBDC;
static unsigned AVFix_src = 0x00473D35;
static unsigned AVFix_dest = (unsigned)& AVFix;
static unsigned AVFix_ret = 0x00473D3B;
static unsigned AVFix_term = 0x00473E85;
static unsigned FireFix_jmp = 0x0079236C;
static unsigned FireFix_patch = 0x007923C5;

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
}

void AnimDetour()
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

void PlayIdleDetour()
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

void AVFix()
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
		:  "m"(AVFix_ret), "m"(AVFix_term)
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
					: "m"(reference), "r"(SNEAKING_STATE)
					: "eax", "ecx", "edx"
				);

				sneaking = (bool)((unsigned) data & 0x00000400);

				unsigned int data = *(unsigned int*) (anim + 0x118);

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
				: "m"(reference), "r"(ITEM_COUNT)
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
						: "r"(i), "m"(reference), "r"(ITEM_GET)
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
								: "m"(item), "r"(ITEM_ISEQUIPPED)
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
									: "m"(item), "r"(ITEM_CONDITION)
									: "ecx"
								);
							}

							container.insert(container.end(), (unsigned char*) &condition, ((unsigned char*) &condition) + 8);

							asm (
								"MOV ECX,%0\n"
								"CALL %1\n"
								"PUSH %0\n"
								:
								: "m"(item), "r"(ITEM_UNK1)
								: "ecx"
							);

							// the following is probably a free function

							asm (
								"MOV ECX,%0\n"
								"CALL %1\n"
								:
								: "r"(ITEM_UNK3), "r"(ITEM_UNK2)
								: "ecx"
							);
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

			QueueMessage(data, emoticon, NULL, NULL, 2.0); // add more later

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

			break;
		}

		case 0x0008 | VAULTFUNCTION: // ChatUpdate - change chatbox state
		{
			unsigned char* _args = (unsigned char*) args;

			bool enabled = (bool) *(unsigned int*)(_args + 1);
			bool locked = (bool) *(unsigned int*)(_args + 6);
			float pos_X = *(double*)(_args + 11);
			float pos_Y = *(double*)(_args + 20);
			float size_X = *(double*)(_args + 29);
			float size_Y = *(double*)(_args + 38);

			HideChatbox(!enabled);
			LockChatbox(locked);
			SetChatboxPos(pos_X, pos_Y);
			SetChatboxSize(size_X, size_Y);
			break;
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
		_args = (void*)(((unsigned) args[1]) + 4 + 2 + 2 + 2); // skip 0001001C, opcode, unk2, numargs
	}
	else
	{
		opcode = *((unsigned short*) args[1]);
		_args = (void*)(((unsigned) args[1]) + 2 + 2 + 2); // skip opcode, unk2, numargs
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
		HideChatbox = reinterpret_cast<Chatbox_HideChatbox>(GetProcAddress(vaultgui, "HideChatbox"));
		LockChatbox = reinterpret_cast<Chatbox_LockChatbox>(GetProcAddress(vaultgui, "LockChatbox"));
		SetChatboxPos = reinterpret_cast<Chatbox_SetChatboxPos>(GetProcAddress(vaultgui, "SetChatboxPos"));
		SetChatboxSize = reinterpret_cast<Chatbox_SetChatboxSize>(GetProcAddress(vaultgui, "SetChatboxSize"));

		if (!AddToChat || !GetQueue || !HideChatbox || !LockChatbox || !SetChatboxPos || !SetChatboxSize)
			DLLerror = true;
	}

	pipeClient.SetPipeAttributes("BethesdaClient", PIPE_LENGTH);

	while (!pipeClient.ConnectToServer());

	pipeServer.SetPipeAttributes("BethesdaServer", PIPE_LENGTH);
	pipeServer.CreateServer();
	pipeServer.ConnectToServer();

	unsigned char buffer[PIPE_LENGTH];

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

		SafeWrite8(noRespawn_NOP, 0x90);  // NOP
		SafeWrite8(noRespawn_NOP + 1, 0x90);  // NOP
		WriteRelJump(noRespawn_jmp_src, noRespawn_jmp_dest);
	}
	else
	{
		SafeWrite8(noRespawn_NOP, 0x75);  // JNZ
		SafeWrite8(noRespawn_NOP + 1, 0x03);
		SafeWrite8(noRespawn_jmp_src + 5, 0x90);  // NOP (original JNZ instruction is 6 bytes, our CALL/JMP only 5. fix required for call return)
		WriteRelCall(noRespawn_jmp_src, noRespawn_jmp_detour);
	}
}

void PatchGame(HINSTANCE& silverlock)
{
	// Loading FOSE
	silverlock = LoadLibrary("fose_1_7_vmp.dll");

	if (silverlock == NULL)
		DLLerror = true;
	else
	{
		// FOSE authors thought it was a smart move to prevent disabling ESC and console key
		unsigned int codebase = (DWORD) silverlock + 0x1000;
		unsigned char NOP[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

		SafeWriteBuf(codebase + 0x14222, NOP, sizeof(NOP));
		SafeWriteBuf(codebase + 0x14260, NOP, sizeof(NOP));
	}

	unsigned char NOP[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};

	FormLookup = (LookupForm) LOOKUP_FORM;
	FuncLookup = (LookupFunc) LOOKUP_FUNC;
	QueueMessage = (QueueUIMessage) QUEUE_UI_MESSAGE;

	SafeWrite8(delegator_dest, 0x51);   // PUSH ECX
	SafeWrite8(delegatorCall_src + 5, 0x59);   // POP ECX
	SafeWrite8(PlayGroup, 0xEB);   // JMP SHORT
	SafeWrite16(playIdle_fix_src + 5, 0x9090); // NOP NOP
	SafeWrite16(LockFix, 0x9090); // NOP NOP
	SafeWrite16(aiFix1, 0x9090); // NOP NOP
	SafeWrite8(aiFix2, 0x30); // redirect local jump to below

	unsigned char aiFix3_[] = {0x85, 0xFF, 0x74, 0xCC, 0xEB, 0xF6};
	SafeWriteBuf(aiFix3, aiFix3_, sizeof(aiFix3_));

	SafeWriteBuf(aiFix4, NOP, 11); // 11x NOP

	SafeWriteBuf(matchRace_NOP1, NOP, sizeof(NOP));
	SafeWriteBuf(matchRace_NOP2, NOP, 3);
	SafeWrite8(matchRace_patch + 1, 0xF1);
	SafeWriteBuf(matchRace_patch + 2, NOP, 4);
	SafeWrite8(matchRace_param, 0x0F);

	unsigned char jmp[] = {0xEB, 0x57, 0x90};
	unsigned char patch[] = {0x85, 0xED, 0x74, 0xE8, 0x8B, 0x55, 0x00, 0xEB, 0xA1};
	SafeWriteBuf(FireFix_jmp, jmp, sizeof(jmp));
	SafeWriteBuf(FireFix_patch, patch, sizeof(patch));

	WriteRelCall(delegatorCall_src, delegatorCall_dest);
	WriteRelCall(delegator_src, delegator_dest);
	WriteRelCall(playIdle_fix_src, playIdle_fix_dest);
	WriteRelJump(playIdle_call_src, playIdle_call_dest);
	WriteRelJump(AVFix_src, AVFix_dest);

	SafeWrite32(pluginsVMP, *(DWORD*)".vmp"); // redirect Plugins.txt

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
