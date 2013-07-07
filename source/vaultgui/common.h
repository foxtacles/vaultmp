#pragma once

#define USE_CEGUI

#include <d3d9.h>
#include <d3dx9.h>

#include <vector>
#include <string>

#include <windowsx.h>

using namespace std;

#define D_MAX_CHAT_ENTRIES 32

/*#define SCREEN_WIDTH 1680
#define SCREEN_HEIGHT 1050*/

struct PlayerScreenName
{
	string name;
	int *posX;
	int *posY;
	int *posZ;
};

#include "GameData.h"
#include "global.h"
#include "taskManager.h"

#include "debug.h"

#ifdef USE_CEGUI

	#include "CEGUI\cegui\include\CEGUI.h"
	#include "CEGUI\cegui\include\RendererModules\Direct3D9\CEGUIDirect3D9Renderer.h"
	#include "CEGUI\cegui\include\CEGUISystem.h"
	#include "CEGUI\cegui\include\CEGUISchemeManager.h"
	#include "CEGUI\cegui\include\CEGUIcolour.h"
	#include "FormattedListboxTextItem.h"

#endif

#define IsHex(a) ((a>='0'&&a<='9')||(a>='A'&&a<='F')||(a>='a'&&a<='f'))

void ResetLog();
void SendToLog(char* str);
char* ExceptionToString(PEXCEPTION_RECORD p);
bool GetScreenPosition(D3DXVECTOR2& ScreenPosition, float& Distance, D3DXVECTOR3 PositionOffset);
void CreateDevice(LPVOID ths, BOOL isKeyboard);