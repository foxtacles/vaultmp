#ifndef DATA_H
#define DATA_H

class Player;

#include "RakNet/MessageIdentifiers.h"

#define FALLOUT3_TICKS 1

using namespace RakNet;

/* Shared data structures and tables */

namespace Data {

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
};

enum {
    CHANNEL_SYSTEM,
    CHANNEL_PLAYER_UPDATE,
    CHANNEL_PLAYER_STATE_UPDATE,
    CHANNEL_PLAYER_CELL_UPDATE,
};

#pragma pack(push, 1)
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
    MAX_SKIP_FLAGS
};

}

#endif
