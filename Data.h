#ifndef DATA_H
#define DATA_H

#include <string>
#include <list>
#include <vector>
#include <map>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakString.h"
#include "RakNet/MessageIdentifiers.h"

#define PLAYER_REFERENCE 0x00000014
#define FALLOUT3_TICKS 1

using namespace std;
using namespace RakNet;

class Player;

/* Shared data structures and tables */

namespace Data
{

typedef vector<string> (*RetrieveParamVector)(string);
typedef bool (*RetrieveBooleanFlag)();
typedef void (*ResultHandler)(vector<void*>);
typedef void (*StringHandler)(string);
typedef pair<vector<string>, RetrieveParamVector> Parameter;
typedef list<Parameter> ParamList;
typedef pair<ParamList, RetrieveBooleanFlag> ParamContainer;

static bool AlwaysTrue()
{
    return true;
}

static vector<string> EmptyVector(string cmd)
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
static const char* XYZ[] = {"X", "Y", "Z"};
static const char* X[] = {"X"};
static const char* Y[] = {"Y"};
static const char* Z[] = {"Z"};
static const char* BaseActorValues[] = {"Health"};
static const char* ActorValues[] = {"Health", "PerceptionCondition", "EnduranceCondition", "LeftAttackCondition", "RightAttackCondition", "LeftMobilityCondition", "RightMobilityCondition"};
static vector<string> data_True(True, True + 1);
static vector<string> data_False(False, False + 1);
static vector<string> data_XYZ(XYZ, XYZ + 3);
static vector<string> data_X(X, X + 1);
static vector<string> data_Y(Y, Y + 1);
static vector<string> data_Z(Z, Z + 1);
static vector<string> data_BaseActorValues(BaseActorValues, BaseActorValues + 1);
static vector<string> data_ActorValues(ActorValues, ActorValues + 7);

static Parameter Param_True = Parameter(data_True, &EmptyVector);
static Parameter Param_False = Parameter(data_False, &EmptyVector);
static Parameter Param_XYZ = Parameter(data_XYZ, &EmptyVector);
static Parameter Param_X = Parameter(data_X, &EmptyVector);
static Parameter Param_Y = Parameter(data_Y, &EmptyVector);
static Parameter Param_Z = Parameter(data_Z, &EmptyVector);
static Parameter Param_BaseActorValues = Parameter(data_BaseActorValues, &EmptyVector);
static Parameter Param_ActorValues = Parameter(data_ActorValues, &EmptyVector);

enum
{
    X_AXIS,
    Y_AXIS,
    Z_AXIS,
};

enum
{
    COND_PERCEPTION,
    COND_ENDURANCE,
    COND_LEFTATTACK,
    COND_RIGHTATTACK,
    COND_LEFTMOBILITY,
    COND_RIGHTMOBILITY,
};

enum
{
    MOV_IDLE,
    MOV_FASTFORWARD,
    MOV_FASTBACKWARD,
    MOV_FASTLEFT,
    MOV_FASTRIGHT,
    MOV_FORWARD,
    MOV_BACKWARD,
    MOV_LEFT,
    MOV_RIGHT,
};

enum
{
    ID_MASTER_QUERY = ID_USER_PACKET_ENUM,
    ID_MASTER_ANNOUNCE,
    ID_MASTER_UPDATE,
    ID_GAME_INIT,
    ID_GAME_RUN,
    ID_GAME_START,
    ID_GAME_END,
    ID_NEW_PLAYER,
    ID_PLAYER_LEFT,
    ID_PLAYER_UPDATE,
    ID_PLAYER_STATE_UPDATE,
    ID_PLAYER_CELL_UPDATE,
    ID_PLAYER_ITEM_UPDATE,
};

enum
{
    CHANNEL_SYSTEM,
    CHANNEL_PLAYER_UPDATE,
    CHANNEL_PLAYER_STATE_UPDATE,
    CHANNEL_PLAYER_CELL_UPDATE,
    CHANNEL_PLAYER_ITEM_UPDATE,
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

enum SkipFlags
{
    SKIPFLAG_GETPOS,
    SKIPFLAG_GETDEAD,
    SKIPFLAG_GETACTORVALUE,
    SKIPFLAG_GETPARENTCELL,
    SKIPFLAG_ITEMUPDATE,
    MAX_SKIP_FLAGS,
};

}

#endif
