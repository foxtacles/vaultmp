#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <queue>
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
#include "Command.h"
#include "Data.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace RakNet;
using namespace Data;
using namespace std;

class Bethesda
{

private:
    static bool NewVegas;
    static bool initialized;
    static string savegame;

    static bool InitializeFallout(bool NewVegas);
    static void InitializeCommands();

    static DWORD WINAPI InjectedCode(LPVOID addr);
    static void InjectedEnd();
    struct INJECT;

    static Player* self;
    static queue<Player*> refqueue;

    static void CommandHandler(vector<void*> command);
    static void StringHandler(string command);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    Bethesda();
public:
    static void InitializeVaultMP(RakPeerInterface* peer, SystemAddress addr, string name, string pwd, bool NewVegas);

};
