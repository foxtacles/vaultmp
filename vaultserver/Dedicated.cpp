#include "Dedicated.h"

using namespace RakNet;
using namespace Data;
using namespace std;

RakPeerInterface* Dedicated::peer;
SocketDescriptor* Dedicated::sockdescr;
int Dedicated::port;
int Dedicated::fileslots;
int Dedicated::connections;
char* Dedicated::announce;
bool Dedicated::query;
bool Dedicated::fileserve;
SystemAddress Dedicated::master;
TimeMS Dedicated::announcetime;
ServerEntry* Dedicated::self = NULL;
Savegame Dedicated::savegame;
ModList Dedicated::modfiles;
#ifdef VAULTMP_DEBUG
Debug* Dedicated::debug;
#endif

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

int Dedicated::GetGameCode()
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

class FileProgress : public FileListProgress
{
    virtual void OnFilePush(const char* fileName, unsigned int fileLengthBytes, unsigned int offset, unsigned int bytesBeingSent, bool done, SystemAddress targetSystem)
    {
        Utils::timestamp();
        printf("Sending %s (%d bytes) to %s\n", fileName, bytesBeingSent, targetSystem.ToString(false));
#ifdef VAULTMP_DEBUG
        Dedicated::debug->PrintFormat("Sending %s (%d bytes) to %s", true, fileName, bytesBeingSent, targetSystem.ToString(false));
#endif
    }

    virtual void OnFilePushesComplete(SystemAddress systemAddress)
    {
        Utils::timestamp();
        printf("Transfer complete (%s)\n", systemAddress.ToString(false));
#ifdef VAULTMP_DEBUG
        Dedicated::debug->PrintFormat("Transfer complete (%s)", true, systemAddress.ToString(false));
#endif
    }

    virtual void OnSendAborted(SystemAddress systemAddress)
    {
        Utils::timestamp();
        printf("Transfer aborted (%s)\n", systemAddress.ToString(false));
#ifdef VAULTMP_DEBUG
        Dedicated::debug->PrintFormat("Transfer aborted (%s)", true, systemAddress.ToString(false));
#endif
    }

} fileProgress;

#ifdef __WIN32__
DWORD WINAPI Dedicated::FileThread(LPVOID data)
#else
void* Dedicated::FileThread(void* data)
#define THREAD_PRIORITY_NORMAL 1000
#endif
{
    PacketizedTCP tcp;
    FileList files;
    FileListTransfer flt;
    IncrementalReadInterface incInterface;
    tcp.Start(Dedicated::port, Dedicated::fileslots);
    tcp.AttachPlugin(&flt);
    flt.AddCallback(&fileProgress);
    flt.StartIncrementalReadThreads(1);

    char dir[MAX_PATH];
    char file[MAX_PATH];
    getcwd(dir, sizeof(dir));

    snprintf(file, sizeof(file), "%s\\%s\\%s", dir, SAVEGAME_PATH, savegame.first.c_str());
    unsigned int len = Utils::FileLength(file);
    files.AddFile(savegame.first.c_str(), file, 0, len, len, FileListNodeContext(FILE_SAVEGAME, 0), true);

    ModList::iterator it;
    int i = 1;
    for (it = modfiles.begin(), i; it != modfiles.end(); ++it, i++)
    {
        snprintf(file, sizeof(file), "%s\\%s\\%s", dir, MODFILES_PATH, it->first.c_str());
        len = Utils::FileLength(file);
        files.AddFile(it->first.c_str(), file, 0, len, len, FileListNodeContext(FILE_MODFILE, i), true);
    }

    Packet* packet;
    char rdy = RAKNET_FILE_RDY;

    while (thread)
    {
        packet = tcp.Receive();
        SystemAddress addr = tcp.HasNewIncomingConnection();

        if (addr != UNASSIGNED_SYSTEM_ADDRESS)
            tcp.Send(&rdy, sizeof(char), addr, false);

        if (packet && packet->data[0] == RAKNET_FILE_RDY && packet->length == 3)
            flt.Send(&files, 0, packet->systemAddress, *((unsigned short*) (packet->data + 1)), MEDIUM_PRIORITY, CHANNEL_SYSTEM, &incInterface, 2000000);

        tcp.DeallocatePacket(packet);
        RakSleep(5);
    }

#ifdef __WIN32__
    return ((DWORD) data);
#else
    return data;
#endif
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
    Dedicated::debug = debug;
    debug->PrintFormat("Vault-Tec Multiplayer Mod Dedicated server debug log (%s)", false, DEDICATED_VERSION);
    debug->PrintFormat("Local host: %s (game: %s)", false, peer->GetMyBoundAddress().ToString(), self->GetGame() == FALLOUT3 ? (char*) "Fallout 3" : self->GetGame() == NEWVEGAS ? (char*) "Fallout New Vegas" : (char*) "TES Oblivion");
    debug->Print("Visit www.vaultmp.com for help and upload this log if you experience problems with the mod.", false);
    debug->Print("-----------------------------------------------------------------------------------------------------", false);
    //debug->PrintSystem();
    //Player::SetDebugHandler(debug);
    VaultException::SetDebugHandler(debug);
    NetworkServer::SetDebugHandler(debug);
    Container::SetDebugHandler(debug);
    Object::SetDebugHandler(debug);
    GameFactory::SetDebugHandler(debug);
#endif

    Packet* packet;

    API::Initialize(self->GetGame());
    Container::Initialize(self->GetGame());
    Client::SetMaximumClients(connections);
    Network::Flush();

    try
    {
        while (thread)
        {
            NetworkResponse response;

            while ((response = Network::Next()).size())
                Network::Dispatch(peer, response);

            for (packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
            {
                if (packet->data[0] == ID_MASTER_UPDATE)
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
                        int game = self->GetGame();
                        std::map<string, string> rules = self->GetServerRules();

                        query.Write(name);
                        query.Write(map);
                        query.Write(players);
                        query.Write(playersMax);
                        query.Write(game);
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
                else
                {
                    try
                    {
                        response = NetworkServer::ProcessPacket(packet);

                        vector<RakNetGUID> closures;
                        for (NetworkResponse::iterator it = response.begin(); it != response.end(); ++it)
                            if (*it->first.first->get() == ID_GAME_END)
                                closures.insert(closures.end(), it->second.begin(), it->second.end());

                        Network::Dispatch(peer, response);

                        for (vector<RakNetGUID>::iterator it = closures.begin(); it != closures.end(); ++it)
                            peer->CloseConnection(*it, true, CHANNEL_SYSTEM, HIGH_PRIORITY);
                    }
                    catch (...)
                    {
                        peer->DeallocatePacket(packet);
                        response = NetworkServer::ProcessEvent(ID_EVENT_SERVER_ERROR);
                        Network::Dispatch(peer, response);
                        throw;
                    }
                }
                switch (packet->data[0])
                {
                    /*
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
    catch (std::exception& e)
    {
        try
        {
            VaultException& vaulterror = dynamic_cast<VaultException&>(e);
            vaulterror.Console();
        }
        catch (std::bad_cast& no_vaulterror)
        {
            VaultException vaulterror(e.what());
            vaulterror.Console();
        }
    }

    peer->Shutdown(300);
    RakPeerInterface::DestroyInstance(peer);

    GameFactory::DestroyAllInstances();
    Container::Cleanup();
    API::Terminate();

#ifdef VAULTMP_DEBUG
    debug->Print("Network thread is going to terminate", true);
    VaultException::FinalizeDebug();
#endif

#ifdef __WIN32__
    return ((DWORD) data);
#else
    return data;
#endif
}

#ifdef __WIN32__
HANDLE Dedicated::InitializeServer(int port, int connections, char* announce, bool query, bool fileserve, int fileslots)
#else
pthread_t Dedicated::InitializeServer(int port, int connections, char* announce, bool query, bool fileserve, int fileslots)
#endif
{
#ifdef __WIN32__
    HANDLE hDedicatedThread;
    HANDLE hFileThread;
#else
    pthread_t hDedicatedThread;
    pthread_t hFileThread;
#endif

    thread = true;
    Dedicated::port = port ? port : RAKNET_STANDARD_PORT;
    Dedicated::connections = connections ? connections : RAKNET_STANDARD_CONNECTIONS;
    Dedicated::announce = strlen(announce) ? announce : NULL;
    Dedicated::query = query;
    Dedicated::fileserve = fileserve;
    Dedicated::fileslots = fileslots;
    Dedicated::self->SetServerPlayers(pair<int, int>(0, Dedicated::connections));

#ifdef __WIN32__
    hDedicatedThread = CreateThread(NULL, 0, DedicatedThread, (LPVOID) 0, 0, NULL);
    if (fileserve)
        hFileThread = CreateThread(NULL, 0, FileThread, (LPVOID) 0, 0, NULL);
#else
    pthread_create(&hDedicatedThread, NULL, DedicatedThread, NULL);
    if (fileserve)
        pthread_create(&hFileThread, NULL, FileThread, NULL);
#endif

    return hDedicatedThread;
}

void Dedicated::SetServerEntry(ServerEntry* self)
{
    Dedicated::self = self;
}

void Dedicated::SetSavegame(Savegame savegame)
{
    Dedicated::savegame = savegame;
}

void Dedicated::SetModfiles(ModList modfiles)
{
    Dedicated::modfiles = modfiles;
}

/* void Dedicated::SetServerConnections(int connections)
{

} */
