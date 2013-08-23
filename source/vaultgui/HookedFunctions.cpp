#include "HookedFunctions.h"

#include "myDirectInput8.h"

#include "TextureHooking.h"

#include "global.h"

#include "Export.h"

#include "import.h"
#include "GUIHelper.h"

IDirect3D9* (WINAPI *Direct3DCreate9_Original)(UINT SDKVersion);
HRESULT (WINAPI *DirectInput8Create_Original)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter);

HMODULE (WINAPI *LoadLibrary_Original)(LPCTSTR lpFileName);

FARPROC (WINAPI *GetProcAddress_Original)(HMODULE hModule,LPCSTR lpProcName);
ATOM (WINAPI *RegisterClass_Original)(const WNDCLASS *lpWndClass);
LRESULT (CALLBACK *WindowProcedure_Original)(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
HANDLE (WINAPI *CreateFile_Original)(LPCTSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);

BOOL (WINAPI *ReadFile_Original)(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped);
HFILE (WINAPI *OpenFile_Original)(LPCSTR lpFileName,LPOFSTRUCT lpReOpenBuff,UINT uStyle);

HRESULT (WINAPI *D3DXCreateTextureFromFileInMemory_Original)(LPDIRECT3DDEVICE9 pDevice,LPCVOID pSrcData,UINT SrcDataSize,LPDIRECT3DTEXTURE9 *ppTexture);

extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;

HRESULT WINAPI DirectInput8Create_Hooked(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	SendToLog("DirectInput8Create_Hooked called");
	HRESULT ret=DirectInput8Create_Original(hinst,dwVersion,riidltf,ppvOut,punkOuter);

	(*ppvOut)=new MyDirectInput8((IDirectInput8*)*ppvOut);

	return ret;
}

IDirect3D9* WINAPI Direct3DCreate9_Hooked(UINT SDKVersion)
{
	SendToLog("Direct3DCreate9_Hooked called");
	IDirect3D9* tmp;
	tmp=(*Direct3DCreate9_Original)(SDKVersion);
	gl_pmyIDirect3D9 = new myIDirect3D9(tmp);
	
	SendToLog("Returning DX Pointer");

	return gl_pmyIDirect3D9;
}

LONG OnApplicationCrash(LPEXCEPTION_POINTERS p)
{
	//Exception handler
	//char* exstr=ExceptionToString(p->ExceptionRecord);
	SendToLog("EXCEPTION!");
	SendToLog(ExceptionToString(p->ExceptionRecord));

	Debug::InspectCrash(p);

	STARTUPINFO info={sizeof(info)};
	PROCESS_INFORMATION processInfo;
	if (CreateProcess("SendFalloutLog.exe", "", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
	{
		/*::WaitForSingleObject(processInfo.hProcess, INFINITE);*/
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}

	//system("SendFalloutLog.exe");
	//Got an exception!
    return EXCEPTION_EXECUTE_HANDLER;
}

HMODULE WINAPI LoadLibrary_Hooked(LPCTSTR lpFileName)
{
	string out="LoadLibrary_Hooked(";
	out+=lpFileName+string(")");
	SendToLog((char*)out.c_str());
	return LoadLibrary_Original(lpFileName);
}

FARPROC WINAPI GetProcAddress_Hooked(HMODULE hModule,LPCSTR lpProcName)
{
		string out="GetProcAddress_Hooked(";
		out+=lpProcName+string(")");
		SendToLog((char*)out.c_str());

		FARPROC tmp=GetProcAddress_Original(hModule,lpProcName);
		SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&OnApplicationCrash);  
		DBB("GetProcAddress_Hooked("<<lpProcName<<")");
		if(strcmp(lpProcName,"Direct3DCreate9")==0)
		{
			Direct3DCreate9_Original=(IDirect3D9* (WINAPI *)(UINT SDKVersion))tmp;
			SendToLog("Returning fake Direct3DCreate9 (hooked function)");
			return (FARPROC)Direct3DCreate9_Hooked;
		}
        return tmp;
}

ATOM WINAPI RegisterClass_Hooked(
  __in  const WNDCLASS *lpWndClass
)
{
	SendToLog("RegisterClass_Hooked called");
	WindowProcedure_Original=lpWndClass->lpfnWndProc;

	((WNDCLASS *)lpWndClass)->lpfnWndProc=CustomWindowProcedure;

	SendToLog("Returning RegisterClass_Original");
	return RegisterClass_Original(lpWndClass);
}

LRESULT CALLBACK CustomWindowProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	static string chatbox_text="";
	/*static bool chatting=false;*/
	static int maxV=0;
	static char buff[100];

	SendToLog("CustomWindowProcedure called");

	switch(message)
	{	

/*		case WM_MOUSEMOVE:
			if(chatting)
			{

				int xPos = GET_X_LPARAM(lparam); 
				int yPos = GET_Y_LPARAM(lparam);
				CEGUI::System::getSingleton().injectMousePosition(xPos,yPos);

			}
			gl_pmyIDirect3DDevice9->wnd->setText("Mouse move!");
		break;


		case WM_LBUTTONDOWN:
			if(chatting)
			{

				int xPos = GET_X_LPARAM(lparam); 
				int yPos = GET_Y_LPARAM(lparam);
				CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MouseButton::LeftButton);

			}
		break;

		case WM_LBUTTONUP:
			if(chatting)
			{
				#ifdef USE_CEGUI
				int xPos = GET_X_LPARAM(lparam); 
				int yPos = GET_Y_LPARAM(lparam);
				CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::MouseButton::LeftButton);
#endif
			}
		break;

		case WM_LBUTTONDBLCLK:
			if(chatting)
			{
				#ifdef USE_CEGUI
				int xPos = GET_X_LPARAM(lparam); 
				int yPos = GET_Y_LPARAM(lparam);
				CEGUI::System::getSingleton().injectMouseButtonDoubleClick(CEGUI::MouseButton::LeftButton);
#endif
			}
		break;*/

		case WM_CHAR:
			switch((char)wparam)
			{
				case 0x08: 
                    
                    // Process a backspace. 
					/*if(chatting&&chatbox_text.length()>0)
					{
						chatbox_text.pop_back();
					}*/
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Backspace);
                     
                    break; 

				case VK_DELETE: 
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Delete);
					break;
 
                case 0x0A: 
                    
                    // Process a linefeed. 
                     
                    break; 
 
                case 0x1B: 
                    
                    // Process an escape. 
					if(GUI_OnMode&&gData.chatting)
						GUI_OnMode(false);
					chatbox_text="";
					gData.chatting=false;
					gData.disableMouseInput=false;
					CEGUI::MouseCursor::getSingleton().hide();
					((CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow("closeBTN"))->setAlpha(0);
					

                    
                    break; 
 
                case 0x09: 
                    
                    // Process a tab. 
                     
                    break; 
				
 
                case 0x0D: 
                    
                    // Process a carriage return. 
					if(gData.chatting)
					{
						
						CEGUI::Editbox* edb = ((CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow("Edit Box"));

						if(edb->hasInputFocus())
						{

							CEGUI::String txt=edb->getText();
							edb->setText("");

							if(GUI_OnChat)
								GUI_OnChat(txt.c_str());
							else
							chatQueue.push_back((char*)txt.c_str());
							//Chatbox_AddToChat((char*)txt.c_str());
							GUIHelper::chatboxHistory_Add(txt.c_str());

							chatbox_text="";
							((CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow("closeBTN"))->setAlpha(0);
							gData.chatting=false;
							gData.disableMouseInput=false;
							CEGUI::MouseCursor::getSingleton().hide();
							gData.lastChatTextTick=GetTickCount();
							if(GUI_OnMode)
								GUI_OnMode(false);
						}
					}
                     
                    break; 

				default:

				if(gData.chatting)
				{
					char t=(char)wparam;
					if(t>=32)
					{
						CEGUI::System::getSingleton().injectChar(t);
					}
				}

				if(((char)wparam=='b'||(char)wparam=='B')&&!gData.chatting)
				{

				}

				if(((char)wparam=='t'||(char)wparam=='T')&&!gData.chatting)
				{
					gData.chatting=!gData.chatting;
					gData.chatting=true;
					gData.disableMouseInput=true;
					CEGUI::MouseCursor::getSingleton().show();
					CEGUI::Editbox* edb = ((CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow("Edit Box"));

					((CEGUI::Editbox*)CEGUI::WindowManager::getSingleton().getWindow("closeBTN"))->setAlpha(1);

					edb->activate();
					if(GUI_OnMode)
						GUI_OnMode(true);
				}
				
			}
			
		break;

		case WM_KEYDOWN:

			switch((char)wparam)
			{
				case VK_ADD:
					
					break;
				case VK_SUBTRACT:
					
					break;

				case VK_DELETE:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Delete);
					break;

				case VK_LEFT:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::ArrowLeft);
					break;

				case VK_RIGHT:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::ArrowRight);
					break;

				case VK_DOWN:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::ArrowDown);
					break;

				case VK_UP:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::ArrowUp);
					break;

				case VK_TAB:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Tab);
					break;

				case VK_CAPITAL:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Capital);
					break;

				case VK_CONTROL:
					if(gData.chatting)
					{
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::LeftControl);
					}
					break;

				case VK_SHIFT:
					if(gData.chatting)
					{
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::LeftShift);
					}
					break;

				case VK_RETURN:
					if(gData.chatting)
					{
						CEGUI::System::getSingleton().injectKeyDown(CEGUI::Key::Return);
					}
					break;
			}

			break;
		
		case WM_KEYUP:

			switch((char)wparam)
			{
				case VK_ADD:
					
					break;
				case VK_SUBTRACT:
					
					break;

				case VK_DELETE:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyUp(CEGUI::Key::Delete);
					break;

				case VK_LEFT:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyUp(CEGUI::Key::ArrowLeft);
					break;

				case VK_RIGHT:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyUp(CEGUI::Key::ArrowRight);
					break;

				case VK_DOWN:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyUp(CEGUI::Key::ArrowDown);
					break;

				case VK_UP:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyUp(CEGUI::Key::ArrowUp);
					break;

				case VK_TAB:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyUp(CEGUI::Key::Tab);
					break;

				case VK_CAPITAL:
					if(gData.chatting)
						CEGUI::System::getSingleton().injectKeyUp(CEGUI::Key::Capital);
					break;

				case VK_CONTROL:
					if(gData.chatting)
					{
						CEGUI::System::getSingleton().injectKeyUp(CEGUI::Key::LeftControl);
					}
					break;

				case VK_SHIFT:
					if(gData.chatting)
					{
						CEGUI::System::getSingleton().injectKeyUp(CEGUI::Key::LeftShift);
					}
					break;

				case VK_RETURN:
					if(gData.chatting)
					{
						CEGUI::System::getSingleton().injectKeyUp(CEGUI::Key::Return);
					}
					break;
			}

			break;

	}
	return WindowProcedure_Original(hwnd,message,wparam,lparam);
}

BOOL WINAPI ReadFile_Hook(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
	return ReadFile_Original(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
}

HFILE WINAPI OpenFile_Hook(LPCSTR lpFileName,LPOFSTRUCT lpReOpenBuff,UINT uStyle)
{
	return OpenFile_Original(lpFileName,lpReOpenBuff,uStyle);
}

HANDLE WINAPI CreateFile_Hook(LPCTSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	return CreateFile_Original(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
}

HRESULT WINAPI D3DXCreateTextureFromFileInMemory_Hook(LPDIRECT3DDEVICE9 pDevice,LPCVOID pSrcData,UINT SrcDataSize,LPDIRECT3DTEXTURE9 *ppTexture)
{
	char* lastTextureLoadedBackup=lastTextureLoaded;
	char tmp[300];
	char *tmp2=(char*)pSrcData;
	sprintf(tmp,"0x%x D3DXCreateTextureFromFileInMemory %s",_ReturnAddress(),lastTextureLoadedBackup);
	SendToLog(tmp);

	if(lastTextureLoadedBackup!=0)
	{
		if(gData.textureHookingDone)
			TextureHooking::hookTextureIfNecessary((char*)lastTextureLoadedBackup,(char**)&pSrcData,(int*)&SrcDataSize);
	}

	HRESULT ret=D3DXCreateTextureFromFileInMemory_Original(pDevice,pSrcData,SrcDataSize,ppTexture);
	if(lastTextureLoadedBackup!=0)
		TextureHooking::registerTexture((char*)lastTextureLoadedBackup,*ppTexture);

	
	/*unsigned int textureHash=createHash((const char*)pSrcData,SrcDataSize);
	sprintf(tmp,"textureDump/%u.png",textureHash);
	D3DXSaveTextureToFile(tmp,D3DXIFF_PNG,(*ppTexture),NULL);*/


	return ret;
}

char* lastTextureLoaded=0;
int loadTextureJmp=0xAA1070;

__declspec(naked) void loadTextureHook()
{
	_asm
	{
		mov lastTextureLoaded,ebx
		jmp loadTextureJmp
	}
}

/*Some mess with player pointer data (FNV)*/
/*
int playerPointer=0;
int playerPointerJmp=0x871E2E;

__declspec(naked) void playerPointerHook()
{
	_asm
	{
		mov playerPointer,ecx
		mov eax,[edx+0x000001d0]
		jmp playerPointerJmp
	}
}*/