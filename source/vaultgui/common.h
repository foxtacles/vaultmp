#pragma once
#include <d3d9.h>
#include <d3dx9.h>

#include <vector>
#include <string>

using namespace std;

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



#define IsHex(a) ((a>='0'&&a<='9')||(a>='A'&&a<='F')||(a>='a'&&a<='f'))

void ResetLog();
void SendToLog(char* str);
char* ExceptionToString(PEXCEPTION_RECORD p);
bool GetScreenPosition(D3DXVECTOR2& ScreenPosition, float& Distance, D3DXVECTOR3 PositionOffset);