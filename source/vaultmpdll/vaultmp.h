#include "../Pipe.h"
#include "../Utils.h"
#include "../vaultmp.h"

// Fallout 3 addresses

static const unsigned int LOOKUP_FORM_FALLOUT3        = 0x00455190; // Credit goes to FOSE
static const unsigned int LOOKUP_FUNC_FALLOUT3        = 0x00519AF0;
static const unsigned int QUEUE_UI_MESSAGE_FALLOUT3   = 0x0061B850; // Credit goes to FOSE
static const unsigned int ALERTED_STATE_FALLOUT3      = 0x006F6C70;
static const unsigned int SNEAKING_STATE_FALLOUT3     = 0x006F58B0;
static const unsigned int ITEM_COUNT_FALLOUT3         = 0x004E85E0;
static const unsigned int ITEM_GET_FALLOUT3           = 0x004E8570;
static const unsigned int ITEM_ISEQUIPPED_FALLOUT3    = 0x0047D260;
static const unsigned int ITEM_CONDITION_FALLOUT3     = 0x0047CF30;
static const unsigned int SETRACE_A_FALLOUT3		  = 0x005556C0;
static const unsigned int SETRACE_B_FALLOUT3		  = 0x00729880; // maybe some kind of render/update function
// unk
static const unsigned int ITEM_UNK1_FALLOUT3          = 0x0047FD30;
static const unsigned int ITEM_UNK2_FALLOUT3          = 0x0086BA60;
static const unsigned int ITEM_UNK3_FALLOUT3          = 0x01090A78;

// New Vegas addresses

static const unsigned int LOOKUP_FORM_NEWVEGAS        = 0x004839C0; // Credit goes to NVSE
static const unsigned int LOOKUP_FUNC_NEWVEGAS        = 0x005B1120;
static const unsigned int QUEUE_UI_MESSAGE_NEWVEGAS   = 0x007052F0; // Credit goes to NVSE
static const unsigned int ALERTED_STATE_NEWVEGAS      = 0x008A16D0;
static const unsigned int SNEAKING_STATE_NEWVEGAS     = 0x004997B0;
static const unsigned int ITEM_COUNT_NEWVEGAS         = 0x00575590;
static const unsigned int ITEM_GET_NEWVEGAS           = 0x005754A0;
static const unsigned int ITEM_ISEQUIPPED_NEWVEGAS    = 0x004BDDD0;
static const unsigned int ITEM_CONDITION_NEWVEGAS     = 0x004BCDB0;
//unk
static const unsigned int ITEM_UNK1_NEWVEGAS          = 0x004459E0;
