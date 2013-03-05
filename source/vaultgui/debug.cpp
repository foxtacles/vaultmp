#include "debug.h"

namespace Debug
{
	vector<string> callStack;

	void FunctionCall(string str)
	{
		/*callStack.push_back(str);*/
	}

	void FunctionReturn(string str)
	{
		/*if(callStack.size()>0)
		{
			if(callStack[callStack.size()-1]!=str)
			{
				//Error!
			}
			callStack.erase(callStack.end());
		}*/
	}



	void InspectCrash(EXCEPTION_POINTERS* pExcPtrs)
	{
		HMODULE hDbgHelp = NULL;
		HANDLE hFile = NULL;
		MINIDUMP_EXCEPTION_INFORMATION mei;
		MINIDUMP_CALLBACK_INFORMATION mci;
    
		hDbgHelp = LoadLibrary("dbghelp.dll");
		if(hDbgHelp==NULL)
		{
			return;
		}

		hFile = CreateFile(
			"crashdump_vmp.dmp",
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if(hFile==INVALID_HANDLE_VALUE)
		{
			return;
		}
   
		mei.ThreadId = GetCurrentThreadId();
		mei.ExceptionPointers = pExcPtrs;
		mei.ClientPointers = FALSE;
		mci.CallbackRoutine = NULL;
		mci.CallbackParam = NULL;

		typedef BOOL (WINAPI *LPMINIDUMPWRITEDUMP)(
			HANDLE hProcess, 
			DWORD ProcessId, 
			HANDLE hFile, 
			MINIDUMP_TYPE DumpType, 
			CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
			CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam, 
			CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

		LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump = 
			(LPMINIDUMPWRITEDUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
		if(!pfnMiniDumpWriteDump)
		{    
			return;
		}

		HANDLE hProcess = GetCurrentProcess();
		DWORD dwProcessId = GetCurrentProcessId();

		BOOL bWriteDump = pfnMiniDumpWriteDump(
			hProcess,
			dwProcessId,
			hFile,
			MiniDumpNormal,
			&mei,
			NULL,
			&mci);

		if(!bWriteDump)
		{    
			return;
		}

		CloseHandle(hFile);

		FreeLibrary(hDbgHelp);


		/*
		char tmp[500];
		tmp[0]=0;
		strcat(tmp,"Crash Call Stack : VaultGUI ");
		for(int i=0;i<callStack.size();i++)
		{
			strcat(tmp,"-> ");
			strcat(tmp,callStack[i].c_str());
		}
		SendToLog(tmp);*/
	}
};