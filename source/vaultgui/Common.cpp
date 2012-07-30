#include <Windows.h>
#include <iostream>
#include <fstream>

#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#include "shlobj.h"

#include <sstream>


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