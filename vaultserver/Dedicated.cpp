#include "Dedicated.h"

using namespace RakNet;
using namespace Data;
using namespace std;

RakPeerInterface* Dedicated::peer;
SocketDescriptor* Dedicated::sockdescr;
int Dedicated::port;
int Dedicated::connections;
AMX* Dedicated::amx;
char* Dedicated::announce;
bool Dedicated::query;
#ifdef VAULTMP_DEBUG
Debug* Dedicated::debug;
#endif

SystemAddress Dedicated::master;
TimeMS Dedicated::announcetime;

ServerEntry* Dedicated::self;

bool Dedicated::thread;

void Dedicated::TerminateThread()
{
    thread = false;
}

void Dedicated::SetServerName(string name)
{
    self->SetServerName(name);
}

void Dedicated::SetServerMap(string map)
{
    self->SetServerMap(map);
}

void Dedicated::SetServerRule(string rule, string value)
{
    self->SetServerRule(rule, value);
}

int Dedicated::GetGame()
{
    return self->GetGame();
}

void Dedicated::Announce(bool announce)
{
    if (peer->GetConnectionState(master) == IS_CONNECTED)
    {
        BitStream query;

        if (announce)
        {
            query.Write((MessageID) ID_MASTER_ANNOUNCE);
            query.Write((bool) true);

            RakString name(self->GetServerName().c_str());
            RakString map(self->GetServerMap().c_str());
            int players = self->GetServerPlayers().first;
            int playersMax = self->GetServerPlayers().second;
            int game = self->GetGame();
            std::map<string, string> rules = self->GetServerRules();

            query.Write(name);
            query.Write(map);
            query.Write(players);
            query.Write(playersMax);
            query.Write(game);
            query.Write(rules.size());

            for (std::map<string, string>::const_iterator i = rules.begin(); i != rules.end(); ++i)
            {
                RakString key(i->first.c_str());
                RakString value(i->second.c_str());
                query.Write(key);
                query.Write(value);
            }
        }
        else
        {
            query.Write((MessageID) ID_MASTER_ANNOUNCE);
            query.Write((bool) false);
        }

        peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, master, false, 0);
    }
    else
    {
        Utils::timestamp();
        printf("Lost connection to MasterServer (%s)\n", master.ToString());
        peer->Connect(master.ToString(false), master.GetPort(), MASTER_VERSION, sizeof(MASTER_VERSION), 0, 0, 3, 100, 0);
    }

    announcetime = GetTimeMS();
}

#ifdef __WIN32__
DWORD WINAPI Dedicated::DedicatedThread(LPVOID data)
#else
void* Dedicated::DedicatedThread(void* data)
#define THREAD_PRIORITY_NORMAL 1000
#endif
{
    sockdescr = new SocketDescriptor(port, 0);
    peer = RakPeerInterface::GetInstance();
    peer->SetIncomingPassword(DEDICATED_VERSION, sizeof(DEDICATED_VERSION));

    if (announce)
    {
        peer->Startup(connections + 1, sockdescr, 1, THREAD_PRIORITY_NORMAL);
        peer->SetMaximumIncomingConnections(connections);
        master.SetBinaryAddress(strtok(announce, ":"));
        char* cport = strtok(NULL, ":");
        master.SetPort(cport != NULL ? atoi(cport) : RAKNET_MASTER_STANDARD_PORT);
        peer->Connect(master.ToString(false), master.GetPort(), MASTER_VERSION, sizeof(MASTER_VERSION), 0, 0, 3, 500, 0);
        announcetime = GetTimeMS();
    }
    else
    {
        peer->Startup(connections, sockdescr, 1, THREAD_PRIORITY_NORMAL);
        peer->SetMaximumIncomingConnections(connections);
    }

#ifdef VAULTMP_DEBUG
    Debug* debug = new Debug((char*) "vaultserver");
    debug->PrintFormat("Vault-Tec Multiplayer Mod Dedicated server debug log (%s)", false, DEDICATED_VERSION);
    debug->PrintFormat("Local host: %s (game: %s)", false, peer->GetMyBoundAddress().ToString(), self->GetGame() == FALLOUT3 ? (char*) "Fallout 3" : self->GetGame() == NEWVEGAS ? (char*) "Fallout New Vegas" : (char*) "TES Oblivion");
    debug->Print("Visit www.vaultmp.com for help and upload this log if you experience problems with the mod.", false);
    debug->Print("-----------------------------------------------------------------------------------------------------", false);
    //debug->PrintSystem();
    //Player::SetDebugHandler(debug);
    VaultException::SetDebugHandler(debug);
    Network::SetDebugHandler(debug);
    Inventory::SetDebugHandler(debug);
#endif

    Packet* packet;

    Inventory::Initialize(self->GetGame());
    Actor::Initialize();
    Player::Initialize();
    Client::SetMaximumClients(connections);

    try
    {
        while (thread)
        {
            for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
            {
                try
                {
                    NetworkResponse response = Network::ProcessPacket(packet, peer->GetMyGUID());
                    Network::Dispatch(peer, response, master, true);
                }
                catch (...)
                {
                    peer->DeallocatePacket(packet);
                    NetworkResponse response = Network::ProcessEvent(ID_EVENT_SERVER_ERROR, peer->GetMyGUID());
                    Network::Dispatch(peer, response, UNASSIGNED_SYSTEM_ADDRESS, true);
                    throw;
                }
                switch (packet->data[0])
                {
                /*case ID_DISCONNECTION_NOTIFICATION:
                case ID_CONNECTION_LOST:
                {
                    Client* client = Client::GetClientFromGUID(packet->guid);
                    Player* player = Player::GetPlayerFromGUID(packet->guid);

                    int ret = 1;

                    if (client != NULL && player != NULL && amx != NULL)
                    {
                        void* args[1];

                        int id = client->GetClientID();

                        args[0] = reinterpret_cast<void*>(&id);

                        ret = Script::Call(amx, (char*) "OnPlayerDisconnect", (char*) "i", args);
                    }

                    if (player != NULL)
                    {
                        delete player;

                        map<RakNetGUID, Player*> players = Player::GetPlayerList();
                        map<RakNetGUID, Player*>::iterator it;

                        BitStream query(packet->data, packet->length, false);
                        query.IgnoreBytes(sizeof(MessageID));
                        query.Reset();

                        query.Write((MessageID) ID_PLAYER_LEFT);
                        query.Write(packet->guid);

                        for (it = players.begin(); it != players.end(); ++it)
                        {
                            RakNetGUID guid = it->first;
                            peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, guid, false, 0);
                        }
                    }

                    if (client != NULL)
                    {
                        delete client;
                        self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), connections));
                    }
                    break;
                }
                case ID_MASTER_UPDATE:
                {
                    if (query)
                    {
                        BitStream query(packet->data, packet->length, false);
                        query.IgnoreBytes(sizeof(MessageID));

                        SystemAddress addr;
                        query.Read(addr);
                        query.Reset();

                        query.Write((MessageID) ID_MASTER_UPDATE);
                        query.Write(addr);

                        RakString name(self->GetServerName().c_str());
                        RakString map(self->GetServerMap().c_str());
                        int players = self->GetServerPlayers().first;
                        int playersMax = self->GetServerPlayers().second;
                        std::map<string, string> rules = self->GetServerRules();

                        query.Write(name);
                        query.Write(map);
                        query.Write(players);
                        query.Write(playersMax);
                        query.Write((int) rules.size());

                        for (std::map<string, string>::const_iterator i = rules.begin(); i != rules.end(); ++i)
                        {
                            RakString key(i->first.c_str());
                            RakString value(i->second.c_str());
                            query.Write(key);
                            query.Write(value);
                        }

                        peer->Send(&query, LOW_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, packet->systemAddress, false, 0);

                        Utils::timestamp();
                        printf("Query processed (%s)\n", packet->systemAddress.ToString());
                        break;
                    }
                    else
                    {
                        Utils::timestamp();
                        printf("Query is disabled (%s)\n", packet->systemAddress.ToString());
                        peer->CloseConnection(packet->systemAddress, true, 0, LOW_PRIORITY);
                        break;
                    }
                }
                case ID_GAME_INIT:
                {
                    BitStream query(packet->data, packet->length, false);
                    query.IgnoreBytes(sizeof(MessageID));

                    RakString rname, rpwd;
                    query.Read(rname);
                    query.Read(rpwd);
                    query.Reset();

                    char name[rname.GetLength()];
                    char pwd[rpwd.GetLength()];
                    strcpy(name, rname.C_String());
                    strcpy(pwd, rpwd.C_String());

                    Client* client = new Client(packet->guid, string(name), string(pwd));
                    self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), connections));

                    int ret = 1;

                    if (amx != NULL)
                    {
                        void* args[3];

                        int id = client->GetClientID();

                        args[0] = reinterpret_cast<void*>(pwd);
                        args[1] = reinterpret_cast<void*>(name);
                        args[2] = reinterpret_cast<void*>(&id);

                        ret = Script::Call(amx, (char*) "OnClientAuthenticate", (char*) "ssi", args);
                    }

                    if (ret)
                    {
                        query.Write((MessageID) ID_GAME_INIT);
                        peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, packet->systemAddress, false, 0);
                    }
                    else
                        peer->CloseConnection(packet->systemAddress, true, 0, HIGH_PRIORITY);
                    break;
                }
                case ID_GAME_RUN:
                {
                    BitStream query;

                    char savegame[128];
                    strcpy(savegame, "default");

                    int ret = 1;

                    if (amx != NULL)
                    {
                        void* args[3];

                        int id = Client::GetClientFromGUID(packet->guid)->GetClientID();
                        int len = sizeof(savegame);

                        args[0] = reinterpret_cast<void*>(&len);
                        args[1] = reinterpret_cast<void*>(savegame);
                        args[2] = reinterpret_cast<void*>(&id);

                        ret = Script::Call(amx, (char*) "OnClientRequestGame", (char*) "isi", args, len);
                    }

                    if (ret)
                    {
                        query.Write((MessageID) ID_GAME_RUN);

                        RakString save(savegame);
                        query.Write(save);

                        peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, packet->systemAddress, false, 0);
                    }
                    else
                        peer->CloseConnection(packet->systemAddress, true, 0, HIGH_PRIORITY);
                    break;
                }
                case ID_GAME_START:
                {
                    map<RakNetGUID, Player*> players = Player::GetPlayerList();
                    map<RakNetGUID, Player*>::iterator it;

                    Client* client = Client::GetClientFromGUID(packet->guid);
                    string name = client->GetAuthName();
                    RakString pname(name.c_str());

                    BitStream query(packet->data, packet->length, false);
                    query.IgnoreBytes(sizeof(MessageID));
                    query.Reset();

                    query.Write((MessageID) ID_NEW_PLAYER);
                    query.Write(packet->guid);
                    query.Write(pname);

                    for (it = players.begin(); it != players.end(); ++it)
                    {
                        RakNetGUID guid = it->first;
                        peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, guid, false, 0);
                    }

                    query.Reset();

                    for (it = players.begin(); it != players.end(); ++it)
                    {
                        RakNetGUID guid = it->first;
                        Player* player = Player::GetPlayerFromGUID(guid);
                        string name = player->GetActorName();
                        RakString pname(name.c_str());

                        query.Write((MessageID) ID_NEW_PLAYER);
                        query.Write(guid);
                        query.Write(pname);
                        peer->Send(&query, HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_SYSTEM, packet->guid, false, 0);
                        query.Reset();

                        if (!player->IsEmpty())
                        {
                            list<Item*> items = player->GetItemList();
                            list<Item*>::iterator it2;

                            for (it2 = items.begin(); it2 != items.end(); ++it2)
                            {
                                pActorItemUpdate item = player->GetActorItemUpdateStruct(*it2);
                                item.guid = guid;
                                peer->Send((char*) &item, sizeof(pActorItemUpdate), HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_PLAYER_ITEM_UPDATE, packet->guid, false, 0);
                            }
                        }
                    }

                    Player* player = new Player(packet->guid, "");
                    player->SetActorName(name);

                    int ret = 1;

                    if (amx != NULL)
                    {
                        void* args[1];

                        int id = Client::GetClientFromGUID(packet->guid)->GetClientID();

                        args[0] = reinterpret_cast<void*>(&id);

                        ret = Script::Call(amx, (char*) "OnPlayerJoin", (char*) "i", args);
                    }
                    break;
                }
                case ID_PLAYER_UPDATE:
                {
                    pActorUpdate* update = (pActorUpdate*) packet->data;

                    if (packet->length != sizeof(pActorUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(packet->guid);

                    player->SetActorPos(X_AXIS, update->X);
                    player->SetActorPos(Y_AXIS, update->Y);
                    player->SetActorPos(Z_AXIS, update->Z);
                    player->SetActorAngle(update->A);
                    player->SetActorAlerted(update->alerted);
                    player->SetActorMoving(update->moving);

                    player->UpdateActorUpdateStruct(update);

                    map<RakNetGUID, Player*> players = Player::GetPlayerList();
                    map<RakNetGUID, Player*>::iterator it;

                    for (it = players.begin(); it != players.end(); ++it)
                    {
                        RakNetGUID guid = it->first;
                        if (guid != packet->guid) peer->Send((char*) update, sizeof(pActorUpdate), MEDIUM_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_PLAYER_UPDATE, guid, false, 0);
                    }
                    break;
                }
                case ID_PLAYER_STATE_UPDATE:
                {
                    pActorStateUpdate* update = (pActorStateUpdate*) packet->data;

                    if (packet->length != sizeof(pActorStateUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(packet->guid);

                    if (update->dead == true && player->IsActorDead() != true)
                    {
                        int ret = 1;

                        if (amx != NULL)
                        {
                            void* args[1];

                            int id = Client::GetClientFromGUID(packet->guid)->GetClientID();

                            args[0] = reinterpret_cast<void*>(&id);

                            ret = Script::Call(amx, (char*) "OnPlayerDeath", (char*) "i", args);
                        }
                    }

                    player->SetActorHealth(update->health);
                    player->SetActorBaseHealth(update->baseHealth);
                    player->SetActorCondition(COND_PERCEPTION, update->conds[COND_PERCEPTION]);
                    player->SetActorCondition(COND_ENDURANCE, update->conds[COND_ENDURANCE]);
                    player->SetActorCondition(COND_LEFTATTACK, update->conds[COND_LEFTATTACK]);
                    player->SetActorCondition(COND_RIGHTATTACK, update->conds[COND_RIGHTATTACK]);
                    player->SetActorCondition(COND_LEFTMOBILITY, update->conds[COND_LEFTMOBILITY]);
                    player->SetActorCondition(COND_RIGHTMOBILITY, update->conds[COND_RIGHTMOBILITY]);
                    player->SetActorDead(update->dead);

                    player->UpdateActorStateUpdateStruct(update);

                    map<RakNetGUID, Player*> players = Player::GetPlayerList();
                    map<RakNetGUID, Player*>::iterator it;

                    for (it = players.begin(); it != players.end(); ++it)
                    {
                        RakNetGUID guid = it->first;
                        if (guid != packet->guid) peer->Send((char*) update, sizeof(pActorStateUpdate), HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_PLAYER_STATE_UPDATE, guid, false, 0);
                    }
                    break;
                }
                case ID_PLAYER_CELL_UPDATE:
                {
                    pActorCellUpdate* update = (pActorCellUpdate*) packet->data;

                    if (packet->length != sizeof(pActorCellUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(packet->guid);

                    int ret = 1;

                    if (amx != NULL)
                    {
                        void* args[1];

                        int id = Client::GetClientFromGUID(packet->guid)->GetClientID();
                        int cell = update->cell;

                        args[0] = reinterpret_cast<void*>(&cell);
                        args[1] = reinterpret_cast<void*>(&id);

                        ret = Script::Call(amx, (char*) "OnPlayerCellChange", (char*) "ii", args);
                    }

                    player->SetActorGameCell(update->cell);
                    player->SetActorNetworkCell(update->cell);

                    player->UpdateActorCellUpdateStruct(update);

                    map<RakNetGUID, Player*> players = Player::GetPlayerList();
                    map<RakNetGUID, Player*>::iterator it;

                    for (it = players.begin(); it != players.end(); ++it)
                    {
                        RakNetGUID guid = it->first;
                        if (guid != packet->guid) peer->Send((char*) update, sizeof(pActorCellUpdate), HIGH_PRIORITY, RELIABLE_SEQUENCED, CHANNEL_PLAYER_CELL_UPDATE, guid, false, 0);
                    }
                    break;
                }
                case ID_PLAYER_ITEM_UPDATE:
                {
                    pActorItemUpdate* update = (pActorItemUpdate*) packet->data;

                    if (packet->length != sizeof(pActorItemUpdate))
                        break;

                    Player* player = Player::GetPlayerFromGUID(packet->guid);

                    if (update->item.count == 0)
                    {
                        if (!player->UpdateItem(string(update->baseID), update->item.condition, update->item.worn))
                        {

                        }
                    }
                    else if (update->item.count > 0)
                    {
                        if (!player->AddItem(string(update->baseID), update->item.count, update->item.type, update->item.condition, update->item.worn))
                        {

                        }
                    }
                    else if (update->item.count < 0)
                    {
                        if (!player->RemoveItem(string(update->baseID), abs(update->item.count)))
                        {

                        }
                    }

                    map<RakNetGUID, Player*> players = Player::GetPlayerList();
                    map<RakNetGUID, Player*>::iterator it;

                    for (it = players.begin(); it != players.end(); ++it)
                    {
                        RakNetGUID guid = it->first;
                        if (guid != packet->guid) peer->Send((char*) update, sizeof(pActorItemUpdate), HIGH_PRIORITY, RELIABLE_ORDERED, CHANNEL_PLAYER_ITEM_UPDATE, guid, false, 0);
                    }
                    break;
                }*/
                }
            }

            RakSleep(2);

            if (announce)
            {
                if ((GetTimeMS() - announcetime) > RAKNET_MASTER_RATE)
                    Announce(true);
            }
        }
    }
    catch (...)
    {

    }

    peer->Shutdown(300);
    RakPeerInterface::DestroyInstance(peer);

    Player::DestroyInstances();
    Actor::DestroyInstances();

#ifdef VAULTMP_DEBUG
    debug->Print("Network thread is going to terminate", true);
    delete debug;
#endif

#ifdef __WIN32__
    return ((DWORD) data);
#else
    return data;
#endif
}

#ifdef __WIN32__
HANDLE Dedicated::InitializeServer(int port, int connections, AMX* amx, char* announce, bool query)
#else
pthread_t Dedicated::InitializeServer(int port, int connections, AMX* amx, char* announce, bool query)
#endif
{
#ifdef __WIN32__
    HANDLE hDedicatedThread;
#else
    pthread_t hDedicatedThread;
#endif

    thread = true;
    Dedicated::port = port ? port : RAKNET_STANDARD_PORT;
    Dedicated::connections = connections ? connections : RAKNET_STANDARD_CONNECTIONS;
    Dedicated::amx = amx;
    Dedicated::announce = announce;
    Dedicated::query = query;

    Dedicated::self->SetServerPlayers(pair<int, int>(0, Dedicated::connections));

#ifdef __WIN32__
    hDedicatedThread = CreateThread(NULL, 0, DedicatedThread, (LPVOID) 0, 0, NULL);
#else
    pthread_create(&hDedicatedThread, NULL, DedicatedThread, NULL);
#endif

    return hDedicatedThread;
}

void Dedicated::SetServerEntry(ServerEntry* self)
{
    Dedicated::self = self;
}

/* void Dedicated::SetServerConnections(int connections)
{

} */
