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

#define DB(a) 	std::ofstream d;d.open("C:\\Users\\PC\\Desktop\\debug.txt",std::ios::app);  d<<a<<std::endl;d.flush();d.close();

int (__cdecl *sub_5B7660)(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)=(int (__cdecl *)(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8))0x5B7660;

int __cdecl HookedFW(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
	{DB("FW("<<std::hex<<"0x"<<a1<<",0x"<<a2<<",0x"<<a3<<",0x"<<a4<<",0x"<<a5<<",0x"<<a6<<",0x"<<a7<<",0x"<<a8<<")");}
	int tmp=sub_5B7660(a1,a2,a3,a4,a5,a6,a7,a8);
	

	return tmp;
}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH: {
			DisableThreadLibraryCalls(hModule);
			InitializeCriticalSection(&cs_GetQueue);
			InitInstance(hModule);
			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"kernel32.dll","GetProcAddress",(PVOID)GetProcAddress_Hooked,(PVOID *)&GetProcAddress_Original)))
			{
				DBB("GetPRocAddress hook injected");
			}
			else
			{
				DBB("GetPRocAddress hook failed");
			}

			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"user32.dll","RegisterClassA",(PVOID)RegisterClass_Hooked,(PVOID *)&RegisterClass_Original)))
			{
				DBB("RegisterClass hook injected");
			}
			else
			{
				DBB("RegisterClass hook failed");
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