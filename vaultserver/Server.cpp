#include "Server.h"

#ifdef VAULTMP_DEBUG
Debug* Server::debug = NULL;

#endif

#ifdef VAULTMP_DEBUG
void Server::SetDebugHandler(Debug* debug)
{
    Server::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Server class", true);
}
#endif

NetworkResponse Server::Authenticate(RakNetGUID guid, string name, string pwd)
{
    NetworkResponse response;
    bool result = Script::Authenticate(name, pwd);

    if (result)
    {
        for (ModList::iterator it = Dedicated::modfiles.begin(); it != Dedicated::modfiles.end(); ++it)
        {
            pDefault* packet = PacketFactory::CreatePacket(ID_GAME_MOD, it->first.c_str(), it->second);
            response.push_back(Network::CreateResponse(packet,
                               (unsigned char) HIGH_PRIORITY,
                               (unsigned char) RELIABLE_ORDERED,
                               CHANNEL_GAME,
                               guid));
        }

        pDefault* packet = PacketFactory::CreatePacket(ID_GAME_START, Dedicated::savegame.first.c_str(), Dedicated::savegame.second);
        response.push_back(Network::CreateResponse(packet,
                           (unsigned char) HIGH_PRIORITY,
                           (unsigned char) RELIABLE_ORDERED,
                           CHANNEL_GAME,
                           guid));
    }
    else
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_GAME_END, ID_REASON_DENIED);
        response = Network::CompleteResponse(Network::CreateResponse(packet,
                                             (unsigned char) HIGH_PRIORITY,
                                             (unsigned char) RELIABLE_ORDERED,
                                             CHANNEL_GAME,
                                             guid));
    }

    return response;
}

NetworkResponse Server::NewPlayer(RakNetGUID guid, NetworkID id, string name)
{
    NetworkResponse response;
    Player* player = (Player*) GameFactory::CreateInstance(ID_PLAYER, 0x00);
    player->SetNetworkID(id);
    player->SetName(name);

    vector<RakNetGUID> network = Client::GetNetworkList();
    Client* client = new Client(guid, player);
    Dedicated::self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), Dedicated::connections));

    unsigned int result = Script::RequestGame(client->GetID());
    player->SetBase(result);

    pDefault* packet = PacketFactory::CreatePacket(ID_PLAYER_NEW, id, name.c_str(), result);
    response = Network::CompleteResponse(Network::CreateResponse(packet,
                                         (unsigned char) HIGH_PRIORITY,
                                         (unsigned char) RELIABLE_ORDERED,
                                         CHANNEL_GAME,
                                         network));

    return response;
}

NetworkResponse Server::Disconnect(RakNetGUID guid, unsigned char reason)
{
    NetworkResponse response;
    Client* client = Client::GetClientFromGUID(guid);

    if (client != NULL)
    {
        Script::Disconnect(client->GetID(), reason);
        Player* player = client->GetPlayer();
        delete client;

        NetworkID id = GameFactory::DestroyInstance(player);

        pDefault* packet = PacketFactory::CreatePacket(ID_PLAYER_LEFT, guid);
        response = Network::CompleteResponse(Network::CreateResponse(packet,
                                             (unsigned char) HIGH_PRIORITY,
                                             (unsigned char) RELIABLE_ORDERED,
                                             CHANNEL_GAME,
                                             Client::GetNetworkList()));

        Dedicated::self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), Dedicated::connections));
    }

    return response;
}
