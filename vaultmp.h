#ifndef VAULTMP_H
#define VAULTMP_H

#ifndef _GLIBCXX_USE_NANOSLEEP
#define _GLIBCXX_USE_NANOSLEEP // remove that
#endif

#ifndef THREAD_PRIORITY_NORMAL
#define THREAD_PRIORITY_NORMAL 1000
#endif


#define NO_CS_TIMEOUT

#define DEDICATED_VERSION       "0.1a revision 148"
#define MASTER_VERSION          "0.1a revision 145"
#define CLIENT_VERSION          "0.1a revision 148"

static const unsigned int FALLOUT3_EN_VER17            =   0x00E59528;
//static const unsigned int FALLOUT3_EN_VER17_STEAM      =
static const unsigned int NEWVEGAS_EN_VER14_STEAM      =   0x00FCB4FE;

static const unsigned int FOSE_VER0122                 =   0x0004E1B5;
static const unsigned int NVSE_VER0209                 =   0x00065388;

static const unsigned int VAULTMP_DLL                  =   0x0003D5E4;
static const unsigned int XLIVE_PATCH                  =   0x0000D57E;

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
static const unsigned char FALLOUT_GAMES        =   FALLOUT3 | NEWVEGAS;
static const unsigned char ALL_GAMES            =   FALLOUT_GAMES;

static const unsigned int MAX_PLAYER_NAME       =   16;
static const unsigned int MAX_PASSWORD_SIZE     =   16;
static const unsigned int MAX_MASTER_SERVER     =   32;
static const unsigned int MAX_SAVEGAME_FILE     =   64;
static const unsigned int MAX_MOD_FILE          =   64;

static const unsigned short VAULTFUNCTION       =   0xE000;

static const unsigned int PIPE_LENGTH           =   2048;
static const unsigned char PIPE_SYS_WAKEUP      =   0x01;
static const unsigned char PIPE_OP_COMMAND      =   0x02;
static const unsigned char PIPE_OP_RETURN       =   0x03;
static const unsigned char PIPE_OP_RETURN_BIG   =   0x04;
static const unsigned char PIPE_ERROR_CLOSE     =   0x05;
static const unsigned char PIPE_GUI_MESSAGE     =   0x06;

static const unsigned int RAKNET_FILE_SERVER    =   1550;
static const unsigned char RAKNET_FILE_RDY      =   0x01;
static const unsigned char FILE_SAVEGAME        =   0x02;
static const unsigned char FILE_MODFILE         =   0x03;

#ifndef __WIN32__
#define stricmp strcasecmp
#define ZeroMemory(a, b) memset(a, 0, b)
#endif

#endif
