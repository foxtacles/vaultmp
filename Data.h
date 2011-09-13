#ifndef DATA_H
#define DATA_H

#include <string>
#include <list>
#include <vector>
#include <map>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakString.h"
#include "RakNet/MessageIdentifiers.h"

#include "VaultFunctor.h"

#define SAFE_FIND(a,b) ((a.find(b) == a.end()) ? throw VaultException("Value %02X not defined in database", b) : a.find(b))

static const unsigned int PLAYER_REFERENCE  = 0x00000014;
static const unsigned int PLAYER_BASE       = 0x00000007;

using namespace std;
using namespace RakNet;

/* Shared data structures and tables */

namespace Data
{

typedef void (*ResultHandler)(signed int, vector<double>&, double, bool);
typedef bool (*RetrieveBooleanFlag)();
typedef pair<vector<string>, VaultFunctor*> Parameter;
typedef list<Parameter> ParamList;
typedef pair<ParamList, RetrieveBooleanFlag> ParamContainer;

static bool AlwaysTrue()
{
    return true;
}

static Parameter BuildParameter(string param)
{
    return Parameter(vector<string>{param}, NULL);
}

static Parameter BuildParameter(vector<string> params)
{
    return Parameter(params, NULL);
}

static Parameter BuildParameter(vector<unsigned char> params)
{
    vector<unsigned char>::iterator it;
    vector<string> convert;

    for (it = params.begin(); it != params.end(); ++it)
    {
        char value[64];
        snprintf(value, sizeof(value), "%d", *it);
        convert.push_back(string(value));
    }

    return BuildParameter(convert);
}

static Parameter BuildParameter(unsigned int param)
{
    char value[64];
    snprintf(value, sizeof(value), "%d", param);
    return BuildParameter(string(value));
}

static Parameter BuildParameter(double param)
{
    char value[64];
    snprintf(value, sizeof(value), "%f", param);
    return BuildParameter(string(value));
}

static const Parameter Param_True = Parameter(vector<string>{"1"}, NULL);
static const Parameter Param_False = Parameter(vector<string>{"0"}, NULL);

static void FreeContainer(ParamContainer& param)
{
    ParamList::iterator it;

    for (it = param.first.begin(); it != param.first.end(); ++it)
        if (it->second)
            delete it->second;
}

enum
{
    ID_MASTER_QUERY = ID_USER_PACKET_ENUM,
    ID_MASTER_ANNOUNCE,
    ID_MASTER_UPDATE,
    ID_GAME_FIRST,
};

enum
{
    CHANNEL_SYSTEM,
    CHANNEL_GAME,
    CHANNEL_CHAT,
};

}

#endif
