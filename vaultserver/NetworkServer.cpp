#include "../Network.h"
#include "Dedicated.h"

#ifdef VAULTMP_DEBUG
Debug* Network::debug = NULL;
#endif

#ifdef VAULTMP_DEBUG
void Network::SetDebugHandler(Debug* debug)
{
    Network::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to Network class", true);
}
#endif

pair<pDefault*, vector<unsigned char> > Network::CreateResponse(pDefault* packet, unsigned char priority, unsigned char reliability, unsigned char channel)
{
    vector<unsigned char> data = vector<unsigned char>();
    data.push_back(priority);
    data.push_back(reliability);
    data.push_back(channel);
    pair<pDefault*, vector<unsigned char> > response = pair<pDefault*, vector<unsigned char> >(packet, data);
    return response;
}

NetworkResponse Network::ProcessEvent(unsigned char id, RakNetGUID guid)
{
    NetworkResponse response;

    switch (id)
    {
    case ID_EVENT_SERVER_ERROR:
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_GAME_END, ID_REASON_ERROR);
        response.push_back(CreateResponse(packet, (unsigned char) HIGH_PRIORITY, (unsigned char) RELIABLE_ORDERED, CHANNEL_SYSTEM));
        break;
    }

    default:
        throw VaultException("Unhandled event type %d", id);
    }

    return response;
}

NetworkResponse Network::ProcessPacket(Packet* data, RakNetGUID guid)
{
    NetworkResponse response;
    pDefault* packet;

    switch (data->data[0])
    {
    case ID_CONNECTION_REQUEST_ACCEPTED:
    {
        Utils::timestamp();
        printf("Connected to MasterServer (%s)\n", data->systemAddress.ToString());
#ifdef VAULTMP_DEBUG
        debug->PrintFormat("Connected to MasterServer (%s)", true, data->systemAddress.ToString());
#endif
        Dedicated::master = data->systemAddress;
        Dedicated::Announce(true);
        break;
    }
    case ID_DISCONNECTION_NOTIFICATION:
    case ID_CONNECTION_LOST:
    {
        Utils::timestamp();
        switch (data->data[0])
        {
            case ID_DISCONNECTION_NOTIFICATION:
                printf("Client disconnected (%s)\n", data->systemAddress.ToString());
#ifdef VAULTMP_DEBUG
                debug->PrintFormat("Client disconnected (%s)", true, data->systemAddress.ToString());
#endif
                break;
            case ID_CONNECTION_LOST:
                printf("Lost connection (%s)\n", data->systemAddress.ToString());
#ifdef VAULTMP_DEBUG
                debug->PrintFormat("Lost connection (%s)", true, data->systemAddress.ToString());
#endif
                break;
        }


        break;
    }
    case ID_NEW_INCOMING_CONNECTION:
    {
        Utils::timestamp();
        printf("New incoming connection from %s\n", data->systemAddress.ToString());
#ifdef VAULTMP_DEBUG
        debug->PrintFormat("New incoming connection from %s", true, data->systemAddress.ToString());
#endif
        break;
    }
    case ID_INVALID_PASSWORD:
    {
        Utils::timestamp();
        printf("MasterServer version mismatch (%s)\n", data->systemAddress.ToString());
#ifdef VAULTMP_DEBUG
        debug->PrintFormat("MasterServer version mismatch (%s)", true, data->systemAddress.ToString());
#endif
        break;
    }
    case ID_CONNECTED_PING:
    case ID_UNCONNECTED_PING:
    case ID_CONNECTION_ATTEMPT_FAILED:
        break;

    default:
    {
        packet = PacketFactory::CreatePacket(data->data, data->length);

        switch (data->data[0])
        {
        case ID_GAME_AUTH:
        {
            char name[MAX_PLAYER_NAME + 1]; ZeroMemory(name, sizeof(name));
            char pwd[MAX_PASSWORD_SIZE + 1]; ZeroMemory(pwd, sizeof(pwd));
            PacketFactory::Access(packet, name, pwd);

            //response = Server::ProcessEvent(ID_EVENT_AUTH_RECEIVED);
            break;
        }

        case ID_GAME_CONFIRM:
        {
            //response = Server::ProcessEvent(ID_EVENT_CONFIRM_RECEIVED);
            break;
        }

        case ID_GAME_END:
        {
            unsigned char reason;
            PacketFactory::Access(packet, &reason);

            //response = Server::ProcessEvent(ID_EVENT_CLOSE_RECEIVED);
            break;
        }

        case ID_PLAYER_UPDATE: // server -> client, client -> server
        {

            break;
        }
        default:
            throw VaultException("Unhandled packet type %d", (int) data->data[0]);
        }
    }
    }

    return response;
}

void Network::Dispatch(RakPeerInterface* peer, NetworkResponse& response, const SystemAddress& target, bool broadcast)
{
    if (peer == NULL)
        throw VaultException("RakPeerInterface is NULL");

    NetworkResponse::iterator it;

    for (it = response.begin(); it != response.end(); ++it)
    {
        peer->Send((char*) it->first->get(), it->first->length(), (PacketPriority) it->second.at(0), (PacketReliability) it->second.at(1), it->second.at(2), target, broadcast);
        delete it->first;
    }
}
