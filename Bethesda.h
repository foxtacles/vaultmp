#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <list>
#include <vector>
#include <string>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakSleep.h"

#include "vaultmp.h"
#include "Player.h"
#include "Inventory.h"
#include "Interface.h"
#include "Lockable.h"
#include "Network.h"
#include "VaultException.h"
#include "Data.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace RakNet;
using namespace Data;
using namespace std;

class Bethesda
{
friend class Network;

private:
    static int game;
    static string savegame;
    static bool initialized;

    static void InitializeGame();
    static void InitializeCommands();

    static DWORD WINAPI InjectedCode(LPVOID addr);
    static void InjectedEnd();
    struct INJECT;

    static Player* self;
    static list<Player*> refqueue;

    static void CommandHandler(signed int key, vector<double> info, double result);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    Bethesda();
public:
    static void InitializeVaultMP(RakPeerInterface* peer, SystemAddress server, string name, string pwd, int game);

};
