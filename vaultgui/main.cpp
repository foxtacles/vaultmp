#include <Windows.h>
#include "CD3D9Hook.h"
#include "Input.h"

bool APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	switch ( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
			{
				installhook();
				InitD3D9Hooks();
				break;
			}
	}

	return true;
}
