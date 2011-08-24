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
    ID_PLAYER_UPDATE,
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
        this->base_len = sizeof(pTypeSpecifier);
    };

public:
    virtual ~pDefault()
    {
        if (stream) delete[] stream;
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
        this->base_len = pDefault::base_len;
    };

    void construct(void* super = NULL, unsigned int len = 0)
    {
        if (!stream)
        {
            unsigned int written = 0;
            stream = new unsigned char[sizeof(pTypeSpecifier) + len];
            memcpy(stream, &type, sizeof(pTypeSpecifier));
            written += sizeof(pTypeSpecifier);
            memcpy((void*) (stream + written), super, len);
            written += len;
            this->len = written;
        }
    };

    void deconstruct(void* super = NULL, unsigned int len = 0)
    {
        if (stream)
        {
            if (this->base_len + len != this->len)
                throw VaultException("Packet has size %d instead of expected %d bytes!", this->len, this->base_len + len);

            unsigned int read = 0;
            type = *reinterpret_cast<pTypeSpecifier*>(stream);
            read += sizeof(pTypeSpecifier);
            if (super) memcpy(super, (void*) (stream + read), len);
        }
    };
};

class pPlayerDefault : public pDefault
{
protected:
    pPlayerDefault(unsigned char type, RakNetGUID guid) : pDefault(type)
    {
        this->guid = guid;
        base();
    };
    pPlayerDefault(unsigned char* stream, unsigned int len) : pDefault(stream, len)
    {
        base();
    };
    RakNetGUID guid;

    void base()
    {
        this->base_len = sizeof(RakNetGUID);
        this->base_len += pDefault::base_len;
    };

    void construct(void* super = NULL, unsigned int len = 0)
    {
        if (!stream)
        {
            unsigned int written = 0;
            stream = new unsigned char[sizeof(pTypeSpecifier) + sizeof(RakNetGUID) + len];
            memcpy(stream, &type, sizeof(pTypeSpecifier));
            written += sizeof(pTypeSpecifier);
            memcpy((void*) (stream + written), &guid, sizeof(RakNetGUID));
            written += sizeof(RakNetGUID);
            memcpy((void*) (stream + written), super, len);
            written += len;
            this->len = written;
        }
    };

    void deconstruct(void* super = NULL, unsigned int len = 0)
    {
        if (stream)
        {
            if (this->base_len + len != this->len)
                throw VaultException("Packet has size %d instead of expected %d bytes!", this->len, this->base_len + len);

            unsigned int read = 0;
            type = *reinterpret_cast<pTypeSpecifier*>(stream);
            read += sizeof(pTypeSpecifier);
            guid = *reinterpret_cast<RakNetGUID*>(stream + read);
            read += sizeof(RakNetGUID);
            if (super) memcpy(super, (void*) (stream + read), len);
        }
    };
};

class pPlayerUpdateDefault : public pPlayerDefault
{
protected:
    pPlayerUpdateDefault(unsigned char type, unsigned char sub_type, RakNetGUID guid) : pPlayerDefault(type, guid)
    {
        this->sub_type.type = sub_type;
        base();
    };
    pPlayerUpdateDefault(unsigned char* stream, unsigned int len) : pPlayerDefault(stream, len)
    {
        base();
    };
    pTypeSpecifier sub_type;

    void base()
    {
        this->base_len = sizeof(pTypeSpecifier);
        this->base_len += pPlayerDefault::base_len;
    };

    void construct(void* super = NULL, unsigned int len = 0)
    {
        if (!stream)
        {
            unsigned int written = 0;
            stream = new unsigned char[sizeof(pTypeSpecifier) + sizeof(pTypeSpecifier) + sizeof(RakNetGUID) + len];
            memcpy(stream, &type, sizeof(pTypeSpecifier));
            written += sizeof(pTypeSpecifier);
            memcpy((void*) (stream + written), &sub_type, sizeof(pTypeSpecifier));
            written += sizeof(pTypeSpecifier);
            memcpy((void*) (stream + written), &guid, sizeof(RakNetGUID));
            written += sizeof(RakNetGUID);
            memcpy((void*) (stream + written), super, len);
            written += len;
            this->len = written;
        }
    };

    void deconstruct(void* super = NULL, unsigned int len = 0)
    {
        if (stream)
        {
            if (this->base_len + len != this->len)
                throw VaultException("Packet has size %d instead of expected %d bytes!", this->len, this->base_len + len);

            unsigned int read = 0;
            type = *reinterpret_cast<pTypeSpecifier*>(stream);
            read += sizeof(pTypeSpecifier);
            sub_type = *reinterpret_cast<pTypeSpecifier*>(stream + read);
            read += sizeof(pTypeSpecifier);
            guid = *reinterpret_cast<RakNetGUID*>(stream + read);
            read += sizeof(RakNetGUID);
            if (super) memcpy(super, (void*) (stream + read), len);
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
    pGameConfirm() : pGameDefault(ID_GAME_CONFIRM)
    {
        construct();
    }
    pGameConfirm(unsigned char* stream, unsigned int len) : pGameDefault(stream, len)
    {
        deconstruct();
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

class pPlayerNew : public pPlayerDefault
{
    friend class PacketFactory;

private:
#pragma pack(push, 1)
    struct _pPlayerNew
    {
        char name[MAX_PLAYER_NAME];
        unsigned int baseID;
    };
#pragma pack(pop)

    _pPlayerNew _data;

    pPlayerNew(RakNetGUID guid, char* name, unsigned int baseID) : pPlayerDefault(ID_PLAYER_NEW, guid)
    {
        ZeroMemory(&_data, sizeof(_data));
        strncpy(_data.name, name, sizeof(_data.name));
        _data.baseID = baseID;
        construct(&_data, sizeof(_data));
    }
    pPlayerNew(unsigned char* stream, unsigned int len) : pPlayerDefault(stream, len)
    {
        deconstruct(&_data, sizeof(_data));
    }
};

class pPlayerLeft : public pPlayerDefault
{
    friend class PacketFactory;

private:
    pPlayerLeft(RakNetGUID guid) : pPlayerDefault(ID_PLAYER_LEFT, guid)
    {
        construct();
    }
    pPlayerLeft(unsigned char* stream, unsigned int len) : pPlayerDefault(stream, len)
    {
        deconstruct();
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
            packet = new pGameConfirm();
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
            RakNetGUID guid = va_arg(args, RakNetGUID);
            char* name = va_arg(args, char*);
            unsigned int baseID = va_arg(args, unsigned int);
            packet = new pPlayerNew(guid, name, baseID);
            break;
        }

        case ID_PLAYER_LEFT:
        {
            RakNetGUID guid = va_arg(args, RakNetGUID);
            packet = new pPlayerLeft(guid);
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
                RakNetGUID* guid = va_arg(args, RakNetGUID*);
                char* name = va_arg(args, char*);
                unsigned int* baseID = va_arg(args, unsigned int*);
                *guid = data->guid;
                strncpy(name, data->_data.name, sizeof(data->_data.name));
                *baseID = data->_data.baseID;
                break;
            }

            case ID_PLAYER_LEFT:
            {
                pPlayerLeft* data = dynamic_cast<pPlayerLeft*>(packet);
                RakNetGUID* guid = va_arg(args, RakNetGUID*);
                *guid = data->guid;
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
