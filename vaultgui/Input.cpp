#include <Windows.h>
#include <stdio.h>

// source http://www.codeproject.com/KB/DLL/keyboardhook.aspx

FILE* f1;
HHOOK hkb;

int isChatinputOpen = 0;
char iBuf[5000] = {0};
extern char textbox_buffer[10000];

LRESULT __declspec( dllexport )__stdcall CALLBACK KeyboardProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	char ch;

	if ( ( ( DWORD )lParam & 0x40000000 ) && ( HC_ACTION == nCode ) )
	{
		if ( ( wParam == VK_SPACE ) || ( wParam == VK_RETURN ) || ( wParam == VK_BACK ) || ( wParam == VK_DELETE ) || ( ( wParam >= 0x2f ) && ( wParam <= 0x100 ) ) )
		{
			if ( wParam == VK_RETURN )
			{
				if( isChatinputOpen )
				{
					strcat( textbox_buffer, iBuf );
					strcat( textbox_buffer, "\n" );
					isChatinputOpen = 0;
					// TODO: send chat msg
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
					if( ch == 't' ) // TODO: make hotkey configurable
					{
						isChatinputOpen = 1;
						return 1;
					}
				}

				else
				{
					if( strlen( iBuf ) < sizeof( iBuf ) - 1 )
						strcat( iBuf, tbuf );

					return 1;
				}
			}
		}
	}

	return CallNextHookEx( hkb, nCode, wParam, lParam );
}

BOOL installhook()
{
	//create a thread to make sure it's hooked
	hkb = SetWindowsHookEx( WH_KEYBOARD_LL, ( HOOKPROC )KeyboardProc, GetModuleHandle( NULL ), 0 );
	// TODO: hook mouse input and intercept it when GUI mouse is enabled

	return TRUE;
}
