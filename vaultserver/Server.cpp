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

    Client* client = new Client(guid, name, pwd);
    Dedicated::self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), Dedicated::connections));
    int result = Script::Authenticate(client->GetClientID(), name, pwd);

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

NetworkResponse Server::Disconnect(RakNetGUID guid, unsigned char reason)
{
    NetworkResponse response;

    Client* client = Client::GetClientFromGUID(guid);
    Player* player = Player::GetPlayerFromGUID(guid);

    if (player != NULL && client != NULL)
        Script::Disconnect(client->GetClientID(), reason);
    else if (client != NULL)
        Script::Disconnect(client->GetClientID(), ID_REASON_DENIED);

    if (player != NULL)
    {
        delete player;

        pDefault* packet = PacketFactory::CreatePacket(ID_PLAYER_LEFT, guid);
        response = Network::CompleteResponse(Network::CreateResponse(packet,
                                             (unsigned char) HIGH_PRIORITY,
                                             (unsigned char) RELIABLE_ORDERED,
                                             CHANNEL_GAME,
                                             Player::GetPlayerNetworkList()));
    }

    if (client != NULL)
    {
        delete client;
        Dedicated::self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), Dedicated::connections));
    }

    return response;
}
