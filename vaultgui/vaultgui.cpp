#define NOMINMAX
#include <windows.h>
#include <string>
#include <queue>

#include "vaultgui.h"
#include "cegui/CEGUI.h"

using namespace std;

queue<string> ChatFIFO;
bool QueueMutex = false;
bool thread = false;

DWORD WINAPI guiThread(LPVOID data)
{


    while (thread)
    {
        while (QueueMutex) Sleep(2);

        QueueMutex = true;

        while (!ChatFIFO.empty())
        {
            string msg = ChatFIFO.front();

            /* To the GUI */

            ChatFIFO.pop();
        }

        QueueMutex = false;

        Sleep(200);
    }

    return ((DWORD) data);
}

extern "C" void __declspec(dllexport) Message(string msg)
{
    while (QueueMutex) Sleep(2);

    QueueMutex = true;

    ChatFIFO.push(msg);

    QueueMutex = false;
}

extern "C" HANDLE __declspec(dllexport) DLLjump()
{
    HANDLE hGUIThread;
    DWORD guiID;

    thread = true;

    hGUIThread = CreateThread(NULL, 0, guiThread, (LPVOID) 0, 0, &guiID);

    return hGUIThread;
}

extern "C" void __declspec(dllexport) DLLend()
{
    thread = false;
}

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
   return TRUE;
}
