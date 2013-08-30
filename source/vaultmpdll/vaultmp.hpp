#include "../Pipe.hpp"
#include "../Utils.hpp"
#include "../vaultmp.hpp"

// Fallout 3 addresses

static const unsigned int LOOKUP_FORM        = 0x00455190; // Credit goes to FOSE
static const unsigned int LOOKUP_FUNC        = 0x00519AF0;
static const unsigned int QUEUE_UI_MESSAGE   = 0x0061B850; // Credit goes to FOSE
static const unsigned int ALERTED_STATE      = 0x006F6C70;
static const unsigned int SNEAKING_STATE     = 0x006F58B0;
static const unsigned int SETPOS             = 0x006F2050;

