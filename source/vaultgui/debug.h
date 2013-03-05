#pragma once

#include "common.h"
#include <DbgHelp.h>

namespace Debug
{
	extern vector<string> callStack;

	void FunctionCall(string str);

	void FunctionReturn(string str);



	void InspectCrash(EXCEPTION_POINTERS* pExcPtrs);
};