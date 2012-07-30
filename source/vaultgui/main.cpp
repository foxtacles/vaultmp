#include <windows.h>
#include <iostream>
#include <fstream>
/*#include <detours.h>

#pragma comment(lib, "detours.lib")
*/
#include "Hook.h"
#include "HookedFunctions.h"


myIDirect3DDevice9* gl_pmyIDirect3DDevice9;
myIDirect3D9*       gl_pmyIDirect3D9;
HINSTANCE           gl_hOriginalDll;
HINSTANCE           gl_hThisInstance;
extern CRITICAL_SECTION cs_GetQueue;

void InitInstance(HANDLE hModule) ;
void ExitInstance() ;

#define DBB(a) 	/*std::ofstream d;d.open("C:\\Users\\PC\\Desktop\\debug.txt",std::ios::app);  d<<a<<std::endl;d.flush();d.close();*/
#define DB(a) 	/*std::ofstream d;d.open("C:\\Users\\PC\\Desktop\\debug.txt",std::ios::app);  d<<a<<std::endl;d.flush();d.close();*/



BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH: {
			DisableThreadLibraryCalls(hModule);
			InitializeCriticalSection(&cs_GetQueue);

			ResetLog();
			SendToLog("DLL Loaded");

			InitInstance(hModule);
			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"kernel32.dll","GetProcAddress",(PVOID)GetProcAddress_Hooked,(PVOID *)&GetProcAddress_Original)))
			{
				SendToLog("GetPRocAddress hook injected");
			}
			else
			{
				SendToLog("GetPRocAddress hook failed");
			}

			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"user32.dll","RegisterClassA",(PVOID)RegisterClass_Hooked,(PVOID *)&RegisterClass_Original)))
			{
				SendToLog("RegisterClass hook injected");
			}
			else
			{
				SendToLog("RegisterClass hook failed");
			}

	    case DLL_PROCESS_DETACH: ExitInstance(); break;
        
        case DLL_THREAD_ATTACH:  break;
	    case DLL_THREAD_DETACH:  break;
		}
	}
	return TRUE;
}

void InitInstance(HANDLE hModule) 
{
	
	// Initialisation
	gl_hOriginalDll        = NULL;
	gl_hThisInstance       = NULL;
	gl_pmyIDirect3D9       = NULL;
	gl_pmyIDirect3DDevice9 = NULL;	
	
	// Storing Instance handle into global var
	gl_hThisInstance = (HINSTANCE)  hModule;
}

void ExitInstance() 
{    
    
}