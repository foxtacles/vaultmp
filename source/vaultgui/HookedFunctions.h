#pragma once 
#include <windows.h>
#include <d3d9.h>
#include "myIDirect3D9.h"
#include "myIDirect3DDevice9.h"
#include <fstream>

#include "FormattedListboxTextItem.h"

#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include "shlobj.h"

extern myIDirect3D9*       gl_pmyIDirect3D9;
extern myIDirect3DDevice9* gl_pmyIDirect3DDevice9;
extern HINSTANCE           gl_hOriginalDll;
extern HINSTANCE           gl_hThisInstance;

#define DBB(a) 	/*std::ofstream d;d.open("C:\\Users\\PC\\Desktop\\debug.txt",std::ios::app);  d<<a<<std::endl;d.flush();d.close();*/

extern IDirect3D9* (WINAPI *Direct3DCreate9_Original)(UINT SDKVersion);
extern HRESULT (WINAPI *DirectInput8Create_Original)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter);

extern FARPROC (WINAPI *GetProcAddress_Original)(HMODULE hModule,LPCSTR lpProcName);
extern ATOM (WINAPI *RegisterClass_Original)(const WNDCLASS *lpWndClass);
extern HMODULE (WINAPI *LoadLibrary_Original)(LPCTSTR lpFileName);

extern BOOL (WINAPI *ReadFile_Original)(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped);
extern HFILE (WINAPI *OpenFile_Original)(LPCSTR lpFileName,LPOFSTRUCT lpReOpenBuff,UINT uStyle);
extern HANDLE (WINAPI *CreateFile_Original)(LPCTSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);
extern HRESULT (WINAPI *D3DXCreateTextureFromFileInMemory_Original)(LPDIRECT3DDEVICE9 pDevice,LPCVOID pSrcData,UINT SrcDataSize,LPDIRECT3DTEXTURE9 *ppTexture);

IDirect3D9* WINAPI Direct3DCreate9_Hooked(UINT SDKVersion);
HRESULT WINAPI DirectInput8Create_Hooked(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter);

HMODULE WINAPI LoadLibrary_Hooked(LPCTSTR lpFileName);

FARPROC WINAPI GetProcAddress_Hooked(HMODULE hModule,LPCSTR lpProcName);
HRESULT PatchIat(HMODULE Module,PSTR ImportedModuleName,PSTR ImportedProcName,PVOID AlternateProc,PVOID *OldProc);
ATOM (WINAPI RegisterClass_Hooked)(const WNDCLASS *lpWndClass);
LRESULT CALLBACK CustomWindowProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

BOOL WINAPI ReadFile_Hook(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped);
HFILE WINAPI OpenFile_Hook(LPCSTR lpFileName,LPOFSTRUCT lpReOpenBuff,UINT uStyle);
HANDLE WINAPI CreateFile_Hook(LPCTSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);
HRESULT WINAPI D3DXCreateTextureFromFileInMemory_Hook(LPDIRECT3DDEVICE9 pDevice,LPCVOID pSrcData,UINT SrcDataSize,LPDIRECT3DTEXTURE9 *ppTexture);

extern char* lastTextureLoaded;
extern int loadTextureJmp;
void loadTextureHook();

extern int playerPointer;
void playerPointerHook();