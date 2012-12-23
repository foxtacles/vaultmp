#ifndef SCRIPTFUNCTION_H
#define SCRIPTFUNCTION_H

#include <string>

#include "boost/any.hpp"

#include "PAWN.h"

typedef unsigned long long(*ScriptFunc)();
typedef std::string ScriptFuncPAWN;

class ScriptFunction
{
	private:
		ScriptFunc fCpp;
		ScriptFuncPAWN fPawn;
		AMX* amx;

	protected:
		std::string def;
		bool pawn;

		ScriptFunction(ScriptFunc fCpp, const std::string& def);
		ScriptFunction(ScriptFuncPAWN fPawn, AMX* amx, const std::string& def);

		unsigned long long Call(const std::vector<boost::any>& args);

};

#endif
