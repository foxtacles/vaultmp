#include "../Pipe.h"
#include "../Utils.h"
#include "../vaultmp.h"

static const unsigned int LOOKUP_FORM_FALLOUT3        = 0x00455190; // Credit goes to FOSE
static const unsigned int LOOKUP_FUNC_FALLOUT3        = 0x00519AF0;
static const unsigned int ITEM_COUNT_FALLOUT3         = 0x004E85E0;
static const unsigned int ITEM_GET_FALLOUT3           = 0x004E8570;
static const unsigned int ITEM_ISEQUIPPED_FALLOUT3    = 0x0047D260;
static const unsigned int ITEM_CONDITION_FALLOUT3     = 0x0047CF30;
// has probably something to do with memory freeing
static const unsigned int ITEM_UNK1_FALLOUT3          = 0x0047FD30;
static const unsigned int ITEM_UNK2_FALLOUT3          = 0x0086BA60;
static const unsigned int ITEM_UNK3_FALLOUT3          = 0x01090A78;

static const unsigned int LOOKUP_FORM_NEWVEGAS        = 0x004839C0; // Credit goes to NVSE
static const unsigned int LOOKUP_FUNC_NEWVEGAS        = 0x005B1120;
static const unsigned int ITEM_COUNT_NEWVEGAS         = 0x00575590;
static const unsigned int ITEM_GET_NEWVEGAS           = 0x005754A0;
static const unsigned int ITEM_ISEQUIPPED_NEWVEGAS    = 0x004BDDD0;
static const unsigned int ITEM_CONDITION_NEWVEGAS     = 0x004BCDB0;
static const unsigned int ITEM_UNK1_NEWVEGAS          = 0x004459E0;

static const unsigned int LOOKUP_FORM_OBLIVION        = 0x0046B250; // Credit goes to OBSE
static const unsigned int LOOKUP_FUNC_OBLIVION        = 0x004FCA30;
