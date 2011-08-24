#ifndef DATA_H
#define DATA_H

#include <string>
#include <list>
#include <vector>
#include <map>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakString.h"
#include "RakNet/MessageIdentifiers.h"

#define PLAYER_REFERENCE    0x00000014
#define PLAYER_BASE         0x00000007

using namespace std;
using namespace RakNet;

/* Shared data structures and tables */

namespace Data
{

typedef void (*ResultHandler)(signed int, vector<double>, double);
typedef vector<string> (*RetrieveParamVector)();
typedef bool (*RetrieveBooleanFlag)();
typedef pair<vector<string>, RetrieveParamVector> Parameter;
typedef list<Parameter> ParamList;
typedef pair<ParamList, RetrieveBooleanFlag> ParamContainer;

static bool AlwaysTrue()
{
    return true;
}

static vector<string> EmptyVector()
{
    return vector<string>();
}

static Parameter BuildParameter(string param)
{
    vector<string> params;
    params.push_back(param);
    Parameter Param_Param = Parameter(params, &Data::EmptyVector);
    return Param_Param;
}

static const char* True[] = {"1"};
static const char* False[] = {"0"};

static vector<string> data_True(True, True + 1);
static vector<string> data_False(False, False + 1);
static Parameter Param_True = Parameter(data_True, &EmptyVector);
static Parameter Param_False = Parameter(data_False, &EmptyVector);

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
    CHANNEL_DATA,
};

#pragma pack(push, 1)

struct str_compare
{
    bool operator() (const char* a, const char* b)
    {
        return (stricmp(a, b) < 0);
    }
};

struct Item
{
    map<const char*, const char*, str_compare>::iterator item;
    int count;
    int type;
    float condition;
    bool worn;

    Item()
    {
        count = 0;
        type = 0;
        condition = 0.00;
        worn = false;
    }
};

struct pActorUpdate
{
    unsigned char type;
    RakNetGUID guid;
    float X, Y, Z, A;
    bool alerted;
    int moving;
};

struct pActorStateUpdate
{
    unsigned char type;
    RakNetGUID guid;
    float health;
    float baseHealth;
    float conds[6];
    bool dead;
};

struct pActorCellUpdate
{
    unsigned char type;
    RakNetGUID guid;
    int cell;
};

struct pActorItemUpdate
{
    unsigned char type;
    bool hidden;
    RakNetGUID guid;
    char baseID[8];
    Item item;
};

#pragma pack(pop)

}

#endif
