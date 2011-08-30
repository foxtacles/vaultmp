#ifndef VAULTMP_H
#define VAULTMP_H

#define DEDICATED_VERSION       "0.1a revision 142"
#define MASTER_VERSION          "0.1a revision 142"
#define CLIENT_VERSION          "0.1a revision 142"

#define CREDITS \
"Vault-Tec Multiplayer Mod is an Open-Source project.\n\
\n\
code: Recycler (www.brickster.net)\n\
network: RakNet (www.jenkinssoftware.com)\n\
scripting: The PAWN language (www.compuphase.com)\n\
music: uFMOD (ufmod.sourceforge.net)\n\
\n\
Greetings fly out to:\n\
mqidx, benG, ArminSeiko\n\
\n\
Thanks to everyone who contributed :-)\n\
\n\
www.vaultmp.com"

#define FALLOUT3                0x01
#define NEWVEGAS                0x02
#define OBLIVION                0x04
#define FALLOUT_GAMES           0x03
#define ALL_GAMES               0x07

#define MAX_PLAYER_NAME         16
#define MAX_PASSWORD_SIZE       16
#define MAX_MASTER_SERVER       32
#define MAX_SAVEGAME_FILE       64
#define MAX_MOD_FILE            64

#define PIPE_LENGTH             300
#define PIPE_SYS_WAKEUP         0x10
#define PIPE_OP_COMMAND         0x11
#define PIPE_OP_RETURN          0x12
#define PIPE_ERROR_CLOSE        0x13
#define PIPE_GUI_MESSAGE        0x14

#define RAKNET_FILE_SERVER      1550
#define RAKNET_FILE_RDY         0x01
#define FILE_SAVEGAME           0x10
#define FILE_MODFILE            0x20

#ifndef WIN32
#define stricmp strcasecmp
#define ZeroMemory(a, b) memset(a, 0, b)
#endif

#endif
