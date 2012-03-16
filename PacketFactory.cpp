#include "PacketFactory.h"

pDefault* PacketFactory::CreatePacket( unsigned char type, ... )
{
    va_list args;
    va_start( args, type );
    pDefault* packet;

    switch ( type )
    {
        case ID_GAME_CONFIRM:
        {
            NetworkID id = va_arg( args, NetworkID );
            char* name = va_arg( args, char* );
            packet = new pGameConfirm( id, name );
            break;
        }

        case ID_GAME_AUTH:
        {
            char* name = va_arg( args, char* );
            char* pwd = va_arg( args, char* );
            packet = new pGameAuth( name, pwd );
            break;
        }

        case ID_GAME_LOAD:
        {
            packet = new pGameLoad();
            break;
        }

        case ID_GAME_MOD:
        {
            char* modfile = va_arg( args, char* );
            unsigned int crc = va_arg( args, unsigned int );
            packet = new pGameMod( modfile, crc );
            break;
        }

        case ID_GAME_START:
        {
            char* savegame = va_arg( args, char* );
            unsigned int crc = va_arg( args, unsigned int );
            packet = new pGameStart( savegame, crc );
            break;
        }

        case ID_GAME_END:
        {
            unsigned char reason = ( unsigned char ) va_arg( args, unsigned int );
            packet = new pGameEnd( reason );
            break;
        }

        case ID_OBJECT_NEW:
        {
            NetworkID id = va_arg( args, NetworkID );
            unsigned int refID = va_arg( args, unsigned int );
            unsigned int baseID = va_arg( args, unsigned int );
            char* name = va_arg( args, char* );
            double X = va_arg(args, double);
            double Y = va_arg(args, double);
            double Z = va_arg(args, double);
            double aX = va_arg(args, double);
            double aY = va_arg(args, double);
            double aZ = va_arg(args, double);
            unsigned int cell = va_arg(args, unsigned int);
            bool enabled = (bool) va_arg(args, unsigned int);
            packet = new pObjectNew( id, refID, baseID, name, X, Y, Z, aX, aY, aZ, cell, enabled);
            break;
        }

        case ID_ITEM_NEW:
        {
            pDefault* pObjectNew = va_arg(args, pDefault*);
            unsigned int count = va_arg(args, unsigned int);
            double condition = va_arg(args, double);
            bool equipped = (bool) va_arg(args, unsigned int);
            packet = new pItemNew(pObjectNew, count, condition, equipped);
            break;
        }

        case ID_PLAYER_NEW:
        {
            NetworkID id = va_arg( args, NetworkID );
            unsigned int refID = va_arg( args, unsigned int );
            unsigned int baseID = va_arg( args, unsigned int );
            char* name = va_arg( args, char* );
            packet = new pPlayerNew( id, refID, baseID, name );
            break;
        }

        case ID_PLAYER_LEFT:
        {
            NetworkID id = va_arg( args, NetworkID );
            packet = new pPlayerLeft( id );
            break;
        }

        case ID_UPDATE_POS:
        {
            NetworkID id = va_arg( args, NetworkID );
            double X = va_arg( args, double );
            double Y = va_arg( args, double );
            double Z = va_arg( args, double );
            packet = new pObjectPos( id, X, Y, Z );
            break;
        }

        case ID_UPDATE_ANGLE:
        {
            NetworkID id = va_arg( args, NetworkID );
            unsigned char axis = ( unsigned char ) va_arg( args, unsigned int );
            double value = va_arg( args, double );
            packet = new pObjectAngle( id, axis, value );
            break;
        }

        case ID_UPDATE_CELL:
        {
            NetworkID id = va_arg( args, NetworkID );
            unsigned int cell = va_arg( args, unsigned int );
            packet = new pObjectCell( id, cell );
            break;
        }

        case ID_UPDATE_VALUE:
        {
            NetworkID id = va_arg( args, NetworkID );
            bool base = ( bool ) va_arg( args, unsigned int );
            unsigned char index = ( unsigned char ) va_arg( args, unsigned int );
            double value = va_arg( args, double );
            packet = new pActorValue( id, base, index, value );
            break;
        }

        case ID_UPDATE_STATE:
        {
            NetworkID id = va_arg( args, NetworkID );
            unsigned char index = ( unsigned char ) va_arg( args, unsigned int );
            unsigned char moving = ( unsigned char ) va_arg( args, unsigned int );
            bool alerted = ( bool ) va_arg( args, unsigned int );
            bool sneaking = ( bool ) va_arg( args, unsigned int );
            packet = new pActorState( id, index, moving, alerted, sneaking );
            break;
        }

        default:
            throw VaultException( "Unhandled packet type %d", ( int ) type );
    }

    va_end( args );

    return packet;
}

pDefault* PacketFactory::CreatePacket( unsigned char* stream, unsigned int len )
{
    pDefault* packet;

    switch ( stream[0] )
    {
        case ID_GAME_CONFIRM:
            packet = new pGameConfirm( stream, len );
            break;

        case ID_GAME_AUTH:
            packet = new pGameAuth( stream, len );
            break;

        case ID_GAME_LOAD:
            packet = new pGameLoad( stream, len );
            break;

        case ID_GAME_MOD:
            packet = new pGameMod( stream, len );
            break;

        case ID_GAME_START:
            packet = new pGameStart( stream, len );
            break;

        case ID_GAME_END:
            packet = new pGameEnd( stream, len );
            break;

        case ID_OBJECT_NEW:
            packet = new pObjectNew( stream, len );
            break;

        case ID_ITEM_NEW:
            packet = new pObjectNew( stream, len );
            break;

        case ID_PLAYER_NEW:
            packet = new pPlayerNew( stream, len );
            break;

        case ID_PLAYER_LEFT:
            packet = new pPlayerLeft( stream, len );
            break;

        case ID_OBJECT_UPDATE:
        case ID_ACTOR_UPDATE:
        {
            if ( len < 2 )
                throw VaultException( "Incomplete object packet type %d", ( int ) stream[0] );

            switch ( stream[1] )
            {
                case ID_UPDATE_POS:
                    packet = new pObjectPos( stream, len );
                    break;

                case ID_UPDATE_ANGLE:
                    packet = new pObjectAngle( stream, len );
                    break;

                case ID_UPDATE_CELL:
                    packet = new pObjectCell( stream, len );
                    break;

                case ID_UPDATE_VALUE:
                    packet = new pActorValue( stream, len );
                    break;

                case ID_UPDATE_STATE:
                    packet = new pActorState( stream, len );
                    break;

                default:
                    throw VaultException( "Unhandled object update packet type %d", ( int ) stream[1] );
            }

            break;
        }

        default:
            throw VaultException( "Unhandled packet type %d", ( int ) stream[0] );
    }

    return packet;
}

void PacketFactory::Access( pDefault* packet, ... )
{
    va_list args;
    va_start( args, packet );

    try
    {
        switch ( packet->type.type )
        {
            case ID_GAME_CONFIRM:
            {
                pGameConfirm* data = dynamic_cast<pGameConfirm*>( packet );
                NetworkID* id = va_arg( args, NetworkID* );
                char* name = va_arg( args, char* );
                *id = data->_data.id;
                strncpy( name, data->_data.name, sizeof( data->_data.name ) );
                break;
            }

            case ID_GAME_AUTH:
            {
                pGameAuth* data = dynamic_cast<pGameAuth*>( packet );
                char* name = va_arg( args, char* );
                char* pwd = va_arg( args, char* );
                strncpy( name, data->_data.name, sizeof( data->_data.name ) );
                strncpy( pwd, data->_data.pwd, sizeof( data->_data.pwd ) );
                break;
            }

            case ID_GAME_LOAD:
            {
                pGameLoad* data = dynamic_cast<pGameLoad*>( packet );
                break;
            }

            case ID_GAME_MOD:
            {
                pGameMod* data = dynamic_cast<pGameMod*>( packet );
                char* modfile = va_arg( args, char* );
                unsigned int* crc = va_arg( args, unsigned int* );
                strncpy( modfile, data->_data.modfile, sizeof( data->_data.modfile ) );
                *crc = data->_data.crc;
                break;
            }

            case ID_GAME_START:
            {
                pGameStart* data = dynamic_cast<pGameStart*>( packet );
                char* savegame = va_arg( args, char* );
                unsigned int* crc = va_arg( args, unsigned int* );
                strncpy( savegame, data->_data.savegame, sizeof( data->_data.savegame ) );
                *crc = data->_data.crc;
                break;
            }

            case ID_GAME_END:
            {
                pGameEnd* data = dynamic_cast<pGameEnd*>( packet );
                unsigned char* reason = va_arg( args, unsigned char* );
                *reason = data->reason.type;
                break;
            }

            case ID_OBJECT_NEW:
            {
                pObjectNew* data = dynamic_cast<pObjectNew*>( packet );
                NetworkID* id = va_arg( args, NetworkID* );
                unsigned int* refID = va_arg( args, unsigned int* );
                unsigned int* baseID = va_arg( args, unsigned int* );
                char* name = va_arg( args, char* );
                double* X = va_arg(args, double*);
                double* Y = va_arg(args, double*);
                double* Z = va_arg(args, double*);
                double* aX = va_arg(args, double*);
                double* aY = va_arg(args, double*);
                double* aZ = va_arg(args, double*);
                unsigned int* cell = va_arg(args, unsigned int*);
                bool* enabled = va_arg(args, bool*);
                *id = data->id;
                *refID = data->refID;
                *baseID = data->baseID;
                strncpy( name, data->_data.name, sizeof( data->_data.name ) );
                *X = data->_data.X;
                *Y = data->_data.Y;
                *Z = data->_data.Z;
                *aX = data->_data.aX;
                *aY = data->_data.aY;
                *aZ = data->_data.aZ;
                *cell = data->_data.cell;
                *enabled = data->_data.enabled;
                break;
            }

            case ID_ITEM_NEW:
            {
                pItemNew* data = dynamic_cast<pItemNew*>( packet );
                pDefault** _pObjectNew = va_arg( args, pDefault** );
                unsigned int* count = va_arg( args, unsigned int* );
                double* condition = va_arg( args, double* );
                bool* equipped = va_arg( args, bool* );
                *_pObjectNew = new pObjectNew(data->id, data->refID, data->baseID, data->_data._data_pObjectNew);
                *count = data->_data.count;
                *condition = data->_data.condition;
                *equipped = data->_data.equipped;
                break;
            }

            case ID_PLAYER_NEW:
            {
                pPlayerNew* data = dynamic_cast<pPlayerNew*>( packet );
                NetworkID* id = va_arg( args, NetworkID* );
                unsigned int* refID = va_arg( args, unsigned int* );
                unsigned int* baseID = va_arg( args, unsigned int* );
                char* name = va_arg( args, char* );
                *id = data->id;
                *refID = data->refID;
                *baseID = data->baseID;
                strncpy( name, data->name, sizeof( data->name ) );
                break;
            }

            case ID_PLAYER_LEFT:
            {
                pPlayerLeft* data = dynamic_cast<pPlayerLeft*>( packet );
                NetworkID* id = va_arg( args, NetworkID* );
                *id = data->id;
                break;
            }

            case ID_OBJECT_UPDATE:
            case ID_ACTOR_UPDATE:
            {
                pObjectUpdateDefault* data = dynamic_cast<pObjectUpdateDefault*>( packet );

                switch ( data->sub_type.type )
                {
                    case ID_UPDATE_POS:
                    {
                        pObjectPos* update = dynamic_cast<pObjectPos*>( data );
                        NetworkID* id = va_arg( args, NetworkID* );
                        double* X = va_arg( args, double* );
                        double* Y = va_arg( args, double* );
                        double* Z = va_arg( args, double* );
                        *id = update->id;
                        *X = update->_data.X;
                        *Y = update->_data.Y;
                        *Z = update->_data.Z;
                        break;
                    }

                    case ID_UPDATE_ANGLE:
                    {
                        pObjectAngle* update = dynamic_cast<pObjectAngle*>( data );
                        NetworkID* id = va_arg( args, NetworkID* );
                        unsigned char* axis = va_arg( args, unsigned char* );
                        double* value = va_arg( args, double* );
                        *id = update->id;
                        *axis = update->_data.axis;
                        *value = update->_data.value;
                        break;
                    }

                    case ID_UPDATE_CELL:
                    {
                        pObjectCell* update = dynamic_cast<pObjectCell*>( data );
                        NetworkID* id = va_arg( args, NetworkID* );
                        unsigned int* cell = va_arg( args, unsigned int* );
                        *id = update->id;
                        *cell = update->cell;
                        break;
                    }

                    case ID_UPDATE_VALUE:
                    {
                        pActorValue* update = dynamic_cast<pActorValue*>( data );
                        NetworkID* id = va_arg( args, NetworkID* );
                        bool* base = va_arg( args, bool* );
                        unsigned char* index = va_arg( args, unsigned char* );
                        double* value = va_arg( args, double* );
                        *id = update->id;
                        *base = update->_data.base;
                        *index = update->_data.index;
                        *value = update->_data.value;
                        break;
                    }

                    case ID_UPDATE_STATE:
                    {
                        pActorState* update = dynamic_cast<pActorState*>( data );
                        NetworkID* id = va_arg( args, NetworkID* );
                        unsigned char* index = va_arg( args, unsigned char* );
                        unsigned char* moving = va_arg( args, unsigned char* );
                        bool* alerted = va_arg( args, bool* );
                        bool* sneaking = va_arg( args, bool* );
                        *id = update->id;
                        *index = update->_data.index;
                        *moving = update->_data.moving;
                        *alerted = update->_data.alerted;
                        *sneaking = update->_data.sneaking;
                        break;
                    }
                }

                break;
            }

            default:
                throw VaultException( "Unhandled packet type %d", ( int ) packet->type.type );
        }
    }

    catch ( ... )
    {
        va_end( args );
        throw;
    }

    va_end( args );
}

NetworkID PacketFactory::ExtractNetworkID( pDefault* packet)
{
    pObjectDefault* data = dynamic_cast<pObjectDefault*>( packet );
    return data->id;
}

unsigned int PacketFactory::ExtractRefID( pDefault* packet)
{
    pObjectNewDefault* data = dynamic_cast<pObjectNewDefault*>( packet );
    return data->refID;
}

unsigned int PacketFactory::ExtractBaseID( pDefault* packet)
{
    pObjectNewDefault* data = dynamic_cast<pObjectNewDefault*>( packet );
    return data->baseID;
}

const char* PacketFactory::ExtractRawData(pDefault* packet)
{
    switch ( packet->type.type )
    {
        case ID_OBJECT_NEW:
        {
            pObjectNew* data = dynamic_cast<pObjectNew*>( packet );
            return reinterpret_cast<const char*>(&data->_data);
        }

        default:
            throw VaultException( "Unhandled packet type %d", ( int ) packet->type.type );
    }

    return NULL;
}
