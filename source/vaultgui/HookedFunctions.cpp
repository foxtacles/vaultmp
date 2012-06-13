#include "HookedFunctions.h"

IDirect3D9* (WINAPI *Direct3DCreate9_Original)(UINT SDKVersion);
FARPROC (WINAPI *GetProcAddress_Original)(HMODULE hModule,LPCSTR lpProcName);
ATOM (WINAPI *RegisterClass_Original)(const WNDCLASS *lpWndClass);
LRESULT (CALLBACK *WindowProcedure_Original)(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

IDirect3D9* WINAPI Direct3DCreate9_Hooked(UINT SDKVersion)
{
	IDirect3D9* tmp;
	tmp=(*Direct3DCreate9_Original)(SDKVersion);
	gl_pmyIDirect3D9 = new myIDirect3D9(tmp);
	
	return gl_pmyIDirect3D9;
}

LONG myFunc(LPEXCEPTION_POINTERS p)
{
	//Exception handler
    return EXCEPTION_EXECUTE_HANDLER;
}

FARPROC WINAPI GetProcAddress_Hooked(HMODULE hModule,LPCSTR lpProcName)
{
		FARPROC tmp=GetProcAddress_Original(hModule,lpProcName);
		SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&myFunc);  
		DBB("GetProcAddress_Hooked("<<lpProcName<<")");
		if(strcmp(lpProcName,"Direct3DCreate9")==0)
		{
			Direct3DCreate9_Original=(IDirect3D9* (WINAPI *)(UINT SDKVersion))tmp;
			return (FARPROC)Direct3DCreate9_Hooked;
		}
        return tmp;
}

ATOM WINAPI RegisterClass_Hooked(
  __in  const WNDCLASS *lpWndClass
)
{
	WindowProcedure_Original=lpWndClass->lpfnWndProc;

	((WNDCLASS *)lpWndClass)->lpfnWndProc=CustomWindowProcedure;

	return RegisterClass_Original(lpWndClass);
}

LRESULT CALLBACK CustomWindowProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static string chatbox_text="";
	static bool chatting=false;

	switch(message)
	{	
		case WM_CHAR:
			switch((char)wparam)
			{
				case 0x08: 
                    
                    // Process a backspace. 
					if(chatting&&chatbox_text.length()>0)
					{
						chatbox_text.pop_back();
						gl_pmyIDirect3DDevice9->chatbox.SetWriting(chatbox_text);
					}
                     
                    break; 
 
                case 0x0A: 
                    
                    // Process a linefeed. 
                     
                    break; 
 
                case 0x1B: 
                    
                    // Process an escape. 
					gl_pmyIDirect3DDevice9->chatbox.SetWriting("");
					chatbox_text="";
					chatting=false;
                    
                    break; 
 
                case 0x09: 
                    
                    // Process a tab. 
                     
                    break; 
				
 
                case 0x0D: 
                    
                    // Process a carriage return. 
					if(chatting)
					{
						chatting=false;
						gl_pmyIDirect3DDevice9->chatbox.Lock();
						gl_pmyIDirect3DDevice9->chatbox.AddToQueue(chatbox_text);
						gl_pmyIDirect3DDevice9->chatbox.Unlock();
						chatbox_text="";
						gl_pmyIDirect3DDevice9->chatbox.SetWriting("");

						
					}
                     
                    break; 

				default:

				if(chatting)
				{
					if(chatbox_text.length()<50)
						chatbox_text+=(char)wparam;
					gl_pmyIDirect3DDevice9->chatbox.SetWriting(chatbox_text);
				}
				if(((char)wparam=='t'||(char)wparam=='T')&&!chatting)
				{
					chatting=!chatting;
				}
				
			}
			
		break;

		case WM_KEYDOWN:

			switch((char)wparam)
			{
				case VK_PRIOR:
					gl_pmyIDirect3DDevice9->chatbox.ScrollUp();
					break;

				case VK_NEXT:
					gl_pmyIDirect3DDevice9->chatbox.ScrollDown();
					break;
				case VK_ADD:
					gl_pmyIDirect3DDevice9->chatbox.SetSize(gl_pmyIDirect3DDevice9->chatbox.GetSize()+0.1);
					break;
				case VK_SUBTRACT:
					gl_pmyIDirect3DDevice9->chatbox.SetSize(gl_pmyIDirect3DDevice9->chatbox.GetSize()-0.1);
					break;
			}

			break;

	}
	return WindowProcedure_Original(hwnd,message,wparam,lparam);
}