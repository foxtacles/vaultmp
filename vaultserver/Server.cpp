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
    Player* player = (Player*) GameFactory::CreateInstance(ID_PLAYER, 0x00000000);
    player->SetNetworkID(id);
    player->SetName(name);

    Client* client = new Client(guid, player->GetNetworkID());
    Dedicated::self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), Dedicated::connections));

    unsigned int result = Script::RequestGame(client->GetID());
    player->SetBase(result);

    pDefault* packet = PacketFactory::CreatePacket(ID_PLAYER_NEW, id, 0x00000000, player->GetBase(), name.c_str());
    response.push_back(Network::CreateResponse(packet,
                                            (unsigned char) HIGH_PRIORITY,
                                            (unsigned char) RELIABLE_ORDERED,
                                            CHANNEL_GAME,
                                            Client::GetNetworkList(client)));

    list<Reference*> players = GameFactory::GetObjectTypes(ID_PLAYER);
    list<Reference*>::iterator it;

    for (it = players.begin(); it != players.end() && *it != player; ++it)
    {
        Player* _player = (Player*) *it;

        packet = PacketFactory::CreatePacket(ID_PLAYER_NEW, _player->GetNetworkID(), _player->GetReference(), _player->GetBase(), _player->GetName().c_str());
        response.push_back(Network::CreateResponse(packet, (unsigned char) HIGH_PRIORITY, (unsigned char) RELIABLE_ORDERED, CHANNEL_GAME, guid));

        vector<unsigned char> data = API::RetrieveAllAxis();
        vector<unsigned char>::iterator it2;

        for (it2 = data.begin(); it2 != data.end(); ++it2)
        {
            packet = PacketFactory::CreatePacket(ID_UPDATE_POS, _player->GetNetworkID(), *it2, _player->GetPos(*it2));
            response.push_back(Network::CreateResponse(packet, (unsigned char) HIGH_PRIORITY, (unsigned char) RELIABLE_ORDERED, CHANNEL_DATA, guid));
        }

        data = API::RetrieveAllValues();

        for (it2 = data.begin(); it2 != data.end(); ++it2)
        {
            packet = PacketFactory::CreatePacket(ID_UPDATE_VALUE, _player->GetNetworkID(), true, *it2, _player->GetActorBaseValue(*it2));
            response.push_back(Network::CreateResponse(packet, (unsigned char) HIGH_PRIORITY, (unsigned char) RELIABLE_ORDERED, CHANNEL_DATA, guid));
            packet = PacketFactory::CreatePacket(ID_UPDATE_VALUE, _player->GetNetworkID(), false, *it2, _player->GetActorValue(*it2));
            response.push_back(Network::CreateResponse(packet, (unsigned char) HIGH_PRIORITY, (unsigned char) RELIABLE_ORDERED, CHANNEL_DATA, guid));
        }

        packet = PacketFactory::CreatePacket(ID_UPDATE_CELL, _player->GetNetworkID(), _player->GetGameCell());
        response.push_back(Network::CreateResponse(packet, (unsigned char) HIGH_PRIORITY, (unsigned char) RELIABLE_ORDERED, CHANNEL_DATA, guid));

        GameFactory::LeaveReference(_player);
    }

    GameFactory::LeaveReference(player); // two times because GetObjectTypes also returned player (and locked it)
    GameFactory::LeaveReference(player);

    return response;
}

NetworkResponse Server::Disconnect(RakNetGUID guid, unsigned char reason)
{
    NetworkResponse response;
    Client* client = Client::GetClientFromGUID(guid);

    if (client != NULL)
    {
        Script::Disconnect(client->GetID(), reason);
        Player* player = (Player*) GameFactory::GetObject(client->GetPlayer());
        delete client;

        NetworkID id = GameFactory::DestroyInstance(player);

        pDefault* packet = PacketFactory::CreatePacket(ID_PLAYER_LEFT, id);
        response = Network::CompleteResponse(Network::CreateResponse(packet,
                                             (unsigned char) HIGH_PRIORITY,
                                             (unsigned char) RELIABLE_ORDERED,
                                             CHANNEL_GAME,
                                             Client::GetNetworkList(NULL)));

        Dedicated::self->SetServerPlayers(pair<int, int>(Client::GetClientCount(), Dedicated::connections));
    }

    return response;
}

NetworkResponse Server::GetPos(RakNetGUID guid, NetworkID id, unsigned char axis, double value)
{
    NetworkResponse response;
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    bool result = (bool) object->SetPos(axis, value);

    if (result)
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_POS, object->GetNetworkID(), axis, value);
        response = Network::CompleteResponse(Network::CreateResponse(packet,
                                                             (unsigned char) HIGH_PRIORITY,
                                                             (unsigned char) RELIABLE_SEQUENCED,
                                                             CHANNEL_DATA,
                                                             Client::GetNetworkList(guid)));
    }

    GameFactory::LeaveReference(object);

    return response;
}

NetworkResponse Server::GetAngle(RakNetGUID guid, NetworkID id, unsigned char axis, double value)
{
    NetworkResponse response;
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    bool result = (bool) object->SetAngle(axis, value);

    if (result)
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_ANGLE, object->GetNetworkID(), axis, value);
        response = Network::CompleteResponse(Network::CreateResponse(packet,
                                                             (unsigned char) HIGH_PRIORITY,
                                                             (unsigned char) RELIABLE_SEQUENCED,
                                                             CHANNEL_DATA,
                                                             Client::GetNetworkList(guid)));
    }

    GameFactory::LeaveReference(object);

    return response;
}

NetworkResponse Server::GetGameCell(RakNetGUID guid, NetworkID id, unsigned int cell)
{
    NetworkResponse response;
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    bool result = (bool) object->SetGameCell(cell);

    if (result)
    {
        Client* client = Client::GetClientFromGUID(guid);

        if (client != NULL)
            Script::CellChange(client->GetID(), cell);

        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_CELL, object->GetNetworkID(), cell);
        response = Network::CompleteResponse(Network::CreateResponse(packet,
                                                             (unsigned char) HIGH_PRIORITY,
                                                             (unsigned char) RELIABLE_SEQUENCED,
                                                             CHANNEL_DATA,
                                                             Client::GetNetworkList(guid)));
    }

    GameFactory::LeaveReference(object);

    return response;
}

NetworkResponse Server::GetActorValue(RakNetGUID guid, NetworkID id, bool base, unsigned char index, double value)
{
    NetworkResponse response;
    Object* object = (Object*) GameFactory::GetObject(id);

    if (object == NULL)
        throw VaultException("Unknown object with NetworkID %lld", id);

    Actor* actor = dynamic_cast<Actor*>(object);

    if (actor == NULL)
    {
        GameFactory::LeaveReference(object);
        throw VaultException("Object %lld is not an Actor", id);
    }

    bool result;

    if (base)
        result = (bool) actor->SetActorBaseValue(index, value);
    else
        result = (bool) actor->SetActorValue(index, value);

    if (result)
    {
        Client* client = Client::GetClientFromGUID(guid);

        if (client != NULL)
            Script::ValueChange(client->GetID(), base, index, value);

        pDefault* packet = PacketFactory::CreatePacket(ID_UPDATE_VALUE, object->GetNetworkID(), base, index, value);
        response = Network::CompleteResponse(Network::CreateResponse(packet,
                                                             (unsigned char) HIGH_PRIORITY,
                                                             (unsigned char) RELIABLE_ORDERED,
                                                             CHANNEL_DATA,
                                                             Client::GetNetworkList(guid)));
    }

    GameFactory::LeaveReference(actor);

    return response;
}
