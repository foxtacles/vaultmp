#ifndef VAULTMP_H
#define VAULTMP_H

//#define NO_CS_TIMEOUT

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

static const unsigned char FALLOUT3             =   0x01;
static const unsigned char NEWVEGAS             =   FALLOUT3 << 1;
static const unsigned char OBLIVION             =   NEWVEGAS << 1;
static const unsigned char FALLOUT_GAMES        =   FALLOUT3 | NEWVEGAS;
static const unsigned char ALL_GAMES            =   FALLOUT_GAMES | OBLIVION;

static const unsigned int MAX_PLAYER_NAME       =   16;
static const unsigned int MAX_PASSWORD_SIZE     =   16;
static const unsigned int MAX_MASTER_SERVER     =   32;
static const unsigned int MAX_SAVEGAME_FILE     =   64;
static const unsigned int MAX_MOD_FILE          =   64;

static const unsigned short VAULTFUNCTION       =   0xE000;

static const unsigned int PIPE_LENGTH           =   300;
static const unsigned char PIPE_SYS_WAKEUP      =   0x01;
static const unsigned char PIPE_OP_COMMAND      =   0x02;
static const unsigned char PIPE_OP_RETURN       =   0x03;
static const unsigned char PIPE_ERROR_CLOSE     =   0x04;
static const unsigned char PIPE_GUI_MESSAGE     =   0x05;

static const unsigned int RAKNET_FILE_SERVER    =   1550;
static const unsigned char RAKNET_FILE_RDY      =   0x01;
static const unsigned char FILE_SAVEGAME        =   0x02;
static const unsigned char FILE_MODFILE         =   0x03;

#ifndef WIN32
#define stricmp strcasecmp
#define ZeroMemory(a, b) memset(a, 0, b)
#endif

#endif
