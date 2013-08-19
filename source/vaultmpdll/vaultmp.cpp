#include <winsock2.h>
#include <cstdio>
#include <vector>
#include <string>
#include <mutex>
#include <queue>
#include <cmath>

#include "vaultmp.h"

using namespace std;

struct remotePlayers
{
    char name[64];
    bool player;
    double health;
    double pos[3];
    double rot[3];
    long int pid;
};

static remotePlayers players[10];

typedef void (*CallCommand)(void*, void*, void*, void*, void*, void*, void*, void*);

mutex mInput;
queue<string> qGUI_OnChat;
queue<bool> qGUI_OnMode;
queue<string> qGUI_OnClick;
queue<pair<string, string>> qGUI_OnText;
queue<pair<string, bool>> qGUI_OnCheckbox;
queue<unsigned int> qActivate;

static HANDLE hProc;
static PipeServer pipeServer;
static PipeClient pipeClient;
static unsigned int (*FormLookup)(unsigned int);
static unsigned int (*FuncLookup)(unsigned int);
static void (*Chatbox_AddToChat)(const char*);
static void (*GUI_CreateFrameWindow)(const char*);
static void (*GUI_AddStaticText)(const char*, const char*);
static void (*GUI_AddTextbox)(const char*, const char*);
static void (*GUI_AddButton)(const char*, const char*);
static void (*GUI_SetVisible)(const char*, bool);
static void (*GUI_AllowDrag)(const char*, bool);
static void (*GUI_SetPosition)(const char*, float, float, float, float);
static void (*GUI_SetSize)(const char*, float, float, float, float);
static void (*GUI_SetText)(const char*, const char*);
static void (*GUI_RemoveWindow)(const char*);
static void (*GUI_ForceGUI)(bool);
static void (*GUI_SetClickCallback)(void (*)(const char*));
static void (*GUI_SetTextChangedCallback)(void (*)(const char*, const char*));
static void (*GUI_Textbox_SetMaxLength)(const char*, unsigned int);
static void (*GUI_Textbox_SetValidationString)(const char*, const char*);
static void (*GUI_AddCheckbox)(const char*, const char*);
static void (*GUI_SetChecked)(const char*, bool);
static void (*GUI_SetCheckboxChangedCallback)(void (*)(const char*, bool));
static void (*SetPlayersDataPointer)(remotePlayers*);
static bool (*QueueUIMessage)(const char* msg, unsigned int emotion, const char* ddsPath, const char* soundName, float msgTime);

static void PatchGame(HINSTANCE& silverlock);
static void BethesdaDelegator();
static void ToggleRespawn();
static void RespawnDetour();
static void AnimDetour();
static void PlayIdleDetour();
static void AVFix();
static void GetActivate();
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
static const unsigned playGroup_fix = 0x0049DD6A;
static const unsigned playGroup_fix_src = 0x0049DD8E;
static const unsigned playGroup_fix_dest = 0x0049DCF1;
static unsigned AVFix_src = 0x00473D35;
static unsigned AVFix_dest = (unsigned)& AVFix;
static unsigned AVFix_ret = 0x00473D3B;
static unsigned AVFix_term = 0x00473E85;
static unsigned FireFix_jmp = 0x0079236C;
static unsigned FireFix_patch = 0x007923C5;
static unsigned GetActivate_jmp = 0x0078A68D;
static unsigned GetActivate_dest = (unsigned)& GetActivate;;
static unsigned GetActivate_ret = 0x0078A995;

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

void GetActivate()
{
	void* object;

	asm volatile(
		"PUSHAD\n"
		"MOV %0,EAX\n"
		: "=m"(object)
		:
		: "eax"
	);

	if (object)
	{
		unsigned int refID = *(unsigned int*)(((unsigned) object) + 0x0C);

		mInput.lock();
		qActivate.push(refID);
		mInput.unlock();
	}

	asm volatile(
		"POPAD\n"
		:
		:
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

				memcpy(result, &idle, 4);
				memcpy((void*)((unsigned) result + 4), &moving, 1);
				memcpy((void*)((unsigned) result + 5), &flags, 1);
				memcpy((void*)((unsigned) result + 6), &weapon, 1);
				memcpy((void*)((unsigned) result + 7), &sneaking, 1);
			}

			break;
		}

		case 0x0003 | VAULTFUNCTION: // GetPosAngle - Get pos and angle
		{
			ZeroMemory(result, sizeof(double));

			if (!reference)
				return false;

			*(unsigned int*) result = sizeof(float) * 6;
			float* data = (float*) ((unsigned) result + 4);

			data[0] = (*(float*)((unsigned char*) reference + 0x20)) * 180 / M_PI;
			data[1] = (*(float*)((unsigned char*) reference + 0x24)) * 180 / M_PI;
			data[2] = (*(float*)((unsigned char*) reference + 0x28)) * 180 / M_PI;
			data[3] = *(float*)((unsigned char*) reference + 0x2C);
			data[4] = *(float*)((unsigned char*) reference + 0x30);
			data[5] = *(float*)((unsigned char*) reference + 0x34);
			return true;
		}

		case 0x0004 | VAULTFUNCTION: // UIMessage - Queue UI message
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length

			unsigned int emoticon = *(unsigned int*)(data + strlen(data) + 2);

			QueueUIMessage(data, emoticon, NULL, NULL, 2.0); // add more later

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

		case 0x0008 | VAULTFUNCTION: // GUIChat - Print chat message
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			Chatbox_AddToChat(data);
			break;
		}

		case 0x0009 | VAULTFUNCTION: // GUIMode - Force in / out of GUI mode
		{
			ZeroMemory(result, sizeof(double));
			unsigned char* _args = (unsigned char*) args;

			if (*_args == 0x6E)
				GUI_ForceGUI(*(unsigned int*) (_args + 1));
			break;
		}

		case 0x0010 | VAULTFUNCTION: // GUICreateWindow - Create new frame window
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			GUI_CreateFrameWindow(data);
			break;
		}

		case 0x0011 | VAULTFUNCTION: // GUICreateButton - Create new button
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			GUI_AddButton(data, data + strlen(data) + 3);
			break;
		}

		case 0x0012 | VAULTFUNCTION: // GUICreateText - Create new text
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			GUI_AddStaticText(data, data + strlen(data) + 3);
			break;
		}

		case 0x0013 | VAULTFUNCTION: // GUICreateEdit - Create new edit box
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			GUI_AddTextbox(data, data + strlen(data) + 3);
			break;
		}

		case 0x0014 | VAULTFUNCTION: // GUIRemoveWindow - Remove window
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			GUI_RemoveWindow(data);
			break;
		}

		case 0x0015 | VAULTFUNCTION: // GUIPos - Update window
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			unsigned char* _args = (unsigned char*) (data + strlen(data) + 1);

			float pos_X = *(double*)(_args + 1);
			float pos_Y = *(double*)(_args + 10);
			float offset_X = *(double*)(_args + 19);
			float offset_Y = *(double*)(_args + 28);

			GUI_SetPosition(data, pos_X, pos_Y, offset_X, offset_Y);
			break;
		}

		case 0x0016 | VAULTFUNCTION: // GUISize - Update window
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			unsigned char* _args = (unsigned char*) (data + strlen(data) + 1);

			float size_X = *(double*)(_args + 1);
			float size_Y = *(double*)(_args + 10);
			float offset_X = *(double*)(_args + 19);
			float offset_Y = *(double*)(_args + 28);

			GUI_SetSize(data, size_X, size_Y, offset_X, offset_Y);
			break;
		}

		case 0x0017 | VAULTFUNCTION: // GUIVisible - Update window
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			unsigned char* _args = (unsigned char*) (data + strlen(data) + 1);

			bool visible = (bool) *(unsigned int*)(_args + 1);

			GUI_SetVisible(data, visible);
			break;
		}

		case 0x0018 | VAULTFUNCTION: // GUILocked - Update window
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			unsigned char* _args = (unsigned char*) (data + strlen(data) + 1);

			bool locked = (bool) *(unsigned int*)(_args + 1);

			GUI_AllowDrag(data, !locked);
			break;
		}

		case 0x0019 | VAULTFUNCTION: // GUIText - Update text
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			GUI_SetText(data, data + strlen(data) + 3);
			break;
		}

		case 0x0021 | VAULTFUNCTION: // GUIMaxLen - Update max length
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			unsigned char* _args = (unsigned char*) (data + strlen(data) + 1);

			unsigned int length = *(unsigned int*)(_args + 1);

			GUI_Textbox_SetMaxLength(data, length);
			break;
		}

		case 0x0022 | VAULTFUNCTION: // GUIValid - Update validation
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			GUI_Textbox_SetValidationString(data, data + strlen(data) + 3);
			break;
		}

		case 0x0023 | VAULTFUNCTION: // GUICreateCheckbox - Create checkbox
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			GUI_AddCheckbox(data, data + strlen(data) + 3);
			break;
		}

		case 0x0024 | VAULTFUNCTION: // GUICheckbox - Update checkbox
		{
			ZeroMemory(result, sizeof(double));
			const char* data = ((char*) args) + 2; // skip length
			unsigned char* _args = (unsigned char*) (data + strlen(data) + 1);

			bool selected = (bool) *(unsigned int*)(_args + 1);

			GUI_SetChecked(data, selected);
			break;
		}

		default:
			break;
	}

	return false;
}

void GUI_OnClick(const char* name)
{
	mInput.lock();
	qGUI_OnClick.push(name);
	mInput.unlock();
}

void GUI_OnText(const char* name, const char* text)
{
	mInput.lock();
	qGUI_OnText.emplace(name, text);
	mInput.unlock();
}

void GUI_OnCheckbox(const char* name, bool selected)
{
	mInput.lock();
	qGUI_OnCheckbox.emplace(name, selected);
	mInput.unlock();
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

	unsigned char result[PIPE_LENGTH];
	ZeroMemory(result, sizeof(result));

	*((unsigned int*)((unsigned) result + 1)) = r;

	bool bigresult = false;

	if ((opcode & VAULTFUNCTION) == VAULTFUNCTION)
		bigresult = vaultfunction((void*) reference, result + 5, _args, opcode);
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

	if (!bigresult)
		result[0] = PIPE_OP_RETURN;
	else
		result[0] = PIPE_OP_RETURN_BIG;

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

		Chatbox_AddToChat = reinterpret_cast<decltype(Chatbox_AddToChat)>(GetProcAddress(vaultgui, "Chatbox_AddToChat"));
		GUI_CreateFrameWindow = reinterpret_cast<decltype(GUI_CreateFrameWindow)>(GetProcAddress(vaultgui, "GUI_CreateFrameWindow"));
		GUI_AddStaticText = reinterpret_cast<decltype(GUI_AddStaticText)>(GetProcAddress(vaultgui, "GUI_AddStaticText"));
		GUI_AddTextbox = reinterpret_cast<decltype(GUI_AddTextbox)>(GetProcAddress(vaultgui, "GUI_AddTextbox"));
		GUI_AddButton = reinterpret_cast<decltype(GUI_AddButton)>(GetProcAddress(vaultgui, "GUI_AddButton"));
		GUI_SetVisible = reinterpret_cast<decltype(GUI_SetVisible)>(GetProcAddress(vaultgui, "GUI_SetVisible"));
		GUI_AllowDrag = reinterpret_cast<decltype(GUI_AllowDrag)>(GetProcAddress(vaultgui, "GUI_AllowDrag"));
		GUI_SetPosition = reinterpret_cast<decltype(GUI_SetPosition)>(GetProcAddress(vaultgui, "GUI_SetPosition"));
		GUI_SetSize = reinterpret_cast<decltype(GUI_SetSize)>(GetProcAddress(vaultgui, "GUI_SetSize"));
		GUI_SetText = reinterpret_cast<decltype(GUI_SetText)>(GetProcAddress(vaultgui, "GUI_SetText"));
		GUI_RemoveWindow = reinterpret_cast<decltype(GUI_RemoveWindow)>(GetProcAddress(vaultgui, "GUI_RemoveWindow"));
		GUI_ForceGUI = reinterpret_cast<decltype(GUI_ForceGUI)>(GetProcAddress(vaultgui, "GUI_ForceGUI"));
		GUI_SetClickCallback = reinterpret_cast<decltype(GUI_SetClickCallback)>(GetProcAddress(vaultgui, "GUI_SetClickCallback"));
		GUI_SetTextChangedCallback = reinterpret_cast<decltype(GUI_SetTextChangedCallback)>(GetProcAddress(vaultgui, "GUI_SetTextChangedCallback"));
		GUI_Textbox_SetMaxLength = reinterpret_cast<decltype(GUI_Textbox_SetMaxLength)>(GetProcAddress(vaultgui, "GUI_Textbox_SetMaxLength"));
		GUI_Textbox_SetValidationString = reinterpret_cast<decltype(GUI_Textbox_SetValidationString)>(GetProcAddress(vaultgui, "GUI_Textbox_SetValidationString"));
		GUI_AddCheckbox = reinterpret_cast<decltype(GUI_AddCheckbox)>(GetProcAddress(vaultgui, "GUI_AddCheckbox"));
		GUI_SetChecked = reinterpret_cast<decltype(GUI_SetChecked)>(GetProcAddress(vaultgui, "GUI_SetChecked"));
		GUI_SetCheckboxChangedCallback = reinterpret_cast<decltype(GUI_SetCheckboxChangedCallback)>(GetProcAddress(vaultgui, "GUI_SetCheckboxChangedCallback"));
		SetPlayersDataPointer = reinterpret_cast<decltype(SetPlayersDataPointer)>(GetProcAddress(vaultgui, "SetPlayersDataPointer"));

		if (!Chatbox_AddToChat || !GUI_CreateFrameWindow || !GUI_AddStaticText || !GUI_AddTextbox || !GUI_AddButton || !GUI_SetVisible || !GUI_AllowDrag || !GUI_SetPosition || !GUI_SetSize || !GUI_SetText || !GUI_RemoveWindow || !GUI_ForceGUI || !GUI_SetClickCallback || !GUI_SetTextChangedCallback || !GUI_Textbox_SetMaxLength || !GUI_Textbox_SetValidationString || !SetPlayersDataPointer || !GUI_AddCheckbox || !GUI_SetChecked || !GUI_SetCheckboxChangedCallback)
			DLLerror = true;

		GUI_SetClickCallback(GUI_OnClick);
		GUI_SetTextChangedCallback(GUI_OnText);
		GUI_SetCheckboxChangedCallback(GUI_OnCheckbox);

/*
players[0].health = 80.0;
sprintf(players[0].name, "asdf");
players[0].pos[0] = 995.0;
players[0].pos[1] = 7157.0;
players[0].pos[2] = 6802.0;
players[0].pid = 1;
players[0].player = true;


players[1].health = 80.0;
sprintf(players[1].name, "asdf123");
players[1].pos[0] = 985.0;
players[1].pos[1] = 7357.0;
players[1].pos[2] = 6802.0;
players[1].pid = 2;
players[1].player = false;

		SetPlayersDataPointer(players);

*/
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
		if (!pipeServer.Receive(buffer))
			continue;

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

		mInput.lock();

		while (!qGUI_OnChat.empty())
		{
			const string& chat = qGUI_OnChat.front();

			buffer[0] = PIPE_OP_RETURN_RAW;
			*reinterpret_cast<unsigned int*>(buffer + 1) = 0x0008 | VAULTFUNCTION;
			*reinterpret_cast<unsigned int*>(buffer + 5) = chat.length();
			memcpy(buffer + 9, chat.c_str(), chat.length());

			pipeClient.Send(buffer);

			qGUI_OnChat.pop();
		}

		while (!qGUI_OnMode.empty())
		{
			bool mode = qGUI_OnMode.front();

			buffer[0] = PIPE_OP_RETURN_RAW;
			*reinterpret_cast<unsigned int*>(buffer + 1) = 0x0009 | VAULTFUNCTION;
			*reinterpret_cast<unsigned int*>(buffer + 5) = 1;
			*reinterpret_cast<bool*>(buffer + 9) = mode;

			pipeClient.Send(buffer);

			qGUI_OnMode.pop();
		}

		while (!qGUI_OnClick.empty())
		{
			const string& name = qGUI_OnClick.front();

			buffer[0] = PIPE_OP_RETURN_RAW;
			*reinterpret_cast<unsigned int*>(buffer + 1) = 0x0020 | VAULTFUNCTION;
			*reinterpret_cast<unsigned int*>(buffer + 5) = name.length();
			memcpy(buffer + 9, name.c_str(), name.length());

			pipeClient.Send(buffer);

			qGUI_OnClick.pop();
		}

		while (!qGUI_OnText.empty())
		{
			const auto& text = qGUI_OnText.front();

			buffer[0] = PIPE_OP_RETURN_RAW;
			*reinterpret_cast<unsigned int*>(buffer + 1) = 0x0019 | VAULTFUNCTION;
			*reinterpret_cast<unsigned int*>(buffer + 5) = text.first.length() + text.second.length() + 2;
			memcpy(buffer + 9, text.first.c_str(), text.first.length() + 1);
			memcpy(buffer + 9 + text.first.length() + 1, text.second.c_str(), text.second.length() + 1);

			pipeClient.Send(buffer);

			qGUI_OnText.pop();
		}

		while (!qGUI_OnCheckbox.empty())
		{
			const auto& checkbox = qGUI_OnCheckbox.front();

			buffer[0] = PIPE_OP_RETURN_RAW;
			*reinterpret_cast<unsigned int*>(buffer + 1) = 0x0024 | VAULTFUNCTION;
			*reinterpret_cast<unsigned int*>(buffer + 5) = checkbox.first.length() + sizeof(bool) + 1;
			memcpy(buffer + 9, checkbox.first.c_str(), checkbox.first.length() + 1);
			memcpy(buffer + 9 + checkbox.first.length() + 1, &checkbox.second, sizeof(bool));

			pipeClient.Send(buffer);

			qGUI_OnCheckbox.pop();
		}

		while (!qActivate.empty())
		{
			unsigned int refID = qActivate.front();

			buffer[0] = PIPE_OP_RETURN_RAW;
			*reinterpret_cast<unsigned int*>(buffer + 1) = 0x0002 | VAULTFUNCTION;
			*reinterpret_cast<unsigned int*>(buffer + 5) = sizeof(refID);
			*reinterpret_cast<unsigned int*>(buffer + 9) = refID;

			pipeClient.Send(buffer);

			qActivate.pop();
		}

		mInput.unlock();
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
		respawn = true;

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

	FormLookup = (decltype(FormLookup)) LOOKUP_FORM;
	FuncLookup = (decltype(FuncLookup)) LOOKUP_FUNC;
	QueueUIMessage = (decltype(QueueUIMessage)) QUEUE_UI_MESSAGE;

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

	unsigned char playGroup_fix_A[] = {0x85, 0xC9, 0x0F, 0x84, 0xFF, 0x00, 0x00, 0x00, 0x8B, 0x71, 0x0C, 0x85, 0xF6, 0xEB, 0x6A};
	unsigned char playGroup_fix_B[] = {0xEB, 0x27};
	SafeWriteBuf(playGroup_fix_dest, playGroup_fix_A, sizeof(playGroup_fix_A));
	SafeWriteBuf(playGroup_fix, playGroup_fix_B, sizeof(playGroup_fix_B));
	WriteRelJump(playGroup_fix_src, playGroup_fix_dest);

	WriteRelCall(GetActivate_jmp, GetActivate_dest);
	WriteRelJump(GetActivate_jmp + 5, GetActivate_ret);

	SafeWrite32(pluginsVMP, *(DWORD*)".vmp"); // redirect Plugins.txt

	ToggleRespawn();
}

extern "C"
{
	void GUI_OnMode(bool enabled)
	{
		mInput.lock();
		qGUI_OnMode.push(enabled);
		mInput.unlock();
	}

	void GUI_OnChat(const char* message)
	{
		mInput.lock();
		qGUI_OnChat.push(message);
		mInput.unlock();
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
