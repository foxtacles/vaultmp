#include <Windows.h>
#include "CD3D9Hook.h"
#include "Input.h"

bool APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	switch ( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
			{
				InitD3D9Hooks();
				CreateThread(0,0,(LPTHREAD_START_ROUTINE)installhook,0,0,0);// TODO: move init code to void DllExport Initiate();
				break;
			}
	}

	return true;
}
