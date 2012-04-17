#ifndef DATA_H
#define DATA_H

#include <string>
#include <list>
#include <vector>
#include <map>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakString.h"
#include "RakNet/MessageIdentifiers.h"

#include "VaultException.h"
#include "VaultFunctor.h"

template <template<typename K, typename... Values> class T, typename K, typename... Values>
inline
static typename T<K, Values...>::iterator SAFE_FIND(T<K, Values...>& a, K b)
{
    typename T<K, Values...>::iterator c = a.find(b);

    if (c != a.end())
        return c;

    throw VaultException("Value %02X not defined in database", b);
}

template <template<typename K, typename... Values> class T, typename K, typename... Values>
inline
static typename T<K, Values...>::const_iterator SAFE_FIND(const T<K, Values...>& a, K b)
{
    typename T<K, Values...>::const_iterator c = a.find(b);

    if (c != a.end())
        return c;

    throw VaultException("Value %02X not defined in database", b);
}

static const unsigned int PLAYER_REFERENCE  = 0x00000014;
static const unsigned int PLAYER_BASE       = 0x00000007;

using namespace std;
using namespace RakNet;

/* Shared data structures and tables */

namespace Data
{

	typedef void ( *ResultHandler )( signed int, vector<double>&, double, bool );
	typedef pair<vector<string>, VaultFunctor*> Parameter;
	typedef list<Parameter> ParamContainer;
	typedef const map<const unsigned int, const char*> Database;
	typedef map<const unsigned char, const unsigned char> IndexLookup;

    template <typename R, typename T>
    inline static R storeIn(T t) { R r; *reinterpret_cast<T*>(&r) = t; return r; }
    template <typename R, typename T>
    inline static T getFrom(R r) { return *reinterpret_cast<T*>(&r); }

	static Parameter BuildParameter( string param )
	{
		return Parameter( vector<string> {param}, NULL );
	}

	static Parameter BuildParameter( vector<string> params )
	{
		return Parameter( params, NULL );
	}

	static Parameter BuildParameter( vector<unsigned char> params )
	{
		vector<unsigned char>::iterator it;
		vector<string> convert;

		for ( it = params.begin(); it != params.end(); ++it )
		{
			char value[64];
			snprintf( value, sizeof( value ), "%d", *it );
			convert.push_back( string( value ) );
		}

		return BuildParameter( convert );
	}

	static Parameter BuildParameter( unsigned int param )
	{
		char value[64];
		snprintf( value, sizeof( value ), "%d", param );
		return BuildParameter( string( value ) );
	}

	static Parameter BuildParameter( double param )
	{
		char value[64];
		snprintf( value, sizeof( value ), "%f", param );
		return BuildParameter( string( value ) );
	}

	static const Parameter Param_True = Parameter( vector<string> {"1"}, NULL );
	static const Parameter Param_False = Parameter( vector<string> {"0"}, NULL );

	static void FreeContainer( ParamContainer& param )
	{
		ParamContainer::iterator it;

		for ( it = param.begin(); it != param.end(); ++it )
			if ( it->second )
				delete it->second;
	}

	enum
	{
		ID_MASTER_QUERY = ID_USER_PACKET_ENUM,
		ID_MASTER_ANNOUNCE,
		ID_MASTER_UPDATE,
		ID_GAME_FIRST,
	};

	enum
	{
		CHANNEL_SYSTEM,
		CHANNEL_GAME,
		CHANNEL_CHAT,
	};

}

#endif
