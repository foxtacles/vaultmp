#include "vaultmp.h"

#include <winsock2.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <map>
#include <chrono>
#include <thread>

#include "Bethesda.h"
#include "ServerEntry.h"
#include "Data.h"
#include "VaultException.h"
#include "ufmod.h"
#include "iniparser/src/dictionary.h"
#include "iniparser/src/iniparser.h"

#include "RakNet/RakPeerInterface.h"
#include "RakNet/PacketizedTCP.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/FileListTransfer.h"
#include "RakNet/FileListTransferCBInterface.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakString.h"
#include "RakNet/RakSleep.h"
#include "RakNet/GetTime.h"

#define MSG_MINTRAYICON         (WM_USER + 1)

#define WND_CLASS_NAME          "vaultmp"

#define RAKNET_CONNECTIONS      2
#define RAKNET_MASTER_ADDRESS   "127.0.0.1"
#define RAKNET_MASTER_PORT      1660

#define IDC_GROUP0              2000
#define IDC_GROUP1              2001
#define IDC_GROUP2              2002
#define IDC_STATIC0             2003
#define IDC_STATIC1             2004
#define IDC_STATIC2             2005
#define IDC_STATIC3             2006
#define IDC_STATIC4             2007
#define IDC_STATIC5             2008
#define IDC_GRID0               2009
#define IDC_GRID1               2010
#define IDC_CHECK0              2011
#define IDC_BUTTON0             2012
#define IDC_BUTTON1             2013
#define IDC_BUTTON2             2014
#define IDC_BUTTON3             2015
#define IDC_EDIT0               2017
#define IDC_EDIT1               2018
#define IDC_EDIT3               2019
#define IDC_PROGRESS0           2020

#define CHIPTUNE                3000
#define ICON_MAIN               5000
#define POWERED                 6000

using namespace RakNet;
using namespace std;

HINSTANCE instance;
HANDLE global_mutex;
HFONT hFont;
HWND wndmain;
HWND wndsortcur;
HWND wndchiptune;
HWND wndlistview;
HWND wndlistview2;
HWND wndprogressbar;
HWND wndsync;
HDC hdc, hdcMem;
HBITMAP hBitmap;
BITMAP bitmap;
PAINTSTRUCT ps;
BOOL sort_flag;

RakPeerInterface* peer;
SocketDescriptor* sockdescr;

typedef map<SystemAddress, ServerEntry> ServerMap;
ServerMap serverList;

SystemAddress* selectedServer = nullptr;
dictionary* config = nullptr;
const char* player_name;
const char* server_name;
unsigned int inittime;
bool multiinst;
bool steam;
unsigned char games;

HWND CreateMainWindow();
int RegisterClasses();
int MessageLoop();
void InitRakNet();
void CreateWindowContent(HWND parent);
void CleanUp();
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

FileListTransfer* flt;
PacketizedTCP* tcp;
ServerEntry* buf;

void seDebugPrivilege()
{
	TOKEN_PRIVILEGES priv;
	HANDLE hThis, hToken;
	LUID luid;
	hThis = GetCurrentProcess();
	OpenProcessToken(hThis, TOKEN_ADJUST_PRIVILEGES, &hToken);
	LookupPrivilegeValue(0, "seDebugPrivilege", &luid);
	priv.PrivilegeCount = 1;
	priv.Privileges[0].Luid = luid;
	priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, false, &priv, 0, 0, 0);
	CloseHandle(hToken);
	CloseHandle(hThis);
}

void MinimizeToTray(HWND hwnd)
{
	NOTIFYICONDATA nid;

	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = ICON_MAIN;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = MSG_MINTRAYICON;
	nid.hIcon = (HICON) LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(ICON_MAIN), IMAGE_ICON, 16, 16, 0);
	strcpy(nid.szTip, "Vault-Tec Multiplayer Mod");

	Shell_NotifyIcon(NIM_ADD, &nid);

	ShowWindow(hwnd, SW_HIDE);
}

void Maximize(HWND hwnd)
{
	NOTIFYICONDATA nid;

	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = ICON_MAIN;
	Shell_NotifyIcon(NIM_DELETE, &nid);
	ShowWindow(hwnd, SW_RESTORE);
	SetForegroundWindow(hwnd);
}

class FileServer : public FileListTransferCBInterface
{
	public:
		bool OnFile(OnFileStruct* onFileStruct)
		{
			char wndtitle[256];
			snprintf(wndtitle, sizeof(wndtitle), "(100%%) %i/%i %s %i bytes / %i bytes\n",
					 onFileStruct->fileIndex + 1,
					 onFileStruct->numberOfFilesInThisSet,
					 onFileStruct->fileName,
					 onFileStruct->byteLengthOfThisFile,
					 onFileStruct->byteLengthOfThisSet);

			SetWindowText(wndmain, wndtitle);

			TCHAR file[MAX_PATH];
			ZeroMemory(file, sizeof(file));

			switch (onFileStruct->context.op)
			{
				case FILE_MODFILE:
				{
					GetModuleFileName(GetModuleHandle(nullptr), (LPTSTR) file, MAX_PATH);
					PathRemoveFileSpec(file);

					strcat(file, "\\Data\\");
					strcat(file, Utils::FileOnly(onFileStruct->fileName));
					break;
				}

				default:
					return true;
			}

			FILE* fp = fopen(file, "rb");

			if (fp != nullptr)
			{
				fclose(fp);

				char msg[256];
				snprintf(msg, sizeof(msg), "%s\n\nalready exists. Do you want to overwrite it?", file);
				int result = MessageBox(nullptr, msg, "Attention", MB_YESNO | MB_ICONWARNING | MB_TOPMOST | MB_TASKMODAL);

				if (result == IDNO)
				{
					return true;
				}
			}

			fp = fopen(file, "wb");
			fwrite(onFileStruct->fileData, onFileStruct->byteLengthOfThisFile, 1, fp);
			fclose(fp);

			return true;
		}

		virtual void OnFileProgress(FileProgressStruct* fps)
		{
			char wndtitle[256];
			snprintf(wndtitle, sizeof(wndtitle), "(%i%%) %i/%i %s %i bytes / %i bytes\n",
					 (int)(100.0 * (double) fps->partCount / (double) fps->partTotal),
					 fps->onFileStruct->fileIndex + 1,
					 fps->onFileStruct->numberOfFilesInThisSet,
					 fps->onFileStruct->fileName,
					 fps->onFileStruct->byteLengthOfThisFile,
					 fps->onFileStruct->byteLengthOfThisSet);

			SetWindowText(wndmain, wndtitle);
		}

		virtual bool OnDownloadComplete(DownloadCompleteStruct*)
		{
			char wndtitle[sizeof(CLIENT_VERSION) + 64];
			snprintf(wndtitle, sizeof(wndtitle), "Vault-Tec Multiplayer Mod %s (FOR TESTING PURPOSES ONLY)", CLIENT_VERSION);
			SetWindowText(wndmain, wndtitle);
			buf = nullptr;
			return false;
		}

} transferCallback;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
#ifdef VAULTMP_DEBUG

	if (LoadLibrary("exchndl.dll") == nullptr)
		return MessageBox(nullptr, "Could not find exchndl.dll!", "Error", MB_OK | MB_ICONERROR);

#endif

	global_mutex = CreateMutex(nullptr, TRUE, "vaultmp");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return MessageBox(nullptr, "Vault-Tec Multiplayer Mod is already running!", "Error", MB_OK | MB_ICONERROR);

	FILE* filecheck = nullptr;
	DWORD checksum, checksum_real;

	filecheck = fopen("Fallout3.exe", "rb");

	if (filecheck != nullptr)
	{
		fclose(filecheck);
		Utils::GenerateChecksum("Fallout3.exe", checksum, checksum_real);

		if (checksum == FALLOUT3_EN_VER17 /*|| checksum == FALLOUT3_EN_VER17_STEAM*/)
		{
			filecheck = fopen("fose_1_7.dll", "rb");

			if (filecheck != nullptr)
			{
				fclose(filecheck);
				Utils::GenerateChecksum("fose_1_7.dll", checksum, checksum_real);

				if (checksum_real == FOSE_VER0122)
				{
					filecheck = fopen("xlive.dll", "rb");

					if (filecheck != nullptr)
					{
						fclose(filecheck);
						Utils::GenerateChecksum("xlive.dll", checksum, checksum_real);

						if (checksum_real == XLIVE_PATCH)
						{
							filecheck = fopen("Data/vaultmpF3.esp", "rb");

							if (filecheck != nullptr)
							{
								fclose(filecheck);
								unsigned int crc;
								Utils::crc32file("Data/vaultmpF3.esp", &crc);

								if (crc == VAULTMP_F3)
								{
									games |= FALLOUT3;
								}
								else
									return MessageBox(nullptr, "vaultmpF3.esp is outdated or has been modified!", "Error", MB_OK | MB_ICONERROR);
							}
							else
								return MessageBox(nullptr, "vaultmpF3.esp is missing!", "Error", MB_OK | MB_ICONERROR);
						}
						else
							return MessageBox(nullptr, "xlive.dll is unpatched!", "Error", MB_OK | MB_ICONERROR);
					}
					else
						return MessageBox(nullptr, "xlive.dll is missing!", "Error", MB_OK | MB_ICONERROR);
				}
				else
					return MessageBox(nullptr, "Your FOSE version is probably outdated!\nhttp://fose.silverlock.org/", "Error", MB_OK | MB_ICONERROR);
			}
			else
				return MessageBox(nullptr, "Could not find FOSE!\nhttp://fose.silverlock.org/", "Error", MB_OK | MB_ICONERROR);
		}
		else
			return MessageBox(nullptr, "Your version of Fallout 3 is not supported!", "Error", MB_OK | MB_ICONERROR);
	}

	filecheck = fopen("FalloutNV.exe", "rb");

	if (filecheck != nullptr)
	{
		fclose(filecheck);
		Utils::GenerateChecksum("FalloutNV.exe", checksum, checksum_real);

		if (checksum == NEWVEGAS_EN_VER14_STEAM)
		{
			steam = (checksum_real == NEWVEGAS_EN_VER14_STEAM);

			filecheck = fopen("nvse_1_4.dll", "rb");

			if (filecheck != nullptr)
			{
				fclose(filecheck);
				Utils::GenerateChecksum("nvse_1_4.dll", checksum, checksum_real);

				if (checksum_real == NVSE_VER0212)
				{
					filecheck = fopen("Data/vaultmpFNV.esp", "rb");

					if (filecheck != nullptr)
					{
						fclose(filecheck);
						unsigned int crc;
						Utils::crc32file("Data/vaultmpFNV.esp", &crc);

						if (crc == VAULTMP_FNV)
						{
							games |= NEWVEGAS;
						}
						else
							return MessageBox(nullptr, "vaultmpFNV.esp is outdated or has been modified!", "Error", MB_OK | MB_ICONERROR);
					}
					else
						return MessageBox(nullptr, "vaultmpFNV.esp is missing!", "Error", MB_OK | MB_ICONERROR);
				}
				else
					return MessageBox(nullptr, "Your NVSE version is probably outdated!\nhttp://nvse.silverlock.org/", "Error", MB_OK | MB_ICONERROR);
			}
			else
				return MessageBox(nullptr, "Could not find NVSE!\nhttp://nvse.silverlock.org/", "Error", MB_OK | MB_ICONERROR);
		}
		else
			return MessageBox(nullptr, "Your version of Fallout: New Vegas is not supported!", "Error", MB_OK | MB_ICONERROR);
	}

	if (!games)
		return MessageBox(nullptr, "Could not find either Fallout 3 or Fallout: New Vegas!", "Error", MB_OK | MB_ICONERROR);

	filecheck = fopen("vaultmp.dll", "rb");

	if (filecheck != nullptr)
	{
		fclose(filecheck);
		Utils::GenerateChecksum("vaultmp.dll", checksum, checksum_real);
        /*
		if (checksum_real != VAULTMP_DLL)
		    return MessageBox(NULL, "vaultmp.dll is not up to date!", "Error", MB_OK | MB_ICONERROR);
        */
	}
	else
		return MessageBox(nullptr, "Could not find vaultmp.dll!", "Error", MB_OK | MB_ICONERROR);

	instance = hInstance;

	seDebugPrivilege();
	InitCommonControls();
	RegisterClasses();
	InitRakNet();

	config = iniparser_load("vaultmp.ini");
	player_name = iniparser_getstring(config, "general:name", "");
	server_name = iniparser_getstring(config, "general:master", "");
	inittime = iniparser_getint(config, "general:inittime", 9000);
	multiinst = (bool) iniparser_getboolean(config, "general:multiinst", 0);
	const char* servers = iniparser_getstring(config,  "general:servers", "");

	char* token;
	char buf[strlen(servers) + 1];
	strcpy(buf, servers);
	token = strtok(buf, ",");

	while (token != nullptr)
	{
		SystemAddress addr;
		char* port;

		if ((port = strchr(token, ':')))
		{
			*port = '\0';
			addr.SetPort(atoi(port + 1));
		}

		addr.SetBinaryAddress(token);

		ServerEntry entry(addr.ToString(true), "", make_pair(0u, 0u), USHRT_MAX, 0);

		serverList.insert(pair<SystemAddress, ServerEntry>(addr, entry));

		token = strtok(nullptr, ",");
	}

	hFont = CreateFont(-11, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana");
	wndmain = CreateMainWindow();

	return MessageLoop();
}

HWND CreateMainWindow()
{
	HWND wnd;
	char wndtitle[sizeof(CLIENT_VERSION) + 64];
	snprintf(wndtitle, sizeof(wndtitle), "Vault-Tec Multiplayer Mod %s (FOR TESTING PURPOSES ONLY)", CLIENT_VERSION);
	wnd = CreateWindowEx(0, WND_CLASS_NAME, wndtitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, (GetSystemMetrics(SM_CXSCREEN) / 2) - 392, (GetSystemMetrics(SM_CYSCREEN) / 2) - 221, 785, 442, HWND_DESKTOP, nullptr, instance, nullptr);
	ShowWindow(wnd, SW_SHOWNORMAL);
	UpdateWindow(wnd);
	return wnd;
}

void InitRakNet()
{
	tcp = PacketizedTCP::GetInstance();
	flt = FileListTransfer::GetInstance();
	sockdescr = new SocketDescriptor();
	tcp->Start(RAKNET_FILE_SERVER, 1);
	tcp->AttachPlugin(flt);
	peer = RakPeerInterface::GetInstance();
	peer->Startup(RAKNET_CONNECTIONS, sockdescr, 1, THREAD_PRIORITY_NORMAL);
}

void CreateWindowContent(HWND parent)
{
	HWND wnd;
	LV_COLUMN col;
	sort_flag = true;

	col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt = LVCFMT_LEFT;

	wnd = CreateWindowEx(0x00000000, "Button", "Server details", 0x50020007, 543, 0, 229, 214, parent, (HMENU) IDC_GROUP0, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);

	wnd = CreateWindowEx(0x00000000, "Button", "Vault-Tec Multiplayer Controls", 0x50020007, 543, 218, 229, 190, parent, (HMENU) IDC_GROUP1, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);

	wnd = CreateWindowEx(0x00000200, "SysListView32", "", 0x50010005 | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 6, 6, 531, 285, parent, (HMENU) IDC_GRID0, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);
	SendMessage(wnd, (LVM_FIRST + 54), 0, 64 | 32);
	wndlistview = wnd;

	col.cx = 299;
	col.pszText = (char*) "Name";
	col.iSubItem = 0;
	SendMessage(wnd, LVM_INSERTCOLUMN, 0, (LPARAM) &col);

	col.cx = 62;
	col.pszText = (char*) "Players";
	col.iSubItem = 1;
	SendMessage(wnd, LVM_INSERTCOLUMN, 1, (LPARAM) &col);

	col.cx = 50;
	col.pszText = (char*) "Ping";
	col.iSubItem = 2;
	SendMessage(wnd, LVM_INSERTCOLUMN, 2, (LPARAM) &col);

	col.cx = 116;
	col.pszText = (char*) "Map";
	col.iSubItem = 3;
	SendMessage(wnd, LVM_INSERTCOLUMN, 3, (LPARAM) &col);

	wnd = CreateWindowEx(0x00000200, "SysListView32", "", 0x50010001, 553, 19, 210, 157, parent, (HMENU) IDC_GRID1, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);
	wndlistview2 = wnd;

	col.cx = 103;
	col.pszText = (char*) "Key";
	col.iSubItem = 0;
	SendMessage(wnd, LVM_INSERTCOLUMN, 0, (LPARAM) &col);

	col.cx = 103;
	col.pszText = (char*) "Value";
	col.iSubItem = 1;
	SendMessage(wnd, LVM_INSERTCOLUMN, 1, (LPARAM) &col);

	wnd = CreateWindowEx(0x00000000, "msctls_progress32", "", 0x50000000, 553, 184, 210, 16, parent, (HMENU) IDC_PROGRESS0, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);
	wndprogressbar = wnd;

	wnd = CreateWindowEx(0x00000000, "Static", "Fallout is a trademark of Bethesda Softworks LLC in the U.S. and/or other countries.", 0x5000030C, 12, 374, 531, 18, parent, (HMENU) IDC_STATIC0, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);

	wnd = CreateWindowEx(0x00000000, "Static", "Vault-Tec Multiplayer Mod is not affliated with Bethesda Softworks LLC.", 0x50000300, 12, 392, 531, 18, parent, (HMENU) IDC_STATIC1, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);

	wnd = CreateWindowEx(0x00000000, "Button", "Powered by", 0x50020007, 6, 294, 531, 78, parent, (HMENU) IDC_GROUP2, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);

	wnd = CreateWindowEx(0x00000000, "Button", "mantronix - the wasteland", 0x50010003, 555, 374, 174, 32, parent, (HMENU) IDC_CHECK0, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);
	wndchiptune = wnd;

	wnd = CreateWindowEx(0x00000000, "Button", "Join Server", WS_BORDER | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 555, 239, 100, 25, parent, (HMENU) IDC_BUTTON0, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);

	wnd = CreateWindowEx(0x00000000, "Button", "Update Server", WS_BORDER | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 660, 239, 100, 25, parent, (HMENU) IDC_BUTTON1, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);

	wnd = CreateWindowEx(0x00000000, "Button", "Master Query", WS_BORDER | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 555, 272, 100, 25, parent, (HMENU) IDC_BUTTON2, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);

	wnd = CreateWindowEx(0x00000000, "Button", "Synchronize", WS_BORDER | WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 660, 272, 100, 25, parent, (HMENU) IDC_BUTTON3, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);
	wndsync = wnd;

	wnd = CreateWindowEx(0x00000200, "Edit", "vaultmp.com", 0x50010080, 611, 305, 146, 20, parent, (HMENU) IDC_EDIT3, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);
	SendMessage(wnd, EM_SETLIMITTEXT, (WPARAM) MAX_MASTER_SERVER, 0);

	wnd = CreateWindowEx(0x00000200, "Edit", "", 0x50010080, 611, 331, 146, 20, parent, (HMENU) IDC_EDIT0, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);
	SendMessage(wnd, EM_SETLIMITTEXT, (WPARAM) MAX_PLAYER_NAME, 0);

	wnd = CreateWindowEx(0x00000200, "Edit", "", 0x500100A0, 611, 357, 146, 20, parent, (HMENU) IDC_EDIT1, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);
	SendMessage(wnd, EM_SETLIMITTEXT, (WPARAM) MAX_PASSWORD_SIZE, 0);

	wnd = CreateWindowEx(0x00000000, "Static", "Master", 0x50000300, 570, 302, 38, 24, parent, (HMENU) IDC_STATIC4, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);

	wnd = CreateWindowEx(0x00000000, "Static", "Name", 0x50000300, 575, 328, 35, 24, parent, (HMENU) IDC_STATIC2, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);

	wnd = CreateWindowEx(0x00000000, "Static", "Password", 0x50000300, 554, 354, 57, 24, parent, (HMENU) IDC_STATIC3, instance, nullptr);
	SendMessage(wnd, WM_SETFONT, (WPARAM) hFont, TRUE);
}

int RegisterClasses()
{
	WNDCLASSEX wc;

	wc.hInstance = instance;
	wc.lpszClassName = WND_CLASS_NAME;
	wc.lpfnWndProc = WindowProcedure;
	wc.style = CS_DBLCLKS;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(ICON_MAIN));
	wc.hIconSm = LoadIcon(instance, MAKEINTRESOURCE(ICON_MAIN));
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszMenuName = nullptr;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;

	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);

	return RegisterClassEx(&wc);
}

int MessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CleanUp();

	return msg.wParam;
}

void CleanUp()
{
	DeleteObject(hFont);
	DeleteObject(hBitmap);
	tcp->DetachPlugin(flt);
	FileListTransfer::DestroyInstance(flt);
	PacketizedTCP::DestroyInstance(tcp);
	peer->Shutdown(300);
	RakPeerInterface::DestroyInstance(peer);
	iniparser_freedict(config);
	CloseHandle(global_mutex);
}

int Create2ColItem(HWND hwndList, char* text1, char* text2)
{
	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	int ret;
	lvi.mask = LVIF_TEXT;
	lvi.pszText = text1;
	ret = ListView_InsertItem(hwndList, &lvi);

	if (ret >= 0)
		ListView_SetItemText(hwndList, ret, 1, text2);

	return ret;
}

int Create4ColItem(HWND hwndList, const SystemAddress* addr, char* text1, char* text2, char* text3, char* text4)
{
	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	int ret;
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.pszText = text1;
	lvi.lParam = (LPARAM) addr;
	ret = ListView_InsertItem(hwndList, &lvi);

	if (ret >= 0)
	{
		ListView_SetItemText(hwndList, ret, 1, text2);
		ListView_SetItemText(hwndList, ret, 2, text3);
		ListView_SetItemText(hwndList, ret, 3, text4);
	}

	return ret;
}

void RefreshServerList()
{
	SendMessage(wndlistview, LVM_DELETEALLITEMS, 0, 0);
	SendMessage(wndlistview2, LVM_DELETEALLITEMS, 0, 0);
	selectedServer = nullptr;

	for (map<SystemAddress, ServerEntry>::iterator i = serverList.begin(); i != serverList.end(); ++i)
	{
		const SystemAddress* addr = &i->first;
		ServerEntry entry = i->second;

		char players[16];
		char ping[16];
		snprintf(players, sizeof(players), "%d / %d", entry.GetServerPlayers().first, entry.GetServerPlayers().second);
		snprintf(ping, sizeof(ping), "%d", entry.GetServerPing());

		Create4ColItem(wndlistview, addr, (char*) entry.GetServerName().c_str(), players, ping, (char*) entry.GetServerMap().c_str());
	}
}

int CALLBACK CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	static char buf[64], buf2[64];

	ListView_GetItemText(wndsortcur, lParam1, lParamSort, buf, sizeof(buf));
	ListView_GetItemText(wndsortcur, lParam2, lParamSort, buf2, sizeof(buf2));

	if (sort_flag)
		return (stricmp(buf, buf2));

	else
		return (stricmp(buf, buf2) * -1);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool update = false;

	switch (message)
	{
		case WM_CREATE:
			CreateWindowContent(hwnd);
			RefreshServerList();
			hBitmap = LoadBitmap(GetModuleHandle(nullptr), MAKEINTRESOURCE(POWERED));
			GetObject(hBitmap, sizeof(BITMAP), &bitmap);

			if (*player_name)
				SetDlgItemText(hwnd, IDC_EDIT0, player_name);

			if (*server_name)
				SetDlgItemText(hwnd, IDC_EDIT3, server_name);

			break;

		case WM_PAINT:
			hdc = BeginPaint(hwnd, &ps);
			hdcMem = CreateCompatibleDC(hdc);

			SelectObject(hdcMem, hBitmap);
			BitBlt(hdc, 13, 310, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

			DeleteDC(hdcMem);
			EndPaint(hwnd, &ps);
			break;

		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED)
				MinimizeToTray(hwnd);

			break;

		case WM_COMMAND:
			switch (wParam)
			{
				case IDC_BUTTON0:

					if (peer->NumberOfConnections() == 0)
					{
						if (selectedServer != nullptr)
						{
							SystemAddress addr = *selectedServer;
							char name[MAX_PLAYER_NAME], pwd[MAX_PASSWORD_SIZE];
							GetDlgItemText(hwnd, IDC_EDIT0, name, sizeof(name));
							GetDlgItemText(hwnd, IDC_EDIT1, pwd, sizeof(pwd));

							if (strlen(name) < 3)
							{
								MessageBox(nullptr, "Please sepcify a player name of at least 3 characters.", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_TASKMODAL);
								break;
							}

							map<SystemAddress, ServerEntry>::iterator i;
							i = serverList.find(*selectedServer);

							unsigned char game = (&i->second)->GetGame();

							if ((games & game) != game || !game)
							{
								switch (game)
								{
									case FALLOUT3:
										MessageBox(nullptr, "Could not find Fallout3.exe!", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_TASKMODAL);
										break;

									case NEWVEGAS:
										MessageBox(nullptr, "Could not find FalloutNV.exe!", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_TASKMODAL);
										break;

									default:
										break;
								}

								break;
							}

							MinimizeToTray(hwnd);

							try
							{
								Bethesda::InitializeVaultMP(peer, addr, name, pwd, game, multiinst, game == NEWVEGAS ? steam : false, inittime);
							}

							catch (std::exception& e)
							{
								try
								{
									VaultException& vaulterror = dynamic_cast<VaultException&>(e);
									vaulterror.Message();
								}

								catch (std::bad_cast& no_vaulterror)
								{
									VaultException vaulterror(e.what());
									vaulterror.Message();
								}
							}

#ifdef VAULTMP_DEBUG
							VaultException::FinalizeDebug();
#endif

							Maximize(hwnd);

							selectedServer = nullptr;
						}
					}

					break;

				case IDC_BUTTON1:
					if (selectedServer != nullptr)
						update = true;

					else break;

				case IDC_BUTTON2:

					/* RakNet Master Query */

					if (peer->NumberOfConnections() == 0)
					{
						if (!update) serverList.clear();

						SystemAddress master;
						char maddr[32];
						GetDlgItemText(hwnd, IDC_EDIT3, maddr, sizeof(maddr));

						if (strcmp(maddr, "") == 0)
						{
							SetDlgItemText(hwnd, IDC_EDIT3, (char*) RAKNET_MASTER_ADDRESS);
							master.SetBinaryAddress((char*) RAKNET_MASTER_ADDRESS);
							master.SetPort(RAKNET_MASTER_PORT);
						}

						else
						{
							master.SetBinaryAddress(strtok(maddr, ":"));
							char* cport = strtok(nullptr, ":");
							master.SetPort(cport != nullptr ? atoi(cport) : RAKNET_MASTER_PORT);
						}

						if (peer->Connect(master.ToString(false), master.GetPort(), MASTER_VERSION, sizeof(MASTER_VERSION), 0, 0, 3, 100, 0) == CONNECTION_ATTEMPT_STARTED)
						{
							bool query = true;
							bool lock = false;
							Packet* packet;

							while (query)
							{
								for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
								{
									switch (packet->data[0])
									{
										case ID_CONNECTION_REQUEST_ACCEPTED:
										{
											BitStream query;

											if (update)
											{
												query.Write((MessageID) ID_MASTER_UPDATE);
												SystemAddress addr = *selectedServer;

												SystemAddress self = peer->GetExternalID(packet->systemAddress);

												if (strcmp(addr.ToString(false), packet->systemAddress.ToString(false)) == 0)
													addr.SetBinaryAddress("127.0.0.1");

												else if (strcmp(addr.ToString(false), "127.0.0.1") == 0)
													addr.SetBinaryAddress(self.ToString(false));

												query.Write(addr);
											}

											else
												query.Write((MessageID) ID_MASTER_QUERY);

											peer->Send(&query, HIGH_PRIORITY, RELIABLE, 0, packet->systemAddress, false, 0);
											break;
										}

										case ID_MASTER_QUERY:
										{
											BitStream query(packet->data, packet->length, false);
											query.IgnoreBytes(sizeof(MessageID));

											unsigned int size;
											query.Read(size);

											SendMessage(wndprogressbar, PBM_SETPOS, 0, 0);
											SendMessage(wndprogressbar, PBM_SETRANGE, 0, MAKELONG(0, size));
											SendMessage(wndprogressbar, PBM_SETSTEP, 1, 0);

											for (unsigned int i = 0; i < size; i++)
											{
												SystemAddress addr;
												RakString name, map;
												unsigned int players, playersMax, rsize, msize;
												unsigned char game;
												std::map<string, string> rules;
												std::vector<string> modfiles;

												query.Read(addr);
												query.Read(name);
												query.Read(map);
												query.Read(players);
												query.Read(playersMax);
												query.Read(game);
												query.Read(rsize);

												ServerEntry entry(name.C_String(), map.C_String(), make_pair(players, playersMax), USHRT_MAX, game);

												for (int j = 0; j < rsize; j++)
												{
													RakString key, value;
													query.Read(key);
													query.Read(value);
													entry.SetServerRule(key.C_String(), value.C_String());
												}

												query.Read(msize);

												for(int j = 0; j < msize; j++)
												{
												    RakString mod_name;
												    query.Read(mod_name);
												    entry.SetModFiles(mod_name.C_String());
												}



												SystemAddress self = peer->GetExternalID(packet->systemAddress);

												if (strcmp(addr.ToString(false), "127.0.0.1") == 0)
													addr.SetBinaryAddress(packet->systemAddress.ToString(false));

												else if (strcmp(addr.ToString(false), self.ToString(false)) == 0)
													addr.SetBinaryAddress("127.0.0.1");

												serverList.insert(pair<SystemAddress, ServerEntry>(addr, entry));

												peer->Ping(addr.ToString(false), addr.GetPort(), false);

												SendMessage(wndprogressbar, PBM_STEPIT, 0, 0);
											}

											peer->CloseConnection(packet->systemAddress, true, 0, LOW_PRIORITY);

											query = false;
											break;
										}

										case ID_MASTER_UPDATE:
										{
											BitStream query(packet->data, packet->length, false);
											query.IgnoreBytes(sizeof(MessageID));

											SystemAddress addr;
											query.Read(addr);

											SystemAddress self = peer->GetExternalID(packet->systemAddress);

											if (strcmp(addr.ToString(false), "127.0.0.1") == 0)
												addr.SetBinaryAddress(packet->systemAddress.ToString(false));

											else if (strcmp(addr.ToString(false), self.ToString(false)) == 0)
												addr.SetBinaryAddress("127.0.0.1");

											std::map<SystemAddress, ServerEntry>::iterator i;
											i = serverList.find(addr);

											if (query.GetNumberOfUnreadBits() > 0)
											{
												RakString name, map;
												unsigned int players, playersMax, rsize;
												unsigned char game;

												query.Read(name);
												query.Read(map);
												query.Read(players);
												query.Read(playersMax);
												query.Read(game);
												query.Read(rsize);

												ServerEntry* entry;

												if (i != serverList.end())
												{
													entry = &i->second;
													entry->SetServerName(name.C_String());
													entry->SetServerMap(map.C_String());
													entry->SetServerPlayers(make_pair(players, playersMax));
													entry->SetGame(game);
												}

												else
												{
													std::pair<std::map<SystemAddress, ServerEntry>::iterator, bool> k;
													k = serverList.insert(make_pair(addr, ServerEntry(name.C_String(), map.C_String(), make_pair(players, playersMax), USHRT_MAX, game)));
													entry = &(k.first)->second;
												}

												for (unsigned int j = 0; j < rsize; j++)
												{
													RakString key, value;
													query.Read(key);
													query.Read(value);
													entry->SetServerRule(key.C_String(), value.C_String());
												}

												peer->Ping(addr.ToString(false), addr.GetPort(), false);
											}

											else if (i != serverList.end())
												serverList.erase(i);

											peer->CloseConnection(packet->systemAddress, true, 0, LOW_PRIORITY);

											query = false;
											break;
										}

										case ID_UNCONNECTED_PONG:
										{
											BitStream query(packet->data, packet->length, false);
											query.IgnoreBytes(sizeof(MessageID));

											TimeMS ping;
											query.Read(ping);

											map<SystemAddress, ServerEntry>::iterator i;
											i = serverList.find(packet->systemAddress);

											if (i != serverList.end())
											{
												ServerEntry* entry = &i->second;
												entry->SetServerPing(GetTimeMS() - ping);
											}

											break;
										}

										case ID_NO_FREE_INCOMING_CONNECTIONS:
										case ID_CONNECTION_ATTEMPT_FAILED:
										{
											if (update && !lock)
											{
												peer->Connect(selectedServer->ToString(false), selectedServer->GetPort(), DEDICATED_VERSION, sizeof(DEDICATED_VERSION), 0, 0, 3, 500, 0);
												lock = true;
											}

											else
											{
												if (update)
												{
													map<SystemAddress, ServerEntry>::iterator i;
													i = serverList.find(*selectedServer);

													if (i != serverList.end())
														serverList.erase(*selectedServer);
												}

												query = false;
											}

											break;
										}

										case ID_INVALID_PASSWORD:
											if (update) MessageBox(nullptr, "MasterServer version mismatch.\nPlease download the most recent binaries from www.vaultmp.com", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_TASKMODAL);

											else MessageBox(nullptr, "Dedicated server version mismatch.\nPlease download the most recent binaries from www.vaultmp.com", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_TASKMODAL);

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
					}

					update = false;

					RefreshServerList();
					break;

				case IDC_BUTTON3:
				{
					/* RakNet File Transfer */

					if (selectedServer != nullptr && serverList.find(*selectedServer)->second.GetGame())
					{
						EnableWindow(wndsync, 0);

						int result = MessageBox(nullptr, "This function downloads files (mods) from the server. vaultmp has no control of which files get downloaded; this is up to the server configuration. Files will be placed in the \"Data\" folder of the corresponding game. Do NOT continue if you do not trust the server!", "Attention", MB_OKCANCEL | MB_ICONWARNING | MB_TOPMOST | MB_TASKMODAL);

						if (result == IDCANCEL)
						{
							EnableWindow(wndsync, 1);
							break;
						}

						SystemAddress server = *selectedServer;
						buf = &serverList.find(*selectedServer)->second;
						tcp->Connect(server.ToString(false), server.GetPort(), false);
						RakSleep(500);
						server = tcp->HasCompletedConnectionAttempt();

						if (server == UNASSIGNED_SYSTEM_ADDRESS)
						{
							MessageBox(nullptr, "Could not establish a connection to the fileserver. The server probably has file downloading disabled or its number of maximum parallel connections reached.", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_TASKMODAL);
							EnableWindow(wndsync, 1);
							break;
						}

						char rdy[3];
						rdy[0] = RAKNET_FILE_RDY;
						*((unsigned short*)(rdy + 1)) = flt->SetupReceive(&transferCallback, false, server);
						tcp->Send(rdy, sizeof(rdy), server, false);
						Packet* packet;

						while (buf)
						{
							packet = tcp->Receive();
							tcp->DeallocatePacket(packet);
							RakSleep(5);
						}

						tcp->CloseConnection(server);

						EnableWindow(wndsync, 1);

						MessageBox(nullptr, "Successfully synchronized with the server!", "Success", MB_OK | MB_ICONINFORMATION | MB_TOPMOST | MB_TASKMODAL);
					}

					break;
				}

				case IDC_CHECK0:
					if (SendMessage(wndchiptune, BM_GETCHECK, 0, 0))
						uFMOD_PlaySong(MAKEINTRESOURCE(CHIPTUNE), GetModuleHandle(nullptr), XM_RESOURCE);

					else
						uFMOD_StopSong();

					break;
			}

			break;

		case MSG_MINTRAYICON:
			if (wParam == ICON_MAIN && lParam == WM_LBUTTONUP)
				Maximize(hwnd);

			break;

		case WM_NOTIFY:
			switch (((LPNMHDR) lParam)->code)
			{
				case (LVN_FIRST - 14):   // LVN_ITEMACTIVATE
				{
					HWND hwndFrom = (HWND)((LPNMHDR) lParam)->hwndFrom;

					if (hwndFrom == wndlistview)
					{
						LVITEM item;
						item.mask = LVIF_PARAM;
						item.iItem = ListView_GetNextItem(hwndFrom, -1, LVNI_SELECTED);
						item.iSubItem = 0;
						ListView_GetItem(hwndFrom, &item);

						selectedServer = (SystemAddress*) item.lParam;

						SendMessage(wndlistview2, LVM_DELETEALLITEMS, 0, 0);

						map<SystemAddress, ServerEntry>::iterator i;
						i = serverList.find(*selectedServer);

						if (i != serverList.end())
						{

						    ServerEntry* entry = &i->second;
						    std::vector<string> modfiles = entry->GetServerModFiles();

							for(unsigned p = 0; p < modfiles.size(); ++p)
							{
							    RakString _mod_name(modfiles.at(p).c_str());
							    char mod_number[8],mod_name[strlen(_mod_name.C_String())];
							    snprintf(mod_number,8,"Mod %d",p+1);
							    strcpy(mod_name, _mod_name.C_String());
							    Create2ColItem(wndlistview2, mod_number, mod_name);
							}


							std::map<string, string> rules = entry->GetServerRules();

							for (map<string, string>::iterator k = rules.begin(); k != rules.end(); ++k)
							{
								string key = k->first;
								string value = k->second;

								char c_key[key.length()];
								char c_value[value.length()];

								strcpy(c_key, key.c_str());
								strcpy(c_value, value.c_str());

								Create2ColItem(wndlistview2, c_key, c_value);
							}

						}
					}

					break;
				}

				case LVN_COLUMNCLICK:
				{
					NMLISTVIEW* nmlv = (NMLISTVIEW*) lParam;
					wndsortcur = (HWND)((LPNMHDR) lParam)->hwndFrom;
					ListView_SortItemsEx(wndsortcur, CompareProc, nmlv->iSubItem);
					sort_flag = !sort_flag;
					break;
				}
			}

			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}
