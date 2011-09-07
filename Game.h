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

    static unsigned char game;
    static RakNetGUID server;

    static void Initialize();
    static void LoadGame(string savegame);
    static void NewPlayer(NetworkID id, unsigned int baseID, string name);
    static void PlayerLeft(NetworkID id);
    static void Enable(NetworkID id, bool enable);
    static void Delete(NetworkID id);
    static void SetName(NetworkID id, string name);
    static void SetName(unsigned int refID);
    static void SetRestrained(NetworkID id, bool restrained, unsigned int delay = 2);
    static void SetPos(NetworkID id, unsigned char axis, double value);
    static void SetAngle(NetworkID id, unsigned char axis, double value);
    static void SetAngle(unsigned int refID, unsigned char axis);
    static void SetNetworkCell(NetworkID id, unsigned int cell);
    static void SetActorValue(NetworkID id, bool base, unsigned char index, double value);
    static void SetActorState(NetworkID id, unsigned char index, bool alerted);
    static void MoveTo(NetworkID id, NetworkID id2, bool cell = false);

    static NetworkResponse Authenticate(string password);
    static void PlaceAtMe(Lockable* data, unsigned int refID);
    static void GetPos(unsigned int refID, unsigned char axis, double value);
    static void GetAngle(unsigned int refID, unsigned char axis, double value);
    static void GetParentCell(unsigned int refID, unsigned int cell);
    static void GetActorValue(unsigned int refID, bool base, unsigned char index, double value);
    static void GetActorState(unsigned int refID, unsigned char index, bool alerted);

    static void Failure_PlaceAtMe(unsigned int refID, unsigned int baseID, unsigned int count, signed int key);

public:

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif
};

#endif
