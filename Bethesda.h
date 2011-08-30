#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <list>
#include <vector>
#include <string>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/BitStream.h"
#include "RakNet/RakSleep.h"

#include "Data.h"
#include "vaultmp.h"
#include "Player.h"
#include "Container.h"
#include "Interface.h"
#include "Lockable.h"
#include "Game.h"
#include "VaultException.h"
#include "NetworkClient.h"
#include "GameFactory.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace RakNet;
using namespace Data;
using namespace std;

typedef pair<string, unsigned int> Savegame;
typedef vector<pair<string, unsigned int> > ModList;

class Bethesda
{
friend class NetworkClient;
friend class Game;

private:
    static int game;
    static string password;
    static Savegame savegame;
    static ModList modfiles;
    static bool initialized;

    static void Initialize();
    static void CommandHandler(signed int key, vector<double> info, double result);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    Bethesda();
public:
    static void InitializeVaultMP(RakPeerInterface* peer, SystemAddress server, string name, string pwd, int game);

};
