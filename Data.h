#ifndef DATA_H
#define DATA_H

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
    ID_PLAYER_UPDATE
};

#pragma pack(push, 1)
struct pPlayerUpdate {
    unsigned char type;
    RakNetGUID guid;
    float X, Y, Z, A;
    float health;
    float baseHealth;
    float conds[6];
    bool dead;
    bool alerted;
    int moving;
};
#pragma pack(pop)

struct fCommand {
    string command;
    string refID;
    bool repeat;
    bool forplayers;
    bool skipflag;
    int flipcmd;
    int sleep;

    fCommand() {
        command = "";
        refID = "";
        repeat = false;
        forplayers = false;
        skipflag = false;
        flipcmd = -1;
        sleep = FALLOUT3_TICKS;
    }
};

enum fCommandSkipFlags {
    SKIPFLAG_GETPOS_X = 0,
    SKIPFLAG_GETPOS_Y,
    SKIPFLAG_GETPOS_Z,
    MAX_SKIP_FLAGS
};

}

#endif
