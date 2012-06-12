#include <windows.h>
#include <d3d9.h>
#include "myIDirect3D9.h"
#include "myIDirect3DDevice9.h"
#include <fstream>

extern myIDirect3D9*       gl_pmyIDirect3D9;
extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;
extern HINSTANCE           gl_hOriginalDll;
extern HINSTANCE           gl_hThisInstance;

#define DBB(a) 	/*std::ofstream d;d.open("C:\\Users\\PC\\Desktop\\debug.txt",std::ios::app);  d<<a<<std::endl;d.flush();d.close();*/

extern IDirect3D9* (WINAPI *Direct3DCreate9_Original)(UINT SDKVersion);
extern FARPROC (WINAPI *GetProcAddress_Original)(HMODULE hModule,LPCSTR lpProcName);
extern ATOM (WINAPI *RegisterClass_Original)(const WNDCLASS *lpWndClass);

IDirect3D9* WINAPI Direct3DCreate9_Hooked(UINT SDKVersion);
FARPROC WINAPI GetProcAddress_Hooked(HMODULE hModule,LPCSTR lpProcName);
HRESULT PatchIat(HMODULE Module,PSTR ImportedModuleName,PSTR ImportedProcName,PVOID AlternateProc,PVOID *OldProc);
ATOM (WINAPI RegisterClass_Hooked)(const WNDCLASS *lpWndClass);
LRESULT CALLBACK CustomWindowProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);



