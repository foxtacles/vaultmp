#include <windows.h>
#include <WinBase.h>
#include "myIDirect3DDevice9.h"
#include <queue>

extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

CRITICAL_SECTION cs_GetQueue;


extern "C"
{
	__declspec(dllexport) void Chatbox_AddToChat(char* c)
	{
		gl_pmyIDirect3DDevice9->chatbox.AddLine(c);
	}

	__declspec(dllexport) char* Chatbox_GetQueue()
	{
		static char buffer[200]={0};
		string tmp="";

		EnterCriticalSection(&cs_GetQueue);

		if((gl_pmyIDirect3DDevice9->chatbox.q).size()>0)
		{
			tmp=(gl_pmyIDirect3DDevice9->chatbox.q).back();
			(gl_pmyIDirect3DDevice9->chatbox.q).pop();
		}
		buffer[0]=0;
		if(tmp.length()>0)
			strcpy(buffer,tmp.c_str());

		LeaveCriticalSection(&cs_GetQueue);

		return buffer;
	}
}