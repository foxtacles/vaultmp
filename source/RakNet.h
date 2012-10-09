#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-pedantic"
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "RakNet/RakPeerInterface.h"
#include "RakNet/PacketizedTCP.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/FileListTransfer.h"
#include "RakNet/FileListTransferCBInterface.h"
#include "RakNet/IncrementalReadInterface.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakString.h"
#include "RakNet/RakSleep.h"
#include "RakNet/GetTime.h"
#include "RakNet/gettimeofday.h"
#include "RakNet/NetworkIDObject.h"
#include "RakNet/NetworkIDManager.h"

#pragma GCC diagnostic pop
