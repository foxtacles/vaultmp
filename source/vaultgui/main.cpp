#include <windows.h>
#include <iostream>
#include <fstream>
/*#include <detours.h>

#pragma comment(lib, "detours.lib")
*/
#include "Hook.h"
#include "HookedFunctions.h"
#include "import.h"

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

			gl_pmyIDirect3DDevice9=0;

			DisableThreadLibraryCalls(hModule);
			InitializeCriticalSection(&cs_GetQueue);

			ResetLog();
			char tmp[200];
			sprintf(tmp,"DLL Loaded (0x%x)",(int)hModule);
			SendToLog(tmp);

			InitInstance(hModule);

			gData.sendClickCallbacks=true;
			gData.sendCheckboxCallbacks=true;
			gData.sendListboxCallbacks=true;

			/*TODO: Remove after testing*/
			gData.gameReady=true;
			
			/**/

			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"kernel32.dll","LoadLibraryA",(PVOID)LoadLibrary_Hooked,(PVOID *)&LoadLibrary_Original)))
			{
				SendToLog("LoadLibrary hook injected");
			}
			else
			{
				SendToLog("LoadLibrary hook failed");
			}

			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"kernel32.dll","GetProcAddress",(PVOID)GetProcAddress_Hooked,(PVOID *)&GetProcAddress_Original)))
			{
				SendToLog("GetPRocAddress hook injected");
			}
			else
			{
				SendToLog("GetPRocAddress hook failed");
			}

			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"DINPUT8.DLL","DirectInput8Create",(PVOID)DirectInput8Create_Hooked,(PVOID *)&DirectInput8Create_Original)))
			{
				SendToLog("DirectInput8Create hook injected");
			}
			else
			{
				SendToLog("DirectInput8Create hook failed");
			}

			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"user32.dll","RegisterClassA",(PVOID)RegisterClass_Hooked,(PVOID *)&RegisterClass_Original)))
			{
				SendToLog("RegisterClass hook injected");
			}
			else
			{
				SendToLog("RegisterClass hook failed");
			}

			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"kernel32.dll","CreateFileA",(PVOID)CreateFile_Hook,(PVOID *)&CreateFile_Original)))
			{
				SendToLog("CreateFile hook injected");
			}
			else
			{
				SendToLog("CreateFile hook failed");
			}
			
			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"kernel32.dll","ReadFile",(PVOID)ReadFile_Hook,(PVOID *)&ReadFile_Original)))
			{
				SendToLog("ReadFile hook injected");
			}
			else
			{
				SendToLog("ReadFile hook failed");
			}
			
			if(SUCCEEDED(PatchIat(GetModuleHandle(NULL),"d3dx9_38.dll","D3DXCreateTextureFromFileInMemory",(PVOID)D3DXCreateTextureFromFileInMemory_Hook,(PVOID *)&D3DXCreateTextureFromFileInMemory_Original)))
			{
				SendToLog("D3DXCreateTextureFromFileInMemory hook injected");
			}
			else
			{
				SendToLog("D3DXCreateTextureFromFileInMemory hook failed");
			}
			//F3
			if(*((unsigned char*)0x87BCA9)==0xE8)
			{
				loadTextureJmp=0x089E810;
				HookCall((BYTE*)0x87BCA9,(BYTE*)loadTextureHook,5);
				SendToLog("Texture Loading Hooked in Fallout 3");
				gData.textureHookingDone=true;
			}
			//FNV
			if(*((unsigned char*)0xE68BA9)==0xE8)
			{
				loadTextureJmp=0xAA1070;
				HookCall((BYTE*)0xE68BA9,(BYTE*)loadTextureHook,5);
				SendToLog("Texture Loading Hooked in Fallout NV");
				gData.textureHookingDone=true;
			}
			//HookCall((BYTE*)0x871E28,(BYTE*)playerPointerHook,6);
			SetupImports();

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