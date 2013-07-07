#include <Windows.h>
#include <iostream>
#include <fstream>

#include "common.h"

#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include "shlobj.h"

#include <sstream>

#include "myDirectDevice.h"

#include "Export.h"


using namespace std;

void ResetLog()
{
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
	{
		PathAppend( szPath, "\\vmp_gui.log" );
		ofstream o(szPath,ios::out);
		o.close();
	}
}

void SendToLog(char* str)
{
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
	{
		PathAppend( szPath, "\\vmp_gui.log" );
		ofstream o(szPath,ios::out|ios::app);
		o<<str<<endl;
		o.flush();
		o.close();
	}

}

char* ExceptionToString(PEXCEPTION_RECORD p)
{
	static char buffer[2056];
	buffer[0]=0;

	std::stringstream ss;
	ss << "Exception Address:"<<hex<<"0x"<<(int)p->ExceptionAddress<<endl;
	const std::string s = ss.str();

	strcpy(buffer,s.c_str());

	return buffer;
}

bool GetScreenPosition(D3DXVECTOR2& ScreenPosition, float& Distance, D3DXVECTOR3 PositionOffset = D3DXVECTOR3(0,0,0))
{
	D3DXMATRIX m;
	D3DXVECTOR4 s;
	D3DXVECTOR3 w;

	w = D3DXVECTOR3(0,0,0) + PositionOffset;

	//VIEW * PROJECTION
	m = D3DXMATRIX(GameData::lastView) * D3DXMATRIX(GameData::lastProjection);

	//get vectors from view * proj
	s.x = w.x * m._11 + w.y * m._21 + w.z * m._31 + m._41;
	s.y = w.x * m._12 + w.y * m._22 + w.z * m._32 + m._42;
	s.z = w.x * m._13 + w.y * m._23 + w.z * m._33 + m._43;
	s.w = w.x * m._14 + w.y * m._24 + w.z * m._34 + m._44;

	//get inverse of w
	float w_inv = 1.0f / s.w;
	//get center of screen
	float fWidth2 = GameData::windowWidth * 0.5f;
	float fHeight2 = GameData::windowHeight * 0.5f;

	//get screen coordinates
	ScreenPosition.x = (1.0f + (s.x * w_inv)) * fWidth2;
	ScreenPosition.y = GameData::windowHeight - ((1.0f + (s.y * w_inv)) * fHeight2);
	//get distance to object from camera position
	Distance = s.z;

	return (s.z > 0.0f);
}


void CreateDevice(LPVOID ths, BOOL isKeyboard)
{

}