#include "NetworkClient.h"
#include "Bethesda.h"
#include "Game.h"

#ifdef VAULTMP_DEBUG
Debug* NetworkClient::debug = NULL;
#endif

#ifdef VAULTMP_DEBUG
void NetworkClient::SetDebugHandler(Debug* debug)
{
    NetworkClient::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to NetworkClient class", true);
}
#endif

NetworkResponse NetworkClient::ProcessEvent(unsigned char id)
{
    NetworkResponse response;

    switch (id)
    {
    case ID_EVENT_CLIENT_ERROR:
    case ID_EVENT_INTERFACE_LOST:
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_GAME_END, ID_REASON_ERROR);
        response = Network::CompleteResponse(Network::CreateResponse(packet, (unsigned char) HIGH_PRIORITY, (unsigned char) RELIABLE_ORDERED, CHANNEL_GAME, Game::server));
        break;
    }
    case ID_EVENT_GAME_STARTED:
    {
        pDefault* packet = PacketFactory::CreatePacket(ID_GAME_CONFIRM);
        response = Network::CompleteResponse(Network::CreateResponse(packet, (unsigned char) HIGH_PRIORITY, (unsigned char) RELIABLE_ORDERED, CHANNEL_GAME, Game::server));
        break;
    }

    default:
        throw VaultException("Unhandled event type %d", id);
    }

    return response;
}

NetworkResponse NetworkClient::ProcessPacket(Packet* data)
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
        response = Game::Authenticate();
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
        case ID_GAME_MOD:
        {
            char modfile[MAX_MOD_FILE + 1]; ZeroMemory(modfile, sizeof(modfile));
            unsigned int crc;
            PacketFactory::Access(packet, modfile, &crc);
            Bethesda::modfiles.push_back(pair<string, unsigned int>(string(modfile), crc));
            break;
        }

        case ID_GAME_START:
        {
#ifdef VAULTMP_DEBUG
            debug->PrintFormat("We were successfully authenticated (%s)", true, data->systemAddress.ToString());
            debug->Print("Initiating vaultmp game thread...", true);
#endif
            char savegame[MAX_SAVEGAME_FILE + 1]; ZeroMemory(savegame, sizeof(savegame));
            unsigned int crc;
            PacketFactory::Access(packet, savegame, &crc);
            Bethesda::savegame = Savegame(string(savegame), crc);

            Bethesda::InitializeGame();

            response = NetworkClient::ProcessEvent(ID_EVENT_GAME_STARTED);
            break;
        }

        case ID_GAME_END:
        {
            unsigned char reason;
            PacketFactory::Access(packet, &reason);

            switch (reason)
            {
            case ID_REASON_KICK:
                throw VaultException("You have been kicked from the server");
            case ID_REASON_BAN:
                throw VaultException("You have been banned from the server");
            case ID_REASON_ERROR:
                throw VaultException("The server encountered an internal error");
            case ID_REASON_DENIED:
                throw VaultException("Your authentication has been denied");
            case ID_REASON_NONE:
                break;
            }

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
