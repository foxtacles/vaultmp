#include "Fallout3.h"

using namespace std;
using namespace RakNet;
using namespace pipe;

typedef HINSTANCE (__stdcall *fLoadLibrary)(char*);
typedef LPVOID (__stdcall *fGetProcAddress)(HINSTANCE, char*);
typedef void (*fDLLjump)(void);

enum {
    ID_MASTER_QUERY = ID_USER_PACKET_ENUM,
    ID_MASTER_ANNOUNCE,
    ID_MASTER_UPDATE,
    ID_GAME_INIT,
    ID_GAME_RUN,
    ID_GAME_END
};

bool Fallout3::endThread;
bool Fallout3::wakeup = false;
HANDLE Fallout3::Fallout3thread;
PipeClient Fallout3::pipeClient;

DWORD Fallout3::lookupProgramID(const char process[])
{
      HANDLE hSnapshot;
      PROCESSENTRY32 ProcessEntry;
      ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
      hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

      if (Process32First(hSnapshot, &ProcessEntry))
         do
         {
          if (!strcmp(ProcessEntry.szExeFile, process))
             {
              CloseHandle(hSnapshot);
              return ProcessEntry.th32ProcessID;
             }
         } while(Process32Next(hSnapshot, &ProcessEntry));

      CloseHandle(hSnapshot);

      return 0;
}

struct Fallout3::INJECT
{
      fLoadLibrary LoadLibrary;
      fGetProcAddress GetProcAddress;
      char DLLpath[256];
      char DLLjump[16];
};

DWORD WINAPI Fallout3::InjectedCode(LPVOID addr)
{
	HINSTANCE hDll;
	fDLLjump DLLjump;
	INJECT* is = (INJECT*) addr;
	hDll = is->LoadLibrary(is->DLLpath);
	DLLjump = (fDLLjump) is->GetProcAddress(hDll, is->DLLjump);
	DLLjump();
	return 0;
}

void Fallout3::InjectedEnd()
{
     /* This is to calculate the size of InjectedCode */
}

DWORD WINAPI Fallout3::Fallout3pipe(LPVOID data)
{
      pipeClient.SetPipeAttributes("Fallout3pipe", 512);

      while (!pipeClient.ConnectToServer() && !endThread);

      string send;
      string recv;
      string low;
      string high;

      if (!endThread)
      {
          do
          {
                recv = pipeClient.Recv();
                low = recv.substr(0, 3);
                high = recv.substr(3);

                if (low.compare("op:") == 0)
                {

                }
                else if (low.compare("up:") == 0)
                    wakeup = true;

                if (lookupProgramID("Fallout3.exe") == 0)
                    endThread = true;
          } while (low.compare("ca:") != 0 && !endThread);
      }

      return ((DWORD) data);
}

HANDLE Fallout3::InitalizeFallout3()
{
    FILE* Fallout3 = fopen("Fallout3.exe", "rb");

    if (Fallout3 != NULL)
    {
        fclose(Fallout3);

        FILE* vaultmp = fopen("vaultmp.dll", "rb");

        if (vaultmp != NULL)
        {
            fclose(vaultmp);

            if (lookupProgramID("Fallout3.exe") == 0)
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;

                ZeroMemory(&si, sizeof(si));
                ZeroMemory(&pi, sizeof(pi));
                si.cb = sizeof(si);

                if (CreateProcess("Fallout3.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
                {
                    HANDLE hProc;

                    hProc = OpenProcess(PROCESS_ALL_ACCESS, false, pi.dwProcessId);

                    if (hProc)
                    {
                        INJECT data;
                        HINSTANCE hDll;
                        TCHAR curdir[MAX_PATH + 1];
                        LPVOID start, thread;
                        DWORD codesize;

                        GetModuleFileName(GetModuleHandle(NULL), (LPTSTR) curdir, MAX_PATH);
                        PathRemoveFileSpec(curdir);

                        strcat(curdir, "\\vaultmp.dll");
                        strcpy(data.DLLpath, curdir);
                        strcpy(data.DLLjump, "DLLjump");

                        hDll = LoadLibrary("kernel32.dll");
                        data.LoadLibrary = (fLoadLibrary) GetProcAddress(hDll, "LoadLibraryA");
                        data.GetProcAddress = (fGetProcAddress) GetProcAddress(hDll, "GetProcAddress");

                        codesize = (DWORD) InjectedEnd - (DWORD) InjectedCode;

                        start = VirtualAllocEx(hProc, 0, codesize + sizeof(INJECT), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
                        thread = (LPVOID) ((DWORD) start + sizeof(INJECT));

                        WriteProcessMemory(hProc, start, (LPVOID) &data, sizeof(INJECT), NULL);
                        WriteProcessMemory(hProc, thread, (LPVOID) InjectedCode, codesize, NULL);

                        CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE) thread, start, 0, 0);

                        /* Initalizing vaultmp.exe <-> Fallout3.exe pipe */

                        HANDLE PipeThread;
                        DWORD Fallout3pipeID;

                        PipeThread = CreateThread(NULL, 0, Fallout3pipe, (LPVOID) 0, 0, &Fallout3pipeID);

                        while (!wakeup) Sleep(2);

                        /* Resuming Fallout3.exe */

                        ResumeThread(pi.hThread);

                        CloseHandle(hProc);

                        return PipeThread;
                    }
                    else
                        return NULL; // Process opening failed
                }
                else
                    return NULL; // Process creation failed
            }
            else
                return NULL; // Fallout3.exe running
        }
        else
            return NULL; // vaultmp.dll missing
    }
    else
         return NULL; // Fallout3.exe missing

    return NULL;
}

void Fallout3::InitalizeVaultMP(RakPeerInterface* peer, SystemAddress addr, string name, string pwd)
{
    endThread = false;
    Fallout3thread = NULL;

    if (peer->Connect(addr.ToString(false), addr.port, 0, 0, 0, 0, 3, 500, 0) == CONNECTION_ATTEMPT_STARTED)
    {
        bool query = true;

        Packet* packet;

        while (query)
        {
            if (Fallout3thread != NULL)
                if (WaitForSingleObject(Fallout3thread, 0) != WAIT_TIMEOUT)
                {
                    BitStream query;
                    query.Write((MessageID) ID_GAME_END);
                    peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, addr, false, 0);
                }

            for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
            {
                switch (packet->data[0])
                {
                case ID_CONNECTION_REQUEST_ACCEPTED:
                {
                    BitStream query;

                    query.Write((MessageID) ID_GAME_INIT);

                    RakString rname(name.c_str());
                    RakString rpwd(pwd.c_str());
                    query.Write(rname);
                    query.Write(rpwd);

                    peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);
                    break;
                }
                case ID_GAME_INIT:
                {
                    BitStream query;

                    Fallout3thread = InitalizeFallout3();

                    if (Fallout3thread != NULL)
                        query.Write((MessageID) ID_GAME_RUN);
                    else
                        query.Write((MessageID) ID_GAME_END);

                    peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);
                    break;
                }
                case ID_NO_FREE_INCOMING_CONNECTIONS:
                case ID_CONNECTION_ATTEMPT_FAILED:
                case ID_DISCONNECTION_NOTIFICATION:
                case ID_CONNECTION_BANNED:
                case ID_CONNECTION_LOST:
                    query = false;
                    break;
                }
            }

            RakSleep(2);
        }
    }

    if (Fallout3thread != NULL)
    {
        if (WaitForSingleObject(Fallout3thread, 0) == WAIT_TIMEOUT)
            endThread = true;
        CloseHandle(Fallout3thread);
    }
}
