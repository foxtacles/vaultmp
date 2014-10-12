#ifndef VAULTMP_H
#define VAULTMP_H

// FIXME
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

#ifndef THREAD_PRIORITY_NORMAL
#define THREAD_PRIORITY_NORMAL 1000
#endif

#define DEDICATED_VERSION "0.1a snapshot \"Gary 2.10\""
#define MASTER_VERSION "0.1a snapshot \"Gary 2.10\""
#define CLIENT_VERSION "0.1a snapshot \"Gary 2.10\""

static const unsigned int FALLOUT3_EN_VER17            =   0x00E59528;
static const unsigned int FOSE_VER0122                 =   0x0004E1B5;
static const unsigned int VAULTMP_DLL                  =   0x000368FD;
static const unsigned int VAULTMP_F3                   =   0x1C877592;
static const unsigned int XLIVE_PATCH                  =   0x0000D57E;

static const unsigned int MAX_PLAYER_NAME       =   16;
static const unsigned int MAX_PASSWORD_SIZE     =   16;
static const unsigned int MAX_MASTER_SERVER     =   32;
static const unsigned int MAX_MOD_FILE          =   64;
static const unsigned int MAX_CELL_NAME         =   36;
static const unsigned int MAX_MESSAGE_LENGTH    =   64;
static const unsigned int MAX_CHAT_LENGTH       =   128;

static const unsigned short VAULTFUNCTION       =   0xE000;

static const unsigned int PIPE_LENGTH           =   2048;
static const unsigned char PIPE_SYS_WAKEUP      =   0x01;
static const unsigned char PIPE_OP_COMMAND      =   0x02;
static const unsigned char PIPE_OP_RETURN       =   0x03;
static const unsigned char PIPE_OP_RETURN_BIG   =   0x04;
static const unsigned char PIPE_OP_RETURN_RAW	=   0x05;
static const unsigned char PIPE_ERROR_CLOSE     =   0x06;

static const unsigned int RAKNET_FILE_SERVER    =   1550;
static const unsigned char RAKNET_FILE_RDY      =   0x01;
static const unsigned char FILE_MODFILE         =   0x02;

#ifndef __WIN32__
#ifndef stricmp
  #define stricmp strcasecmp
#endif
#endif

#ifndef ZeroMemory
#define ZeroMemory(a, b) memset(a, 0, b)
#endif

#endif
