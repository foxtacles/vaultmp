#ifndef PACKETTYPES_H
#define PACKETTYPES_H

#include "vaultmp.h"
#include "VaultException.h"
#include "Data.h"

using namespace Data;

enum
{
    ID_EVENT_INTERFACE_LOST,
    ID_EVENT_CLIENT_ERROR,
    ID_EVENT_SERVER_ERROR,
    ID_EVENT_GAME_STARTED,
    ID_EVENT_AUTH_RECEIVED,
    ID_EVENT_CONFIRM_RECEIVED,
    ID_EVENT_CLOSE_RECEIVED,
};

enum
{
    ID_GAME_AUTH = ID_GAME_FIRST,
    ID_GAME_CONFIRM,
    ID_GAME_MOD,
    ID_GAME_START,
    ID_GAME_END,

    ID_PLAYER_NEW,
    ID_PLAYER_LEFT,

    ID_OBJECT_UPDATE,
    ID_ACTOR_UPDATE,
};

enum
{
    ID_UPDATE_POS,
    ID_UPDATE_ANGLE,
    ID_UPDATE_CELL,
    ID_UPDATE_VALUE,
};

enum
{
    ID_REASON_KICK = 0,
    ID_REASON_BAN,
    ID_REASON_ERROR,
    ID_REASON_DENIED,
    ID_REASON_NONE,
};

/* ************************************** */

// Standard definitions

/* ************************************** */

#pragma pack(push, 1)

struct pTypeSpecifier
{
    unsigned char type;
};

#pragma pack(pop)

class pDefault
{
friend class PacketFactory;

protected:
    pDefault(unsigned char type)
    {
        this->type.type = type;
        this->stream = NULL;
        this->len = 0;
        base();
    };

    pDefault(unsigned char* stream, unsigned int len)
    {
        this->stream = new unsigned char[len];
        memcpy(this->stream, stream, len);
        this->len = len;
        base();
    };

    pTypeSpecifier type;
    unsigned char* stream;
    unsigned int len;
    unsigned int base_len;

    virtual void construct(void*, unsigned int) = 0;
    virtual void deconstruct(void*, unsigned int) = 0;

    void base()
    {
        base_len = sizeof(pTypeSpecifier);
    };

public:
    virtual ~pDefault()
    {
        if (stream)
            delete[] stream;
    };

    const unsigned char* get()
    {
        return stream;
    };

    unsigned int length()
    {
        return len;
    };
};

class pGameDefault : public pDefault
{
friend class PacketFactory;

protected:
    pGameDefault(unsigned char type) : pDefault(type)
    {
        base();
    };

    pGameDefault(unsigned char* stream, unsigned int len) : pDefault(stream, len)
    {
        base();
    };

    void base()
    {

    };

    virtual void construct(void* super = NULL, unsigned int len = 0)
    {
        if (!stream)
        {
            unsigned int written = 0;
            stream = new unsigned char[base_len + len];
            memcpy(stream, &type, sizeof(pTypeSpecifier));
            written += sizeof(pTypeSpecifier);
            memcpy((void*) (stream + written), super, len);
            written += len;
            this->len = written;
        }
    };

    virtual void deconstruct(void* super = NULL, unsigned int len = 0)
    {
        if (stream)
        {
            if (base_len + len != this->len)
                throw VaultException("Packet has size %d instead of expected %d bytes!", this->len, base_len + len);

            unsigned int read = 0;
            type = *reinterpret_cast<pTypeSpecifier*>(stream);
            read += sizeof(pTypeSpecifier);
            if (super)
                memcpy(super, (void*) (stream + read), len);
        }
    };
};

class pObjectDefault : public pDefault
{
friend class PacketFactory;

protected:
    pObjectDefault(unsigned char type, NetworkID id) : pDefault(type)
    {
        this->id = id;
        base();
    };

    pObjectDefault(unsigned char* stream, unsigned int len) : pDefault(stream, len)
    {
        base();
    };

    NetworkID id;

    void base()
    {
        base_len += sizeof(NetworkID);
    };

    virtual void construct(void* super = NULL, unsigned int len = 0)
    {
        if (!stream)
        {
            unsigned int written = 0;
            stream = new unsigned char[base_len + len];
            memcpy(stream, &type, sizeof(pTypeSpecifier));
            written += sizeof(pTypeSpecifier);
            memcpy((void*) (stream + written), &id, sizeof(NetworkID));
            written += sizeof(NetworkID);
            memcpy((void*) (stream + written), super, len);
            written += len;
            this->len = written;
        }
    };

    virtual void deconstruct(void* super = NULL, unsigned int len = 0)
    {
        if (stream)
        {
            if (base_len + len != this->len)
                throw VaultException("Packet has size %d instead of expected %d bytes!", this->len, base_len + len);

            unsigned int read = 0;
            type = *reinterpret_cast<pTypeSpecifier*>(stream);
            read += sizeof(pTypeSpecifier);
            id = *reinterpret_cast<NetworkID*>(stream + read);
            read += sizeof(NetworkID);
            if (super)
                memcpy(super, (void*) (stream + read), len);
        }
    };
};

class pObjectNewDefault : public pObjectDefault
{
friend class PacketFactory;

protected:
    pObjectNewDefault(unsigned char type, unsigned int refID, unsigned int baseID, NetworkID id) : pObjectDefault(type, id)
    {
        this->refID = refID;
        this->baseID = baseID;
        base();
    };

    pObjectNewDefault(unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
    {
        base();
    };

    unsigned int refID;
    unsigned int baseID;

    void base()
    {
        base_len += sizeof(unsigned int);
        base_len += sizeof(unsigned int);
    };

    virtual void construct(void* super = NULL, unsigned int len = 0)
    {
        if (!stream)
        {
            unsigned int written = 0;
            stream = new unsigned char[base_len + len];
            memcpy(stream, &type, sizeof(pTypeSpecifier));
            written += sizeof(pTypeSpecifier);
            memcpy((void*) (stream + written), &id, sizeof(NetworkID));
            written += sizeof(NetworkID);
            memcpy((void*) (stream + written), &refID, sizeof(unsigned int));
            written += sizeof(unsigned int);
            memcpy((void*) (stream + written), &baseID, sizeof(unsigned int));
            written += sizeof(unsigned int);
            memcpy((void*) (stream + written), super, len);
            written += len;
            this->len = written;
        }
    };

    virtual void deconstruct(void* super = NULL, unsigned int len = 0)
    {
        if (stream)
        {
            if (base_len + len != this->len)
                throw VaultException("Packet has size %d instead of expected %d bytes!", this->len, base_len + len);

            unsigned int read = 0;
            type = *reinterpret_cast<pTypeSpecifier*>(stream);
            read += sizeof(pTypeSpecifier);
            id = *reinterpret_cast<NetworkID*>(stream + read);
            read += sizeof(NetworkID);
            refID = *reinterpret_cast<unsigned int*>(stream + read);
            read += sizeof(unsigned int);
            baseID = *reinterpret_cast<unsigned int*>(stream + read);
            read += sizeof(unsigned int);
            if (super)
                memcpy(super, (void*) (stream + read), len);
        }
    };
};

class pObjectUpdateDefault : public pObjectDefault
{
friend class PacketFactory;

protected:
    pObjectUpdateDefault(unsigned char type, unsigned char sub_type, NetworkID id) : pObjectDefault(type, id)
    {
        this->sub_type.type = sub_type;
        base();
    };

    pObjectUpdateDefault(unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
    {
        base();
    };

    pTypeSpecifier sub_type;

    void base()
    {
        base_len += sizeof(pTypeSpecifier);
    };

    virtual void construct(void* super = NULL, unsigned int len = 0)
    {
        if (!stream)
        {
            unsigned int written = 0;
            stream = new unsigned char[sizeof(pTypeSpecifier) + sizeof(pTypeSpecifier) + sizeof(NetworkID) + len];
            memcpy(stream, &type, sizeof(pTypeSpecifier));
            written += sizeof(pTypeSpecifier);
            memcpy((void*) (stream + written), &sub_type, sizeof(pTypeSpecifier));
            written += sizeof(pTypeSpecifier);
            memcpy((void*) (stream + written), &id, sizeof(NetworkID));
            written += sizeof(NetworkID);
            memcpy((void*) (stream + written), super, len);
            written += len;
            this->len = written;
        }
    };

    virtual void deconstruct(void* super = NULL, unsigned int len = 0)
    {
        if (stream)
        {
            if (base_len + len != this->len)
                throw VaultException("Packet has size %d instead of expected %d,%d bytes!", this->len, base_len , len);

            unsigned int read = 0;
            type = *reinterpret_cast<pTypeSpecifier*>(stream);
            read += sizeof(pTypeSpecifier);
            sub_type = *reinterpret_cast<pTypeSpecifier*>(stream + read);
            read += sizeof(pTypeSpecifier);
            id = *reinterpret_cast<NetworkID*>(stream + read);
            read += sizeof(NetworkID);
            if (super)
                memcpy(super, (void*) (stream + read), len);
        }
    };
};

/* ************************************** */

// Real types

/* ************************************** */

class pGameConfirm : public pGameDefault
{
friend class PacketFactory;

private:
#pragma pack(push, 1)
    struct _pGameConfirm
    {
        NetworkID id;
        char name[MAX_PLAYER_NAME];
    };
#pragma pack(pop)

    _pGameConfirm _data;

    pGameConfirm(NetworkID id, const char* name) : pGameDefault(ID_GAME_CONFIRM)
    {
        ZeroMemory(&_data, sizeof(_data));
        _data.id = id;
        strncpy(_data.name, name, sizeof(_data.name));
        construct(&_data, sizeof(_data));
    }
    pGameConfirm(unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
    {
        deconstruct(&_data, sizeof(_data));
    }
};

class pGameAuth : public pGameDefault
{
friend class PacketFactory;

private:
#pragma pack(push, 1)
    struct _pGameAuth
    {
        char name[MAX_PLAYER_NAME];
        char pwd[MAX_PASSWORD_SIZE];
    };
#pragma pack(pop)

    _pGameAuth _data;

    pGameAuth(const char* name, const char* pwd) : pGameDefault(ID_GAME_AUTH)
    {
        ZeroMemory(&_data, sizeof(_data));
        strncpy(_data.name, name, sizeof(_data.name));
        strncpy(_data.pwd, pwd, sizeof(_data.pwd));
        construct(&_data, sizeof(_data));
    }
    pGameAuth(unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
    {
        deconstruct(&_data, sizeof(_data));
    }
};

class pGameMod : public pGameDefault
{
friend class PacketFactory;

private:
#pragma pack(push, 1)
    struct _pGameMod
    {
        char modfile[MAX_MOD_FILE];
        unsigned int crc;
    };
#pragma pack(pop)

    _pGameMod _data;

    pGameMod(const char* modfile, unsigned int crc) : pGameDefault(ID_GAME_MOD)
    {
        ZeroMemory(&_data, sizeof(_data));
        strncpy(_data.modfile, modfile, sizeof(_data.modfile));
        _data.crc = crc;
        construct(&_data, sizeof(_data));
    }
    pGameMod(unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
    {
        deconstruct(&_data, sizeof(_data));
    }
};

class pGameStart : public pGameDefault
{
friend class PacketFactory;

private:
#pragma pack(push, 1)
    struct _pGameStart
    {
        char savegame[MAX_SAVEGAME_FILE];
        unsigned int crc;
    };
#pragma pack(pop)

    _pGameStart _data;

    pGameStart(const char* savegame, unsigned int crc) : pGameDefault(ID_GAME_START)
    {
        ZeroMemory(&_data, sizeof(_data));
        strncpy(_data.savegame, savegame, sizeof(_data.savegame));
        _data.crc = crc;
        construct(&_data, sizeof(_data));
    }
    pGameStart(unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
    {
        deconstruct(&_data, sizeof(_data));
    }
};

class pGameEnd : public pGameDefault
{
friend class PacketFactory;

private:
    pTypeSpecifier reason;

    pGameEnd(unsigned char _reason) : pGameDefault(ID_GAME_END)
    {
        reason.type = _reason;
        construct(&reason, sizeof(reason));
    }
    pGameEnd(unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
    {
        deconstruct(&reason, sizeof(reason));
    }
};

/* ************************************** */

class pPlayerNew : public pObjectNewDefault
{
friend class PacketFactory;

private:
    char name[MAX_PLAYER_NAME];

    pPlayerNew(NetworkID id, unsigned int refID, unsigned int baseID, char* name) : pObjectNewDefault(ID_PLAYER_NEW, refID, baseID, id)
    {
        ZeroMemory(this->name, sizeof(this->name));
        strncpy(this->name, name, sizeof(this->name));
        construct(name, sizeof(this->name));
    }
    pPlayerNew(unsigned char* stream, unsigned int len) : pObjectNewDefault(stream, len)
    {
        deconstruct(name, sizeof(this->name));
    }
};

class pPlayerLeft : public pObjectDefault
{
friend class PacketFactory;

private:
    pPlayerLeft(NetworkID id) : pObjectDefault(ID_PLAYER_LEFT, id)
    {
        construct();
    }
    pPlayerLeft(unsigned char* stream, unsigned int len) : pObjectDefault(stream, len)
    {
        deconstruct();
    }
};

/* ************************************** */

class pObjectPos : public pObjectUpdateDefault
{
friend class PacketFactory;

private:
#pragma pack(push, 1)
    struct _pObjectPos
    {
        unsigned char axis;
        double value;
    };
#pragma pack(pop)

    _pObjectPos _data;

    pObjectPos(NetworkID id, unsigned char axis, double value) : pObjectUpdateDefault(ID_OBJECT_UPDATE, ID_UPDATE_POS, id)
    {
        _data.axis = axis;
        _data.value = value;
        construct(&_data, sizeof(_data));
    }
    pObjectPos(unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
    {
        deconstruct(&_data, sizeof(_data));
    }
};

class pObjectAngle : public pObjectUpdateDefault
{
friend class PacketFactory;

private:
#pragma pack(push, 1)
    struct _pObjectAngle
    {
        unsigned char axis;
        double value;
    };
#pragma pack(pop)

    _pObjectAngle _data;

    pObjectAngle(NetworkID id, unsigned char axis, double value) : pObjectUpdateDefault(ID_OBJECT_UPDATE, ID_UPDATE_ANGLE, id)
    {
        _data.axis = axis;
        _data.value = value;
        construct(&_data, sizeof(_data));
    }
    pObjectAngle(unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
    {
        deconstruct(&_data, sizeof(_data));
    }
};

class pObjectCell : public pObjectUpdateDefault
{
friend class PacketFactory;

private:
    unsigned int cell;

    pObjectCell(NetworkID id, unsigned int cell) : pObjectUpdateDefault(ID_OBJECT_UPDATE, ID_UPDATE_CELL, id)
    {
        this->cell = cell;
        construct(&this->cell, sizeof(this->cell));
    }
    pObjectCell(unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
    {
        deconstruct(&this->cell, sizeof(this->cell));
    }
};

/* ************************************** */

class pActorValue : public pObjectUpdateDefault
{
friend class PacketFactory;

private:
#pragma pack(push, 1)
    struct _pActorValue
    {
        bool base;
        unsigned char index;
        double value;
    };
#pragma pack(pop)

    _pActorValue _data;

    pActorValue(NetworkID id, bool base, unsigned char index, double value) : pObjectUpdateDefault(ID_ACTOR_UPDATE, ID_UPDATE_VALUE, id)
    {
        _data.base = base;
        _data.index = index;
        _data.value = value;
        construct(&_data, sizeof(_data));
    }
    pActorValue(unsigned char* stream, unsigned int len) : pObjectUpdateDefault(stream, len)
    {
        deconstruct(&_data, sizeof(_data));
    }
};


/* ************************************** */

// Factory

/* ************************************** */

class PacketFactory
{
private:
    PacketFactory();

public:
    static pDefault* CreatePacket(unsigned char type, ...)
    {
        va_list args;
        va_start(args, type);
        pDefault* packet;

        switch (type)
        {
        case ID_GAME_CONFIRM:
        {
            NetworkID id = va_arg(args, NetworkID);
            char* name = va_arg(args, char*);
            packet = new pGameConfirm(id, name);
            break;
        }

        case ID_GAME_AUTH:
        {
            char* name = va_arg(args, char*);
            char* pwd = va_arg(args, char*);
            packet = new pGameAuth(name, pwd);
            break;
        }

        case ID_GAME_MOD:
        {
            char* modfile = va_arg(args, char*);
            unsigned int crc = va_arg(args, unsigned int);
            packet = new pGameMod(modfile, crc);
            break;
        }

        case ID_GAME_START:
        {
            char* savegame = va_arg(args, char*);
            unsigned int crc = va_arg(args, unsigned int);
            packet = new pGameStart(savegame, crc);
            break;
        }

        case ID_GAME_END:
        {
            unsigned char reason = (unsigned char) va_arg(args, unsigned int);
            packet = new pGameEnd(reason);
            break;
        }

        case ID_PLAYER_NEW:
        {
            NetworkID id = va_arg(args, NetworkID);
            unsigned int refID = va_arg(args, unsigned int);
            unsigned int baseID = va_arg(args, unsigned int);
            char* name = va_arg(args, char*);
            packet = new pPlayerNew(id, refID, baseID, name);
            break;
        }

        case ID_PLAYER_LEFT:
        {
            NetworkID id = va_arg(args, NetworkID);
            packet = new pPlayerLeft(id);
            break;
        }

        case ID_UPDATE_POS:
        {
            NetworkID id = va_arg(args, NetworkID);
            unsigned char axis = (unsigned char) va_arg(args, unsigned int);
            double value = va_arg(args, double);
            packet = new pObjectPos(id, axis, value);
            break;
        }

        case ID_UPDATE_ANGLE:
        {
            NetworkID id = va_arg(args, NetworkID);
            unsigned char axis = (unsigned char) va_arg(args, unsigned int);
            double value = va_arg(args, double);
            packet = new pObjectAngle(id, axis, value);
            break;
        }

        case ID_UPDATE_CELL:
        {
            NetworkID id = va_arg(args, NetworkID);
            unsigned int cell = va_arg(args, unsigned int);
            packet = new pObjectCell(id, cell);
            break;
        }

        case ID_UPDATE_VALUE:
        {
            NetworkID id = va_arg(args, NetworkID);
            bool base = (bool) va_arg(args, unsigned int);
            unsigned char index = (unsigned char) va_arg(args, unsigned int);
            double value = va_arg(args, double);
            packet = new pActorValue(id, base, index, value);
            break;
        }

        default:
            throw VaultException("Unhandled packet type %d", (int) type);
        }

        va_end(args);

        return packet;
    }

    static pDefault* CreatePacket(unsigned char* stream, unsigned int len)
    {
        pDefault* packet;

        switch (stream[0])
        {
        case ID_GAME_CONFIRM:
            packet = new pGameConfirm(stream, len);
            break;
        case ID_GAME_AUTH:
            packet = new pGameAuth(stream, len);
            break;
        case ID_GAME_MOD:
            packet = new pGameMod(stream, len);
            break;
        case ID_GAME_START:
            packet = new pGameStart(stream, len);
            break;
        case ID_GAME_END:
            packet = new pGameEnd(stream, len);
            break;
        case ID_PLAYER_NEW:
            packet = new pPlayerNew(stream, len);
            break;
        case ID_PLAYER_LEFT:
            packet = new pPlayerLeft(stream, len);
            break;
        case ID_OBJECT_UPDATE:
        case ID_ACTOR_UPDATE:
        {
            if (len < 2)
                throw VaultException("Incomplete object packet type %d", (int) stream[0]);
            switch (stream[1])
            {
            case ID_UPDATE_POS:
                packet = new pObjectPos(stream, len);
                break;
            case ID_UPDATE_ANGLE:
                packet = new pObjectAngle(stream, len);
                break;
            case ID_UPDATE_CELL:
                packet = new pObjectCell(stream, len);
                break;
            case ID_UPDATE_VALUE:
                packet = new pActorValue(stream, len);
                break;
            default:
                throw VaultException("Unhandled object update packet type %d", (int) stream[1]);
            }
            break;
        }
        default:
            throw VaultException("Unhandled packet type %d", (int) stream[0]);
        }

        return packet;
    }

    static void Access(pDefault* packet, ...)
    {
        va_list args;
        va_start(args, packet);

        try
        {
            switch (packet->type.type)
            {
            case ID_GAME_CONFIRM:
            {
                pGameConfirm* data = dynamic_cast<pGameConfirm*>(packet);
                NetworkID* id = va_arg(args, NetworkID*);
                char* name = va_arg(args, char*);
                *id = data->_data.id;
                strncpy(name, data->_data.name, sizeof(data->_data.name));
                break;
            }

            case ID_GAME_AUTH:
            {
                pGameAuth* data = dynamic_cast<pGameAuth*>(packet);
                char* name = va_arg(args, char*);
                char* pwd = va_arg(args, char*);
                strncpy(name, data->_data.name, sizeof(data->_data.name));
                strncpy(pwd, data->_data.pwd, sizeof(data->_data.pwd));
                break;
            }

            case ID_GAME_MOD:
            {
                pGameMod* data = dynamic_cast<pGameMod*>(packet);
                char* modfile = va_arg(args, char*);
                unsigned int* crc = va_arg(args, unsigned int*);
                strncpy(modfile, data->_data.modfile, sizeof(data->_data.modfile));
                *crc = data->_data.crc;
                break;
            }

            case ID_GAME_START:
            {
                pGameStart* data = dynamic_cast<pGameStart*>(packet);
                char* savegame = va_arg(args, char*);
                unsigned int* crc = va_arg(args, unsigned int*);
                strncpy(savegame, data->_data.savegame, sizeof(data->_data.savegame));
                *crc = data->_data.crc;
                break;
            }

            case ID_GAME_END:
            {
                pGameEnd* data = dynamic_cast<pGameEnd*>(packet);
                unsigned char* reason = va_arg(args, unsigned char*);
                *reason = data->reason.type;
                break;
            }

            case ID_PLAYER_NEW:
            {
                pPlayerNew* data = dynamic_cast<pPlayerNew*>(packet);
                NetworkID* id = va_arg(args, NetworkID*);
                unsigned int* refID = va_arg(args, unsigned int*);
                unsigned int* baseID = va_arg(args, unsigned int*);
                char* name = va_arg(args, char*);
                *id = data->id;
                *refID = data->refID;
                *baseID = data->baseID;
                strncpy(name, data->name, sizeof(data->name));
                break;
            }

            case ID_PLAYER_LEFT:
            {
                pPlayerLeft* data = dynamic_cast<pPlayerLeft*>(packet);
                NetworkID* id = va_arg(args, NetworkID*);
                *id = data->id;
                break;
            }

            case ID_OBJECT_UPDATE:
            case ID_ACTOR_UPDATE:
            {
                pObjectUpdateDefault* data = dynamic_cast<pObjectUpdateDefault*>(packet);

                switch (data->sub_type.type)
                {
                    case ID_UPDATE_POS:
                    {
                        pObjectPos* update = dynamic_cast<pObjectPos*>(data);
                        NetworkID* id = va_arg(args, NetworkID*);
                        unsigned char* axis = va_arg(args, unsigned char*);
                        double* value = va_arg(args, double*);
                        *id = update->id;
                        *axis = update->_data.axis;
                        *value = update->_data.value;
                        break;
                    }
                    case ID_UPDATE_ANGLE:
                    {
                        pObjectAngle* update = dynamic_cast<pObjectAngle*>(data);
                        NetworkID* id = va_arg(args, NetworkID*);
                        unsigned char* axis = va_arg(args, unsigned char*);
                        double* value = va_arg(args, double*);
                        *id = update->id;
                        *axis = update->_data.axis;
                        *value = update->_data.value;
                        break;
                    }
                    case ID_UPDATE_CELL:
                    {
                        pObjectCell* update = dynamic_cast<pObjectCell*>(data);
                        NetworkID* id = va_arg(args, NetworkID*);
                        unsigned int* cell = va_arg(args, unsigned int*);
                        *id = update->id;
                        *cell = update->cell;
                        break;
                    }
                    case ID_UPDATE_VALUE:
                    {
                        pActorValue* update = dynamic_cast<pActorValue*>(data);
                        NetworkID* id = va_arg(args, NetworkID*);
                        bool* base = va_arg(args, bool*);
                        unsigned char* index = va_arg(args, unsigned char*);
                        double* value = va_arg(args, double*);
                        *id = update->id;
                        *base = update->_data.base;
                        *index = update->_data.index;
                        *value = update->_data.value;
                        break;
                    }
                }
                break;
            }

            default:
                throw VaultException("Unhandled packet type %d", (int) packet->type.type);
            }

        }
        catch (...)
        {
            va_end(args);
            throw;
        }

        va_end(args);
    }
};

#endif
