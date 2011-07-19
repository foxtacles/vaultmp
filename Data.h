#ifndef DATA_H
#define DATA_H

class Player;

#include <string>
#include <map>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakString.h"
#include "RakNet/MessageIdentifiers.h"

#define FALLOUT3_TICKS 1

using namespace std;
using namespace RakNet;

/* Shared data structures and tables */

namespace Data {

enum {
    X_AXIS,
    Y_AXIS,
    Z_AXIS,
};

enum {
    COND_PERCEPTION,
    COND_ENDURANCE,
    COND_LEFTATTACK,
    COND_RIGHTATTACK,
    COND_LEFTMOBILITY,
    COND_RIGHTMOBILITY,
};

enum {
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

enum {
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

enum {
    CHANNEL_SYSTEM,
    CHANNEL_PLAYER_UPDATE,
    CHANNEL_PLAYER_STATE_UPDATE,
    CHANNEL_PLAYER_CELL_UPDATE,
    CHANNEL_PLAYER_ITEM_UPDATE,
};

#pragma pack(push, 1)

struct str_compare { bool operator() (const char* a, const char* b) { return (stricmp(a, b) < 0); } };

struct Item {
    map<const char*, const char*, str_compare>::iterator item;
    int count;
    int type;
    float condition;
    bool worn;

    Item() {
        count = 0;
        type = 0;
        condition = 0.00;
        worn = false;
    }
};

struct pPlayerUpdate {
    unsigned char type;
    RakNetGUID guid;
    float X, Y, Z, A;
    bool alerted;
    int moving;
};

struct pPlayerStateUpdate {
    unsigned char type;
    RakNetGUID guid;
    float health;
    float baseHealth;
    float conds[6];
    bool dead;
};

struct pPlayerCellUpdate {
    unsigned char type;
    RakNetGUID guid;
    DWORD cell;
};

struct pPlayerItemUpdate {
    unsigned char type;
    bool hidden;
    RakNetGUID guid;
    char baseID[8];
    Item item;
};

#pragma pack(pop)

struct fCommand {
    string command;
    Player* player;
    bool repeat;
    bool forplayers;
    bool enabledonly;
    int skipflag;
    int priority;
    int curPriority;
    int sleep;
    int tcount;

    fCommand() {
        command = "";
        player = NULL;
        repeat = false;
        forplayers = false;
        enabledonly = true;
        skipflag = -1;
        priority = 0;
        curPriority = 0;
        sleep = FALLOUT3_TICKS;
        tcount = GetTickCount();
    }
};

enum fCommandSkipFlags {
    SKIPFLAG_GETPOS_X = 0,
    SKIPFLAG_GETPOS_Y,
    SKIPFLAG_GETPOS_Z,
    SKIPFLAG_GETDEAD,
    SKIPFLAG_GETHEALTH,
    SKIPFLAG_GETPARENTCELL,
    SKIPFLAG_ITEMUPDATE,
    MAX_SKIP_FLAGS,
};

}

#endif
