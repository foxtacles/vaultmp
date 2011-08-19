#include "Network.h"
#include "Bethesda.h"

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
    case ID_EVENT_CLIENT_ERROR:
    case ID_EVENT_INTERFACE_LOST:
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_GAME_END, ID_REASON_ERROR);
        response.push_back(CreateResponse(packet, (unsigned char) HIGH_PRIORITY, (unsigned char) RELIABLE_ORDERED, CHANNEL_SYSTEM));
        break;
    }
    case ID_EVENT_GAME_STARTED:
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_GAME_CONFIRM);
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
#ifdef VAULTMP_DEBUG
        debug->PrintFormat("Connection request accepted (%s)", true, data->systemAddress.ToString());
#endif

        //response = Game::ProcessEvent();
        break;
    }
    case ID_DISCONNECTION_NOTIFICATION:
    {
#ifdef VAULTMP_DEBUG
        debug->PrintFormat("Connection closed (%s)", true, data->systemAddress.ToString());
#endif
        break;
    }
    case ID_INVALID_PASSWORD:
        throw VaultException("Dedicated server version mismatch.\nPlease download the most recent binaries from www.vaultmp.com");
    case ID_NO_FREE_INCOMING_CONNECTIONS:
        throw VaultException("The server is full");
    case ID_CONNECTION_ATTEMPT_FAILED:
        throw VaultException("Failed to connect to the server");
    case ID_CONNECTION_BANNED:
        throw VaultException("You are banned from the server");
    case ID_CONNECTION_LOST:
        throw VaultException("Lost connection to the server");

    default:
    {
        packet = PacketFactory::CreatePacket(data->data, data->length);

        switch (data->data[0])
        {
        case ID_GAME_START:
        {
#ifdef VAULTMP_DEBUG
            debug->PrintFormat("We were successfully authenticated (%s)", true, data->systemAddress.ToString());
            debug->Print("Initiating vaultmp game thread...", true);
#endif

            Bethesda::InitializeGame();

            response = Network::ProcessEvent(ID_EVENT_GAME_STARTED, guid);
            break;
        }

        case ID_GAME_END:
        {
            unsigned char reason;
            PacketFactory::Access(packet, &reason);

            break;
        }

        case ID_PLAYER_NEW:
        {
            RakNetGUID _guid;
            char name[MAX_PLAYER_NAME + 1]; ZeroMemory(name, sizeof(name));
            unsigned int baseID;
            PacketFactory::Access(packet, &_guid, name, &baseID);

            //response = Game::ProcessEvent();
            break;
        }

        case ID_PLAYER_LEFT:
        {
            RakNetGUID _guid;
            PacketFactory::Access(packet, &_guid);

            //response = Game::ProcessEvent();
            break;
        }

        case ID_PLAYER_UPDATE:
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
