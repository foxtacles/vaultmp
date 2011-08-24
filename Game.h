#ifndef GAME_H
#define GAME_H

#include "vaultmp.h"
#include "PacketTypes.h"
#include "Interface.h"
#include "Player.h"
#include "Network.h"
#include "VaultException.h"

class Game
{
friend class NetworkClient;
friend class Bethesda;

private:

    Game();

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    static Player* self;
    static RakNetGUID server;
    static list<Player*> refqueue;

    static NetworkResponse Authenticate();

    static void InitializeCommands();

public:

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif
};

#endif
