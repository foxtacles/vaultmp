#include "../Pipe.h"
#include "../Utils.h"
#include "../vaultmp.h"

// Fallout 3 addresses

static const unsigned int LOOKUP_FORM        = 0x00455190; // Credit goes to FOSE
static const unsigned int LOOKUP_FUNC        = 0x00519AF0;
static const unsigned int QUEUE_UI_MESSAGE   = 0x0061B850; // Credit goes to FOSE
static const unsigned int ALERTED_STATE      = 0x006F6C70;
static const unsigned int SNEAKING_STATE     = 0x006F58B0;

static const unsigned int ITEM_COUNT         = 0x004E85E0;
static const unsigned int ITEM_GET           = 0x004E8570;
static const unsigned int ITEM_ISEQUIPPED    = 0x0047D260;
static const unsigned int ITEM_CONDITION     = 0x0047CF30;
static const unsigned int ITEM_UNK1          = 0x0047FD30;
static const unsigned int ITEM_UNK2          = 0x0086BA60;
static const unsigned int ITEM_UNK3          = 0x01090A78;
