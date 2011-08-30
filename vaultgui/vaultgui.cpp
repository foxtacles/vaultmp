#include <windows.h>
#include <string>
#include <queue>

#include "vaultgui.h"
#include "CD3D9Hook.h"
#include "CGUIWindow.h"
#include "../CriticalSection.h"

using namespace std;

CriticalSection cs;
queue<string> ChatFIFO;
bool thread = false;

VOID Render();

DWORD WINAPI guiThread(LPVOID data)
{
    //InitD3D9Hooks();

    while (thread)
    {

        //Render();
        Sleep(50);
    }

    return ((DWORD) data);
}

extern "C" void __declspec(dllexport) Message(string msg)
{
    cs.StartSession();

    ChatFIFO.push(msg);

    cs.EndSession();
}

extern "C" void __declspec(dllexport) End()
{
    thread = false;
}

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    thread = true;
    CreateThread(NULL, 0, guiThread, (LPVOID) 0, 0, NULL);
    return TRUE;
}
