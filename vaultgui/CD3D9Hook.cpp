#define CINTERFACE // allow VTable access
#include "CD3D9Hook.h"
#include "CGUIWindow.h"
#include "CGUISettings.h"
#include "vaultDetour.h"
#include "Input.h"
#include <d3d9.h>

typedef IDirect3D9* ( WINAPI* tDirect3DCreate9 )( UINT );
typedef HRESULT ( WINAPI* tCreateDevice )( IDirect3D9* , UINT , D3DDEVTYPE , HWND , DWORD , D3DPRESENT_PARAMETERS* , IDirect3DDevice9** );
typedef void ( WINAPI* tEndScene )( BOOL );
typedef HRESULT ( WINAPI* tReset )( D3DPRESENT_PARAMETERS* );
tDirect3DCreate9 oDirect3DCreate9 = 0;
tDirect3DCreate9 aD3DCreate9 = 0;
IDirect3D9* pDirect3D = 0;
IDirect3DDevice9* pDevice = 0;
tCreateDevice oCreateDevice = 0;// trampoline
tCreateDevice aCreateDevice = 0;// the actual dev->CreateDevice
tEndScene oEndScene = 0;
void* oRelease = 0;
void* oPresent = 0;
tReset oReset = 0;
int iWindowed = 0;


char nullprefix = 0x00;//textbox buffer must have a null byte before it
char textbox_buffer[10000] = {"Welcome to the Vault-MP Chatbox!\n\x00"};
CD3DRender rend;
CWindow* tw = 0;
int drawgui = 1;
char* ui_cache = 0;


#define DllExport   __declspec( dllexport )__stdcall

void DllExport AppendChatMessageA( char* msg )
{
	strcat( textbox_buffer, msg );
}

void DllExport FlushBuffer()
{
	memset( textbox_buffer, 0, sizeof( textbox_buffer ) );
	strcpy( textbox_buffer, "Textbox buffer has been flushed\n" );
}

void DllExport FlushBuffer_NoMsg()
{
	memset( textbox_buffer, 0, sizeof( textbox_buffer ) );
}

void DllExport ToggleDrawing( int tog )
{
	/*enum tog
	{
		DRAW_NOTHING,
		DRAW_ALL
	*/
	drawgui = tog;
}

__declspec( naked ) void hPresent()
{
	_asm push ebp

	if( drawgui == 1 )
	{
		rend.BeginScene();
		rend.Text_Begin();
		rend.Shape_Begin();
		tw->Render();
		rend.Draw2DTriangle( rend.cX - 2, rend.cY - 2, rend.cX + 17, rend.cY + 10, rend.cX + 4, rend.cY + 17, rend.Easy_Grey ); // cursor outline
		rend.Draw2DTriangle( rend.cX, rend.cY, rend.cX + 15, rend.cY + 10, rend.cX + 5, rend.cY + 15, rend.Easy_Green ); // cursor fill
		rend.DrawTextMain( 20, 20, rend.Normal, iBuf, rend.Black ); // TOOO: remove when keyboard input is working
		rend.Shape_End();
		rend.Text_End();
	}

	_asm pop ebp
	_asm jmp oPresent
}

__declspec( naked ) void hRelease()
{
	rend.Release();
	_asm jmp oRelease
}

__declspec( naked ) HRESULT hReset( D3DPRESENT_PARAMETERS* pPresentationParameters )
{
	_asm pushad
	rend.Lost();
	//rend.Release();
	//rend.Reset();
	_asm popad

	//iWindowed = pPresentationParameters->Windowed;
	//_asm jmp oReset
	oReset( pPresentationParameters );
	_asm pushad

	rend.Reset();
	//rend.Initialize();
	//rend.Lost();
	_asm popad
}

HRESULT APIENTRY hCreateDevice( IDirect3D9* pDirect3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface )
{
	HRESULT ret = oCreateDevice( pDirect3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface );
	iWindowed = pPresentationParameters->Windowed;

	if( *ppReturnedDeviceInterface )
	{
		if( *ppReturnedDeviceInterface != pDevice )
		{
			pDevice = *ppReturnedDeviceInterface;

			//oEndScene = (tEndScene)DetourForSteam((*ppReturnedDeviceInterface)->lpVtbl->EndScene,hEndScene);
			oPresent = ( void* )DetourForSteam( ( *ppReturnedDeviceInterface )->lpVtbl->Present, hPresent );
			//oRelease = (void*)DetourForSteam((*ppReturnedDeviceInterface)->lpVtbl->Release,hRelease);
			oReset = ( tReset )DetourForSteam( ( *ppReturnedDeviceInterface )->lpVtbl->Reset, hReset );
		}

		rend.d = pDevice;
		rend.Initialize();

		if( tw )
			delete tw;

		tw = new CWindow( 1, 1, WINDOW_W, WINDOW_H, &rend, "Vault-MP GUI" );
		tw->AddTab( "Chat" );
		tw->AddTextbox( 2, 48, WINDOW_W - 18, WINDOW_H - 51, textbox_buffer, 0 );
		/*
		tw->AddTab("Settings");
		tw->AddTextbox(2, 48, WINDOW_W-18, WINDOW_H-51, "Nothing here, yet", 1);
		*/
	}

	return ret;
}

IDirect3D9* APIENTRY hDirect3DCreate9( UINT uiSDKVersion )
{
	IDirect3D9* ret = oDirect3DCreate9( uiSDKVersion );

	if( !pDirect3D )
	{
		pDirect3D = ret;
		aCreateDevice = ret->lpVtbl->CreateDevice;
		oCreateDevice = ( tCreateDevice )DetourForSteam( aCreateDevice, hCreateDevice );
	}

	return ret;
}

void InitD3D9Hooks()
{
	HMODULE md3d9 = LoadLibraryA( "d3d9.dll" );

	if( md3d9 )
	{
		aD3DCreate9 = ( tDirect3DCreate9 )GetProcAddress( md3d9, "Direct3DCreate9" );

		if( aD3DCreate9 )
		{
			oDirect3DCreate9 = ( tDirect3DCreate9 )DetourForSteam( aD3DCreate9, hDirect3DCreate9 );
		}
	}
}

void DestD3D9Hooks()
{
	// minor TODO: remove all hooks, free all trampolines
}
