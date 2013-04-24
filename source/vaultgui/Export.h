#include <iostream>

using namespace std;

extern "C"
{
	__declspec(dllexport) void Chatbox_AddToChat(char* c);

	__declspec(dllexport) char* Chatbox_GetQueue();

	__declspec(dllexport) void Chatbox_AddPlayerName(string name,int* x,int* y,int* z);

	__declspec(dllexport) void Chatbox_DeletePlayerName(string name);

	__declspec(dllexport) void SetPlayersDataPointer(void* p);

	__declspec(dllexport) void HideChatbox(bool hide);

	__declspec(dllexport) void LockChatbox(bool lock);

	__declspec(dllexport) void SetChatboxPos(float x,float y);

	__declspec(dllexport) void SetChatboxSize(float x,float y);




	__declspec(dllexport) void GUI_CreateFrameWindow(char *name);

	__declspec(dllexport) void GUI_SetFrameWindowPosition(char* name,float x,float y);

	__declspec(dllexport) void GUI_SetFrameWindowSize(char* name,float x,float y);

	__declspec(dllexport) void GUI_AddStaticText(char* parent,char* name);

	__declspec(dllexport) void GUI_SetStaticTextPosition(char* name,float x,float y);

	__declspec(dllexport) void GUI_SetStaticTextSize(char* name,float x,float y);

	__declspec(dllexport) void GUI_SetStaticTextText(char* name,char* txt);
}