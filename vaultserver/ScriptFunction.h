#ifndef SCRIPTFUNCTION_H
#define SCRIPTFUNCTION_H

#include <string>

#include "boost/any.hpp"

#include "PAWN.h"

using namespace std;

typedef unsigned long long (*ScriptFunc)();
typedef string ScriptFuncPAWN;

class ScriptFunction
{
private:
    ScriptFunc fCpp;
    ScriptFuncPAWN fPawn;
    AMX* amx;

protected:
    string def;
    bool pawn;

    ScriptFunction(ScriptFunc fCpp, string def);
    ScriptFunction(ScriptFuncPAWN fPawn, AMX* amx, string def);

    unsigned long long Call(const vector<boost::any>& args);

};

#endif
