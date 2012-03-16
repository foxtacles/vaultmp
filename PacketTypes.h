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

		pDefault( unsigned char* stream, unsigned int len )
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

	private:
		pGameDefault( const pGameDefault& );
		pGameDefault& operator=( const pGameDefault& );

	protected:
		pGameDefault( unsigned char type ) : pDefault( type )
		{
			base();
		};

		pGameDefault( unsigned char* stream, unsigned int len ) : pDefault( stream, len )
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

		pObjectDefault( unsigned char* stream, unsigned int len ) : pDefault( stream, len )
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

		pObjectNewDefault( unsigned char* stream, unsigned int len ) : pObjectDefault( stream, len )
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

		pObjectUpdateDefault( unsigned char* stream, unsigned int len ) : pObjectDefault( stream, len )
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
    struct _pGameConfirm
    {
        NetworkID id;
        char name[MAX_PLAYER_NAME];
    };

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

class pGameConfirm : public pGameDefault
{
		friend class PacketFactory;

	private:
		_pGameConfirm _data;

		pGameConfirm( NetworkID id, const char* name ) : pGameDefault( ID_GAME_CONFIRM )
		{
			ZeroMemory( &_data, sizeof( _data ) );
			_data.id = id;
			strncpy( _data.name, name, sizeof( _data.name ) );
			construct( &_data, sizeof( _data ) );
		}
		pGameConfirm( unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pGameConfirm( const pGameConfirm& );
		pGameConfirm& operator=( const pGameConfirm& );
};

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
		pGameAuth( unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
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
		pGameLoad( unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
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
		pGameMod( unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
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
		pGameStart( unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
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
		pGameEnd( unsigned char* stream, unsigned int len ) : pGameDefault( stream, len )
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
		pObjectNew( NetworkID id, unsigned int refID, unsigned int baseID, _pObjectNew& data) : pObjectNewDefault( ID_OBJECT_NEW, refID, baseID, id )
		{
            _data = data;
			construct( &_data, sizeof( _data ) );
		}
		pObjectNew( unsigned char* stream, unsigned int len ) : pObjectNewDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pObjectNew( const pObjectNew& );
		pObjectNew& operator=( const pObjectNew& );
};

class pItemNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		_pItemNew _data;

		pItemNew(pDefault* _data_pObjectNew, unsigned int count, double condition, bool equipped) : pObjectNewDefault( ID_ITEM_NEW, PacketFactory::ExtractRefID(_data_pObjectNew), PacketFactory::ExtractBaseID(_data_pObjectNew), PacketFactory::ExtractNetworkID(_data_pObjectNew))
		{
            memcpy(&this->_data._data_pObjectNew, PacketFactory::ExtractRawData(_data_pObjectNew), sizeof(this->_data._data_pObjectNew));
			_data.count = count;
			_data.condition = condition;
			_data.equipped = equipped;
			construct( &_data, sizeof( _data ) );
		}
		pItemNew( unsigned char* stream, unsigned int len ) : pObjectNewDefault( stream, len )
		{
			deconstruct( &_data, sizeof( _data ) );
		}

		pItemNew( const pItemNew& );
		pItemNew& operator=( const pItemNew& );
};

/*
class pContainerNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		_pContainerNew _data;

		pContainerNew(pDefault* _data_pObjectNew) : pObjectNewDefault( ID_CONTAINER_NEW, PacketFactory::ExtractRefID(_data_pObjectNew), PacketFactory::ExtractBaseID(_data_pObjectNew), PacketFactory::ExtractNetworkID(_data_pObjectNew))
		{

			construct( &_data, sizeof( _data ) );
		}
		pContainerNew( unsigned char* stream, unsigned int len ) : pObjectNewDefault( stream, len )
		{

			deconstruct( &_data, sizeof( _data ) );
		}

		pContainerNew( const pContainerNew& );
		pContainerNew& operator=( const pContainerNew& );
};
*/

class pPlayerNew : public pObjectNewDefault
{
		friend class PacketFactory;

	private:
		char name[MAX_PLAYER_NAME];

		pPlayerNew( NetworkID id, unsigned int refID, unsigned int baseID, char* name ) : pObjectNewDefault( ID_PLAYER_NEW, refID, baseID, id )
		{
			ZeroMemory( this->name, sizeof( this->name ) );
			strncpy( this->name, name, sizeof( this->name ) );
			construct( name, sizeof( this->name ) );
		}
		pPlayerNew( unsigned char* stream, unsigned int len ) : pObjectNewDefault( stream, len )
		{
			deconstruct( name, sizeof( this->name ) );
		}

		pPlayerNew( const pPlayerNew& );
		pPlayerNew& operator=( const pPlayerNew& );
};

class pPlayerLeft : public pObjectDefault
{
		friend class PacketFactory;

	private:
		pPlayerLeft( NetworkID id ) : pObjectDefault( ID_PLAYER_LEFT, id )
		{
			construct();
		}
		pPlayerLeft( unsigned char* stream, unsigned int len ) : pObjectDefault( stream, len )
		{
			deconstruct();
		}

		pPlayerLeft( const pPlayerLeft& );
		pPlayerLeft& operator=( const pPlayerLeft& );
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

#endif
