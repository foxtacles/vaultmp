#include <Windows.h>
#include <stdio.h>
#include "Input.h"

// source http://www.codeproject.com/KB/DLL/keyboardhook.aspx

FILE* f1;
HHOOK hkb;

extern char inputToggle;
extern char mouseInputToggle;
int isChatinputOpen = 1;
int isChatMouseInputOpen = 1;
char iBuf[5000] = {0};
extern char textbox_buffer[10000];

char UpToLow(char chr) {
	if (chr >= 0x41 && chr <= 0x5A)
		return chr + 0x20;
	return chr;
}

char ChangeCase(char chr)
{
	if (chr >= 0x41 && chr <= 0x5A)
		return chr + 0x20;
	return chr - 0x20;
}

LRESULT __declspec( dllexport ) CALLBACK KeyboardProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	char ch;

	if ( ( ( DWORD )lParam & 0x40000000 ) && ( HC_ACTION == nCode ) )
	{
		if ( ( wParam == VK_SPACE ) || ( wParam == VK_RETURN ) || ( wParam == VK_BACK ) || ( wParam == VK_DELETE ) || ( wParam == VK_LBUTTON ) || ( ( wParam >= 0x2f ) && ( wParam <= 0x100 ) ) )
		{
			if ( wParam == VK_RETURN )
			{
				if( isChatinputOpen )
				{
					if(strlen(iBuf)>0)
					{
						strcat( textbox_buffer, iBuf );
						strcat( textbox_buffer, "\n" );
						// TODO: send chat msg
					}
					isChatinputOpen = 0;
					memset( iBuf, 0, sizeof( iBuf ) );
					return 1;
				}
			}

			else if( wParam == VK_BACK || wParam == VK_DELETE )
			{
				if( isChatinputOpen )
				{
					size_t i = strlen( iBuf );

					if( i > 0 )
						iBuf[i - 1] = 0;

					return 1;
				}
			}

			else if( wParam == VK_LBUTTON )
			{
				if(isChatMouseInputOpen)
				{
					return 1;
				}
			}

			else
			{
				BYTE ks[256];
				GetKeyboardState( ks );

				WORD w;
				UINT scan = 0;
				ToAscii( wParam, scan, ks, &w, 0 );
				ch = char( w );
				char tbuf[2] = {0};
				tbuf[0] = ch;

				if( !isChatinputOpen )
				{
					if(UpToLow(ch) == mouseInputToggle)
					{
						if(isChatMouseInputOpen)
							isChatMouseInputOpen = 0;
						else
							isChatMouseInputOpen = 1;
					}
					else if(UpToLow(ch) == inputToggle)
					{
						isChatinputOpen = 1;
						return 1;
					}
				}
				else
				{
					if((((WORD)GetKeyState(VK_CAPITAL)) & 0xffff) != 0)// caps lock is on
					{
						if(GetAsyncKeyState(VK_SHIFT) & 1) //shift is down
						{
							tbuf[0] = ChangeCase(ch);
						}
					}
					else//caps lock is off
					{
						if(GetAsyncKeyState(VK_SHIFT) & 1) //shift is down
						{
							tbuf[0] = ChangeCase(ch);
						}
					}
					if( strlen( iBuf ) < sizeof( iBuf ) - 1 )
						strcat( iBuf, tbuf );

					return 1;
				}
			}
		}
	}

	return 0;
}

typedef LRESULT (CALLBACK*tWndproc)(HWND,UINT,WPARAM,LPARAM);
tWndproc oWndproc=0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
		if(KeyboardProc(HC_ACTION, wParam, lParam))
			return 1;
		break;
	}
	return oWndproc(hWnd,message,wParam,lParam);
}

// TODO: export this with an argument for gameid
BOOL installhook(LPVOID)
{
	HWND hw=0; // TODO: search for different window names depending on GameID that can be defined by void DllExport SetGameID(BYTE id);
	while(!hw)
	{
		hw = FindWindowA(0,"Fallout3");
		Sleep(50);
	}
	while(!oWndproc)
	{
		oWndproc = (tWndproc)GetWindowLongA(hw, GWL_WNDPROC); // this will be very bad if a so called "handle" is returned instead of an actual address
		Sleep(50);
	}
	SetWindowLongA(hw, GWL_WNDPROC, (LONG)WndProc);

	return TRUE;
}