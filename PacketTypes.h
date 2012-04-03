#ifndef PACKETTYPES_H
#define PACKETTYPES_H

#include "vaultmp.h"
#include "VaultException.h"
#include "Data.h"

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

	private:
		pDefault( const pDefault& );
		pDefault& operator=( const pDefault& );

	protected:
		pDefault( unsigned char type )
		{
			this->type.type = type;
			this->stream = NULL;
			this->len = 0;
			base();
		};

		pDefault( const unsigned char* stream, unsigned int len )
		{
			this->stream = new unsigned char[len];
			memcpy( this->stream, stream, len );
			this->len = len;
			base();
		};

		pTypeSpecifier type;
		unsigned char* stream;
		unsigned int len;
		unsigned int base_len;

		virtual void construct( void*, unsigned int ) = 0;
		virtual void deconstruct( void*, unsigned int ) = 0;

		void base()
		{
			base_len = sizeof( pTypeSpecifier );
		};

	public:
		virtual ~pDefault()
		{
			if ( stream )
				delete[] stream;
		};

		const unsigned char* get() const
		{
			return stream;
		};

		unsigned int length() const
		{
			return len;
		};

		unsigned int base_length() const
		{
			return base_len;
		};
};

class pGameDefault : public pDefault
{
		friend class PacketFactory;

	private:
		pGameDefault( const pGameDefault& );
		pGameDefault& operator=( const pGameDefault& );

	protected:
		pGameDefault( unsigned char type ) : pDefault( type )
		{
			base();
		};

		pGameDefault( const unsigned char* stream, unsigned int len ) : pDefault( stream, len )
		{
			base();
		};

		void base()
		{

		};

		virtual void construct( void* super = NULL, unsigned int len = 0 )
		{
			if ( !stream )
			{
				unsigned int written = 0;
				stream = new unsigned char[base_len + len];
				memcpy( stream, &type, sizeof( pTypeSpecifier ) );
				written += sizeof( pTypeSpecifier );
				memcpy( ( void* ) ( stream + written ), super, len );
				written += len;
				this->len = written;
			}
		};

		virtual void deconstruct( void* super = NULL, unsigned int len = 0 )
		{
			if ( stream )
			{
				if ( base_len + len != this->len )
					throw VaultException( "Packet has size %d instead of expected %d bytes!", this->len, base_len + len );

				unsigned int read = 0;
				type = *reinterpret_cast<pTypeSpecifier*>( stream );
				read += sizeof( pTypeSpecifier );

				if ( super )
					memcpy( super, ( void* ) ( stream + read ), len );
			}
		};
};

class pObjectDefault : public pDefault
{
		friend class PacketFactory;

	private:
		pObjectDefault( const pObjectDefault& );
		pObjectDefault& operator=( const pObjectDefault& );

	protected:
		pObjectDefault( unsigned char type, NetworkID id ) : pDefault( type )
		{
			this->id = id;
			base();
		};

		pObjectDefault( const unsigned char* stream, unsigned int len ) : pDefault( stream, len )
		{
			base();
		};

		NetworkID id;

		void base()
		{
			base_len += sizeof( NetworkID );
		};

		virtual void construct( void* super = NULL, unsigned int len = 0 )
		{
			if ( !stream )
			{
				unsigned int written = 0;
				stream = new unsigned char[base_len + len];
				memcpy( stream, &type, sizeof( pTypeSpecifier ) );
				written += sizeof( pTypeSpecifier );
				memcpy( ( void* ) ( stream + written ), &id, sizeof( NetworkID ) );
				written += sizeof( NetworkID );
				memcpy( ( void* ) ( stream + written ), super, len );
				written += len;
				this->len = written;
			}
		};

		virtual void deconstruct( void* super = NULL, unsigned int len = 0 )
		{
			if ( stream )
			{
				if ( base_len + len != this->len )
					throw VaultException( "Packet has size %d instead of expected %d bytes!", this->len, base_len + len );

				unsigned int read = 0;
				type = *reinterpret_cast<pTypeSpecifier*>( stream );
				read += sizeof( pTypeSpecifier );
				id = *reinterpret_cast<NetworkID*>( stream + read );
				read += sizeof( NetworkID );

				if ( super )
					memcpy( super, ( void* ) ( stream + read ), len );
			}
		};
};

class pObjectNewDefault : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectNewDefault( const pObjectNewDefault& );
		pObjectNewDefault& operator=( const pObjectNewDefault& );

	protected:
		pObjectNewDefault( unsigned char type, unsigned int refID, unsigned int baseID, NetworkID id ) : pObjectDefault( type, id )
		{
			this->refID = refID;
			this->baseID = baseID;
			base();
		};

		pObjectNewDefault( const unsigned char* stream, unsigned int len ) : pObjectDefault( stream, len )
		{
			base();
		};

		unsigned int refID;
		unsigned int baseID;

		void base()
		{
			base_len += sizeof( unsigned int );
			base_len += sizeof( unsigned int );
		};

		virtual void construct( void* super = NULL, unsigned int len = 0 )
		{
			if ( !stream )
			{
				unsigned int written = 0;
				stream = new unsigned char[base_len + len];
				memcpy( stream, &type, sizeof( pTypeSpecifier ) );
				written += sizeof( pTypeSpecifier );
				memcpy( ( void* ) ( stream + written ), &id, sizeof( NetworkID ) );
				written += sizeof( NetworkID );
				memcpy( ( void* ) ( stream + written ), &refID, sizeof( unsigned int ) );
				written += sizeof( unsigned int );
				memcpy( ( void* ) ( stream + written ), &baseID, sizeof( unsigned int ) );
				written += sizeof( unsigned int );
				memcpy( ( void* ) ( stream + written ), super, len );
				written += len;
				this->len = written;
			}
		};

		virtual void deconstruct( void* super = NULL, unsigned int len = 0 )
		{
			if ( stream )
			{
				if ( base_len + len != this->len )
					throw VaultException( "Packet has size %d instead of expected %d bytes!", this->len, base_len + len );

				unsigned int read = 0;
				type = *reinterpret_cast<pTypeSpecifier*>( stream );
				read += sizeof( pTypeSpecifier );
				id = *reinterpret_cast<NetworkID*>( stream + read );
				read += sizeof( NetworkID );
				refID = *reinterpret_cast<unsigned int*>( stream + read );
				read += sizeof( unsigned int );
				baseID = *reinterpret_cast<unsigned int*>( stream + read );
				read += sizeof( unsigned int );

				if ( super )
					memcpy( super, ( void* ) ( stream + read ), len );
			}
		};
};

class pObjectUpdateDefault : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectUpdateDefault( const pObjectUpdateDefault& );
		pObjectUpdateDefault& operator=( const pObjectUpdateDefault& );

	protected:
		pObjectUpdateDefault( unsigned char type, unsigned char sub_type, NetworkID id ) : pObjectDefault( type, id )
		{
			this->sub_type.type = sub_type;
			base();
		};

		pObjectUpdateDefault( const unsigned char* stream, unsigned int len ) : pObjectDefault( stream, len )
		{
			base();
		};

		pTypeSpecifier sub_type;

		void base()
		{
			base_len += sizeof( pTypeSpecifier );
		};

		virtual void construct( void* super = NULL, unsigned int len = 0 )
		{
			if ( !stream )
			{
				unsigned int written = 0;
				stream = new unsigned char[sizeof( pTypeSpecifier ) + sizeof( pTypeSpecifier ) + sizeof( NetworkID ) + len];
				memcpy( stream, &type, sizeof( pTypeSpecifier ) );
				written += sizeof( pTypeSpecifier );
				memcpy( ( void* ) ( stream + written ), &sub_type, sizeof( pTypeSpecifier ) );
				written += sizeof( pTypeSpecifier );
				memcpy( ( void* ) ( stream + written ), &id, sizeof( NetworkID ) );
				written += sizeof( NetworkID );
				memcpy( ( void* ) ( stream + written ), super, len );
				written += len;
				this->len = written;
			}
		};

		virtual void deconstruct( void* super = NULL, unsigned int len = 0 )
		{
			if ( stream )
			{
				if ( base_len + len != this->len )
					throw VaultException( "Packet has size %d instead of expected %d,%d bytes!", this->len, base_len , len );

				unsigned int read = 0;
				type = *reinterpret_cast<pTypeSpecifier*>( stream );
				read += sizeof( pTypeSpecifier );
				sub_type = *reinterpret_cast<pTypeSpecifier*>( stream + read );
				read += sizeof( pTypeSpecifier );
				id = *reinterpret_cast<NetworkID*>( stream + read );
				read += sizeof( NetworkID );

				if ( super )
					memcpy( super, ( void* ) ( stream + read ), len );
			}
		};
};

/* ************************************** */

// Real types

/* ************************************** */

#pragma pack(push, 1)
    struct _pGameMod
    {
        char modfile[MAX_MOD_FILE];
        unsigned int crc;
    };

    struct _pGameAuth
    {
        char name[MAX_PLAYER_NAME];
        char pwd[MAX_PASSWORD_SIZE];
    };

    struct _pGameStart
    {
        char savegame[MAX_SAVEGAME_FILE];
        unsigned int crc;
    };
#pragma pack(pop)

class pGameAuth : public pGameDefault
{
		friend class PacketFactory;

	private:
		_pGameAuth _data;

		pGameAuth( const char* name, const char* pwd ) : pGameDefault( ID_GAME_AUTH )
		{
			ZeroMemory( &_data, sizeof( _data ) );
			strncpy( _data.name, name, sizeof( _data.name ) );
			strncpy( _data.pwd, pwd, sizeof( _data.pwd ) );
			construct( &_data, sizeof( _data ) );
		}
		pGameAuth( const unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pGameAuth( const pGameAuth& );
		pGameAuth& operator=( const pGameAuth& );
};

class pGameLoad : public pGameDefault
{
		friend class PacketFactory;

	private:
		pGameLoad() : pGameDefault( ID_GAME_LOAD )
		{
			construct();
		}
		pGameLoad( const unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
		{
			deconstruct();
		}

		pGameLoad( const pGameLoad& );
		pGameLoad& operator=( const pGameLoad& );
};

class pGameMod : public pGameDefault
{
		friend class PacketFactory;

	private:
		_pGameMod _data;

		pGameMod( const char* modfile, unsigned int crc ) : pGameDefault( ID_GAME_MOD )
		{
			ZeroMemory( &_data, sizeof( _data ) );
			strncpy( _data.modfile, modfile, sizeof( _data.modfile ) );
			_data.crc = crc;
			construct( &_data, sizeof( _data ) );
		}
		pGameMod( const unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pGameMod( const pGameMod& );
		pGameMod& operator=( const pGameMod& );
};

class pGameStart : public pGameDefault
{
		friend class PacketFactory;

	private:
		_pGameStart _data;

		pGameStart( const char* savegame, unsigned int crc ) : pGameDefault( ID_GAME_START )
		{
			ZeroMemory( &_data, sizeof( _data ) );
			strncpy( _data.savegame, savegame, sizeof( _data.savegame ) );
			_data.crc = crc;
			construct( &_data, sizeof( _data ) );
		}
		pGameStart( const unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pGameStart( const pGameStart& );
		pGameStart& operator=( const pGameStart& );
};

class pGameEnd : public pGameDefault
{
		friend class PacketFactory;

	private:
		pTypeSpecifier reason;

		pGameEnd( unsigned char _reason ) : pGameDefault( ID_GAME_END )
		{
			reason.type = _reason;
			construct( &reason, sizeof( reason ) );
		}
		pGameEnd( const unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
		{
			deconstruct( &reason, sizeof( reason ) );
		}

		pGameEnd( const pGameEnd& );
		pGameEnd& operator=( const pGameEnd& );
};

/* ************************************** */

#pragma pack(push, 1)
    struct _pObjectNew
    {
        char name[MAX_PLAYER_NAME];
        double X;
        double Y;
        double Z;
        double aX;
        double aY;
        double aZ;
        unsigned int cell;
        bool enabled;
    };

    struct _pItemNew
    {
        _pObjectNew _data_pObjectNew;
        unsigned int count;
        double condition;
        bool equipped;
    };
#pragma pack(pop)

class pObjectNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		_pObjectNew _data;

		pObjectNew( NetworkID id, unsigned int refID, unsigned int baseID, char* name, double X, double Y, double Z, double aX, double aY, double aZ, unsigned int cell, bool enabled) : pObjectNewDefault( ID_OBJECT_NEW, refID, baseID, id )
		{
			strncpy( this->_data.name, name, sizeof( this->_data.name ) );
			_data.X = X;
			_data.Y = Y;
			_data.Z = Z;
			_data.aX = aX;
			_data.aY = aY;
			_data.aZ = aZ;
			_data.cell = cell;
			_data.enabled = enabled;
			construct( &_data, sizeof( _data ) );
		}
		pObjectNew( NetworkID id, unsigned int refID, unsigned int baseID, const _pObjectNew& data) : pObjectNewDefault( ID_OBJECT_NEW, refID, baseID, id )
		{
            _data = data;
			construct( &_data, sizeof( _data ) );
		}
		pObjectNew( const unsigned char* stream, unsigned int len ) : pObjectNewDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pObjectNew( const pObjectNew& );
		pObjectNew& operator=( const pObjectNew& );

    public:
        inline
        static unsigned int data_length()
        {
            return sizeof(_pObjectNew);
        }
};

class pItemNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		_pItemNew _data;

		pItemNew(const pDefault* _data_pObjectNew, unsigned int count, double condition, bool equipped) : pObjectNewDefault( ID_ITEM_NEW, PacketFactory::ExtractReference(_data_pObjectNew), PacketFactory::ExtractBase(_data_pObjectNew), PacketFactory::ExtractNetworkID(_data_pObjectNew))
		{
            memcpy(&this->_data._data_pObjectNew, PacketFactory::ExtractRawData(_data_pObjectNew), pObjectNew::data_length());
			_data.count = count;
			_data.condition = condition;
			_data.equipped = equipped;
			construct( &_data, sizeof( _data ) );
		}
		pItemNew( NetworkID id, unsigned int refID, unsigned int baseID, _pItemNew& data) : pObjectNewDefault( ID_ITEM_NEW, refID, baseID, id )
		{
            _data = data;
			construct( &_data, sizeof( _data ) );
		}
		pItemNew( const unsigned char* stream, unsigned int len ) : pObjectNewDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pItemNew( const pItemNew& );
		pItemNew& operator=( const pItemNew& );

    public:
        inline
        static unsigned int data_length()
        {
            return sizeof(_pItemNew);
        }

        // I need a design solution for that

        inline
        static unsigned int as_packet_length()
        {
            return sizeof(pTypeSpecifier) + sizeof(NetworkID) + sizeof(unsigned int) + sizeof(unsigned int) + sizeof(_pItemNew);
        }

};

class pContainerNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		unsigned char* _data;

		pContainerNew(const pDefault* _data_pObjectNew, list<const pDefault*>& _data_pItemNew) : pObjectNewDefault( ID_CONTAINER_NEW, PacketFactory::ExtractReference(_data_pObjectNew), PacketFactory::ExtractBase(_data_pObjectNew), PacketFactory::ExtractNetworkID(_data_pObjectNew))
		{
            unsigned int at = 0;
            unsigned int length = pItemNew::as_packet_length();
            unsigned int size = _data_pItemNew.size();
            unsigned int mem = pObjectNew::data_length() + (length * size) + sizeof(unsigned int);
            _data = new unsigned char[mem];

            memcpy(&this->_data[0], PacketFactory::ExtractRawData(_data_pObjectNew), pObjectNew::data_length());
            at += pObjectNew::data_length();

            memcpy(&this->_data[at], &size, sizeof(unsigned int));
            at += sizeof(unsigned int);

            if (size > 0)
            {
                list<const pDefault*>::const_iterator it;

                for (it = _data_pItemNew.begin(); it != _data_pItemNew.end(); ++it, at += length)
                {
                    const pDefault* packet = *it;

                    if (packet->length() != length)
                    {
                        delete[] _data;
                        throw VaultException( "Packet has size %d instead of expected %d bytes!", packet->length(), length );
                    }

                    memcpy(&_data[at], packet->get(), length);
                }
            }

            construct( _data, mem);
		}
		pContainerNew( NetworkID id, unsigned int refID, unsigned int baseID, const unsigned char* data) : pObjectNewDefault( ID_CONTAINER_NEW, refID, baseID, id )
		{
		    unsigned int mem = pContainerNew::data_length(data);
            _data = new unsigned char[mem];
            memcpy(_data, data, mem);

			construct( _data, mem);
		}
		pContainerNew( const unsigned char* stream, unsigned int len ) : pObjectNewDefault( stream, len )
		{
            unsigned int mem = len - this->base_length();
            _data = new unsigned char[mem];

            deconstruct( _data, mem);
		}
		virtual ~pContainerNew()
		{
            delete[] _data;
		}

		pContainerNew( const pContainerNew& );
		pContainerNew& operator=( const pContainerNew& );

    public:
        static unsigned int data_length(const unsigned char* data)
        {
            unsigned int length = pItemNew::as_packet_length();
            unsigned int size = *reinterpret_cast<const unsigned int*>(&data[pObjectNew::data_length()]);
            unsigned int mem = pObjectNew::data_length() + (length * size) + sizeof(unsigned int);

            return mem;
        }
};

class pActorNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		unsigned char* _data;

		pActorNew(const pDefault* _data_pContainerNew, map<unsigned char, double>& values, map<unsigned char, double>& baseValues, unsigned char moving, unsigned char moving_xy, bool alerted, bool sneaking, bool dead) : pObjectNewDefault( ID_ACTOR_NEW, PacketFactory::ExtractReference(_data_pContainerNew), PacketFactory::ExtractBase(_data_pContainerNew), PacketFactory::ExtractNetworkID(_data_pContainerNew))
		{
            unsigned int at = 0;
            unsigned int length = sizeof(unsigned char) + sizeof(double);

            if (values.size() != baseValues.size())
                throw VaultException("Lengths of values / base values of an Actor must be equal!");

            unsigned int size = values.size();
            unsigned int container_length = pContainerNew::data_length(PacketFactory::ExtractRawData(_data_pContainerNew));
            unsigned int mem = container_length + (length * size * 2) + sizeof(unsigned int) + (sizeof(unsigned char) * 2) + (sizeof(bool) * 3);
            _data = new unsigned char[mem];

            memcpy(&this->_data[0], PacketFactory::ExtractRawData(_data_pContainerNew), container_length);
            at += container_length;

            memcpy(&this->_data[at], &size, sizeof(unsigned int));
            at += sizeof(unsigned int);

            if (size > 0)
            {
                map<unsigned char, double>::iterator it;

                for (it = values.begin(); it != values.end(); ++it, at += length)
                {
                    memcpy(&_data[at], &it->first, sizeof(unsigned char));
                    memcpy(&_data[at + sizeof(unsigned char)], &it->second, sizeof(double));
                }

                for (it = baseValues.begin(); it != baseValues.end(); ++it, at += length)
                {
                    memcpy(&_data[at], &it->first, sizeof(unsigned char));
                    memcpy(&_data[at + sizeof(unsigned char)], &it->second, sizeof(double));
                }
            }

            memcpy(&_data[at], &moving, sizeof(unsigned char));
            at += sizeof(unsigned char);

            memcpy(&_data[at], &moving_xy, sizeof(unsigned char));
            at += sizeof(unsigned char);

            memcpy(&_data[at], &alerted, sizeof(bool));
            at += sizeof(bool);

            memcpy(&_data[at], &sneaking, sizeof(bool));
            at += sizeof(bool);

            memcpy(&_data[at], &dead, sizeof(bool));
            at += sizeof(bool);

            construct( _data, mem);
		}
		pActorNew( NetworkID id, unsigned int refID, unsigned int baseID, const unsigned char* data) : pObjectNewDefault( ID_ACTOR_NEW, refID, baseID, id )
		{
		    unsigned int mem = pActorNew::data_length(data);
            _data = new unsigned char[mem];
            memcpy(_data, data, mem);

			construct( _data, mem);
		}
		pActorNew( const unsigned char* stream, unsigned int len ) : pObjectNewDefault( stream, len )
		{
            unsigned int mem = len - this->base_length();
            _data = new unsigned char[mem];

            deconstruct( _data, mem);
		}
		virtual ~pActorNew()
		{
            delete[] _data;
		}

		pActorNew( const pActorNew& );
		pActorNew& operator=( const pActorNew& );

    public:
        static unsigned int data_length(const unsigned char* data)
        {
            unsigned int mem = pContainerNew::data_length(data);
            unsigned int length = sizeof(unsigned char) + sizeof(double);
            unsigned int size = *reinterpret_cast<const unsigned int*>(&data[mem]);
            mem += (length * size * 2) + sizeof(unsigned int) + (sizeof(unsigned char) * 2) + (sizeof(bool) * 3);;

            return mem;
        }
};

class pPlayerNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		unsigned char* _data;

		pPlayerNew(const pDefault* _data_pActorNew, map<unsigned char, pair<unsigned char, bool> >& controls) : pObjectNewDefault( ID_PLAYER_NEW, PacketFactory::ExtractReference(_data_pActorNew), PacketFactory::ExtractBase(_data_pActorNew), PacketFactory::ExtractNetworkID(_data_pActorNew))
		{
            unsigned int at = 0;
            unsigned int length = sizeof(unsigned char) + sizeof(unsigned char) + sizeof(bool);
            unsigned int size = controls.size();
            unsigned int actor_length = pActorNew::data_length(PacketFactory::ExtractRawData(_data_pActorNew));
            unsigned int mem = actor_length + (length * size) + sizeof(unsigned int);
            _data = new unsigned char[mem];

            memcpy(&this->_data[0], PacketFactory::ExtractRawData(_data_pActorNew), actor_length);
            at += actor_length;

            memcpy(&this->_data[at], &size, sizeof(unsigned int));
            at += sizeof(unsigned int);

            if (size > 0)
            {
                map<unsigned char, pair<unsigned char, bool> >::iterator it;

                for (it = controls.begin(); it != controls.end(); ++it, at += length)
                {
                    memcpy(&_data[at], &it->first, sizeof(unsigned char));
                    memcpy(&_data[at + sizeof(unsigned char)], &it->second.first, sizeof(unsigned char));
                    memcpy(&_data[at + (sizeof(unsigned char) * 2)], &it->second.second, sizeof(bool));
                }
            }

            construct( _data, mem);
		}
		pPlayerNew( unsigned char* stream, unsigned int len ) : pObjectNewDefault( stream, len )
		{
            unsigned int mem = len - this->base_length();
            _data = new unsigned char[mem];

            deconstruct( _data, mem);
		}
		virtual ~pPlayerNew()
		{
            delete[] _data;
		}

		pPlayerNew( const pPlayerNew& );
		pPlayerNew& operator=( const pPlayerNew& );
};

class pObjectRemove : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pObjectRemove( NetworkID id ) : pObjectDefault( ID_OBJECT_REMOVE, id )
		{
			construct();
		}
		pObjectRemove( unsigned char* stream, unsigned int len ) : pObjectDefault( stream, len )
		{
			deconstruct();
		}

		pObjectRemove( const pObjectRemove& );
		pObjectRemove& operator=( const pObjectRemove& );
};

/* ************************************** */

#pragma pack(push, 1)
    struct _pObjectPos
    {
        double X;
        double Y;
        double Z;
    };

    struct _pObjectAngle
    {
        unsigned char axis;
        double value;
    };
#pragma pack(pop)

class pObjectPos : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pObjectPos _data;

		pObjectPos( NetworkID id, double X, double Y, double Z ) : pObjectUpdateDefault( ID_OBJECT_UPDATE, ID_UPDATE_POS, id )
		{
			_data.X = X;
			_data.Y = Y;
			_data.Z = Z;
			construct( &_data, sizeof( _data ) );
		}
		pObjectPos( unsigned char* stream, unsigned int len ) : pObjectUpdateDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pObjectPos( const pObjectPos& );
		pObjectPos& operator=( const pObjectPos& );
};

class pObjectAngle : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pObjectAngle _data;

		pObjectAngle( NetworkID id, unsigned char axis, double value ) : pObjectUpdateDefault( ID_OBJECT_UPDATE, ID_UPDATE_ANGLE, id )
		{
			_data.axis = axis;
			_data.value = value;
			construct( &_data, sizeof( _data ) );
		}
		pObjectAngle( unsigned char* stream, unsigned int len ) : pObjectUpdateDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pObjectAngle( const pObjectAngle& );
		pObjectAngle& operator=( const pObjectAngle& );
};

class pObjectCell : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		unsigned int cell;

		pObjectCell( NetworkID id, unsigned int cell ) : pObjectUpdateDefault( ID_OBJECT_UPDATE, ID_UPDATE_CELL, id )
		{
			this->cell = cell;
			construct( &this->cell, sizeof( this->cell ) );
		}
		pObjectCell( unsigned char* stream, unsigned int len ) : pObjectUpdateDefault( stream, len )
		{
			deconstruct( &this->cell, sizeof( this->cell ) );
		}

		pObjectCell( const pObjectCell& );
		pObjectCell& operator=( const pObjectCell& );
};

/* ************************************** */

#pragma pack(push, 1)
    struct _pActorValue
    {
        bool base;
        unsigned char index;
        double value;
    };

    struct _pActorState
    {
        unsigned char index;
        unsigned char moving;
        bool alerted;
        bool sneaking;
    };
#pragma pack(pop)

class pActorValue : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pActorValue _data;

		pActorValue( NetworkID id, bool base, unsigned char index, double value ) : pObjectUpdateDefault( ID_ACTOR_UPDATE, ID_UPDATE_VALUE, id )
		{
			_data.base = base;
			_data.index = index;
			_data.value = value;
			construct( &_data, sizeof( _data ) );
		}
		pActorValue( unsigned char* stream, unsigned int len ) : pObjectUpdateDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pActorValue( const pActorValue& );
		pActorValue& operator=( const pActorValue& );
};

class pActorState : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pActorState _data;

		pActorState( NetworkID id, unsigned char index, unsigned char moving, bool alerted, bool sneaking ) : pObjectUpdateDefault( ID_ACTOR_UPDATE, ID_UPDATE_STATE, id )
		{
			_data.index = index;
			_data.moving = moving;
			_data.alerted = alerted;
			_data.sneaking = sneaking;
			construct( &_data, sizeof( _data ) );
		}
		pActorState( unsigned char* stream, unsigned int len ) : pObjectUpdateDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pActorState( const pActorState& );
		pActorState& operator=( const pActorState& );
};

/* ************************************** */

#pragma pack(push, 1)
    struct _pPlayerControl
    {
        unsigned char control;
        unsigned char key;
    };
#pragma pack(pop)

class pPlayerControl : public pObjectUpdateDefault
{
		friend class PacketFactory;

	private:
		_pPlayerControl _data;

		pPlayerControl( NetworkID id, unsigned char control, unsigned char key) : pObjectUpdateDefault( ID_PLAYER_UPDATE, ID_UPDATE_CONTROL, id )
		{
			_data.control = control;
			_data.key = key;
			construct( &_data, sizeof( _data ) );
		}
		pPlayerControl( unsigned char* stream, unsigned int len ) : pObjectUpdateDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pPlayerControl( const pPlayerControl& );
		pPlayerControl& operator=( const pPlayerControl& );
};

#endif
