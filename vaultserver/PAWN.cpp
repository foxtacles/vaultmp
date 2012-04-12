#include "PAWN.h"

/* AMX extension modules */
#define FLOATPOINT // floating point for console
#define AMXCONSOLE_NOIDLE // no idle state
#define AMXTIME_NOIDLE // no idle state
#define NDEBUG // no debug
#include "amx/amxcore.c"
#undef __T
#include "amx/amxfile.c"
#undef __T
#include "amx/amxcons.c"
#undef __T
#include "amx/amxstring.c"
#include "amx/amxtime.c"
#include "amx/float.c"

#include "Script.h"

AMX_NATIVE_INFO PAWN::vaultmp_functions[] =
{
	{"timestamp", PAWN::vaultmp_timestamp},
	{"CreateTimer", PAWN::vaultmp_CreateTimer},
	{"CreateTimerEx", PAWN::vaultmp_CreateTimerEx},
	{"KillTimer", PAWN::vaultmp_KillTimer},
	{"MakePublic", PAWN::vaultmp_MakePublic},
	{"CallPublic", PAWN::vaultmp_CallPublic},

	{"SetServerName", PAWN::vaultmp_SetServerName},
	{"SetServerMap", PAWN::vaultmp_SetServerMap},
	{"SetServerRule", PAWN::vaultmp_SetServerRule},
	{"GetGameCode", PAWN::vaultmp_GetGameCode},

	{"ValueToString", PAWN::vaultmp_ValueToString},
	{"AxisToString", PAWN::vaultmp_AxisToString},
	{"AnimToString", PAWN::vaultmp_AnimToString},

	{"GetReference", PAWN::vaultmp_GetReference},
	{"GetBase", PAWN::vaultmp_GetBase},
	{"GetName", PAWN::vaultmp_GetName},
	{"GetPos", PAWN::vaultmp_GetPos},
	{"GetAngle", PAWN::vaultmp_GetAngle},
	{"GetCell", PAWN::vaultmp_GetCell},
	{"GetActorValue", PAWN::vaultmp_GetActorValue},
	{"GetActorBaseValue", PAWN::vaultmp_GetActorBaseValue},
	{"GetActorMovingAnimation", PAWN::vaultmp_GetActorMovingAnimation},
	{"GetActorAlerted", PAWN::vaultmp_GetActorAlerted},
	{"GetActorSneaking", PAWN::vaultmp_GetActorSneaking},
	{"GetActorDead", PAWN::vaultmp_GetActorDead},
	{"IsActorJumping", PAWN::vaultmp_IsActorJumping},

    {"AddItem", PAWN::vaultmp_AddItem},
    {"RemoveItem", PAWN::vaultmp_RemoveItem},
    {"SetActorValue", PAWN::vaultmp_SetActorValue},
    {"SetActorBaseValue", PAWN::vaultmp_SetActorBaseValue},

	{0, 0}
};

int PAWN::RegisterVaultmpFunctions( AMX* amx )
{
	return PAWN::Register( amx, PAWN::vaultmp_functions, -1 );
}

cell PAWN::vaultmp_timestamp( AMX* amx, const cell* params )
{
	cell i = 1;
	Utils::timestamp();
	return i;
}

cell PAWN::vaultmp_CreateTimer( AMX* amx, const cell* params )
{
	cell i = 1, interval;
	int len;
	cell* source;

	source = amx_Address( amx, params[1] );
	amx_StrLen( source, &len );
	char name[len + 1];

	amx_GetString( name, source, 0, UNLIMITED );

	interval = params[2];

	i = ( cell ) Script::CreateTimerPAWN( string( name ), amx, ( unsigned int ) interval );

	return i;
}

cell PAWN::vaultmp_CreateTimerEx( AMX* amx, const cell* params )
{
	cell i = 1, interval;
	int len;
	cell* source;

	source = amx_Address( amx, params[1] );
	amx_StrLen( source, &len );
	char name[len + 1];

	amx_GetString( name, source, 0, UNLIMITED );

	interval = params[2];

	source = amx_Address( amx, params[3] );
	amx_StrLen( source, &len );
	char def[len + 1];

	amx_GetString( def, source, 0, UNLIMITED );

	vector<boost::any> args;
	unsigned int count = ( params[0] / sizeof( cell ) ) - 3;

	if ( count != len )
		throw VaultException( "Script call: Number of arguments does not match definition" );

	for ( int i = 0; i < count; ++i )
	{
		cell* data = amx_Address( amx, params[i + 4] );

		switch ( def[i] )
		{
			case 'i':
				{
					args.push_back( ( unsigned int ) *data );
					break;
				}

			case 'l':
				{
					args.push_back( ( unsigned long long ) *data );
					break;
				}

			case 'f':
				{
					args.push_back( ( double ) amx_ctof( *data ) );
					break;
				}

			case 's':
				{
					amx_StrLen( data, &len );
					char str[len + 1];
					amx_GetString( str, data, 0, UNLIMITED );
					args.push_back( string( str ) );
					break;
				}

			default:
				throw VaultException( "PAWN call: Unknown argument identifier %02X", def[i] );
		}
	}

	i = ( cell ) Script::CreateTimerPAWNEx( string( name ), amx, ( unsigned int ) interval, string( def ), args );

	return i;
}

cell PAWN::vaultmp_KillTimer( AMX* amx, const cell* params )
{
	cell i = 1, id;

	id = params[1];

	Script::KillTimer( ( NetworkID ) id );

	return i;
}

cell PAWN::vaultmp_MakePublic( AMX* amx, const cell* params )
{
	cell i = 1;
	int len;
	cell* source;

	source = amx_Address( amx, params[1] );
	amx_StrLen( source, &len );
	char real[len + 1];

	amx_GetString( real, source, 0, UNLIMITED );

	source = amx_Address( amx, params[2] );
	amx_StrLen( source, &len );
	char name[len + 1];

	amx_GetString( name, source, 0, UNLIMITED );

	source = amx_Address( amx, params[3] );
	amx_StrLen( source, &len );
	char def[len + 1];

	amx_GetString( def, source, 0, UNLIMITED );

	Script::MakePublicPAWN( string( real ), amx, string( name ), string( def ) );

	return i;
}

cell PAWN::vaultmp_CallPublic( AMX* amx, const cell* params )
{
	cell i = 1;
	int len;
	cell* source;

	source = amx_Address( amx, params[1] );
	amx_StrLen( source, &len );
	char name[len + 1];

	amx_GetString( name, source, 0, UNLIMITED );

	string def = Public::GetDefinition( string( name ) );

	vector<boost::any> args;
	unsigned int count = ( params[0] / sizeof( cell ) ) - 1;

	if ( count != def.length() )
		throw VaultException( "Script call: Number of arguments does not match definition" );

	for ( int i = 0; i < count; ++i )
	{
		cell* data = amx_Address( amx, params[i + 2] );

		switch ( def[i] )
		{
			case 'i':
				{
					args.push_back( ( unsigned int ) *data );
					break;
				}

			case 'l':
				{
					args.push_back( ( unsigned long long ) *data );
					break;
				}

			case 'f':
				{
					args.push_back( ( double ) amx_ctof( *data ) );
					break;
				}

			case 'p':
				{
					args.push_back( ( void* ) data );
					break;
				}

			case 's':
				{
					amx_StrLen( data, &len );
					char str[len + 1];
					amx_GetString( str, data, 0, UNLIMITED );
					args.push_back( string( str ) );
					break;
				}

			default:
				throw VaultException( "PAWN call: Unknown argument identifier %02X", def[i] );
		}
	}

	i = ( cell ) Script::CallPublicPAWN( string( name ), args );

	return i;
}

cell PAWN::vaultmp_SetServerName( AMX* amx, const cell* params )
{
	cell i = 1;
	int len;
	cell* source;

	source = amx_Address( amx, params[1] );
	amx_StrLen( source, &len );
	char name[len + 1];

	amx_GetString( name, source, 0, UNLIMITED );

	Dedicated::SetServerName( string( name ) );

	return i;
}

cell PAWN::vaultmp_SetServerMap( AMX* amx, const cell* params )
{
	cell i = 1;
	int len;
	cell* source;

	source = amx_Address( amx, params[1] );
	amx_StrLen( source, &len );
	char map[len + 1];

	amx_GetString( map, source, 0, UNLIMITED );

	Dedicated::SetServerMap( string( map ) );

	return i;
}

cell PAWN::vaultmp_SetServerRule( AMX* amx, const cell* params )
{
	cell i = 1;
	int len, len2;
	cell* source;
	cell* source2;

	source = amx_Address( amx, params[1] );
	source2 = amx_Address( amx, params[2] );
	amx_StrLen( source, &len );
	amx_StrLen( source2, &len2 );

	char rule[len + 1];
	char value[len2 + 1];

	amx_GetString( rule, source, 0, UNLIMITED );
	amx_GetString( value, source2, 0, UNLIMITED );

	Dedicated::SetServerRule( string( rule ), string( value ) );

	return i;
}

cell PAWN::vaultmp_GetGameCode( AMX* amx, const cell* params )
{
	return ( cell ) Dedicated::GetGameCode();
}

cell PAWN::vaultmp_ValueToString( AMX* amx, const cell* params )
{
	cell i = 1, index;
	cell* dest;

	index = params[1];
	dest = amx_Address( amx, params[2] );

	string value = API::RetrieveValue_Reverse( ( unsigned char ) index );

	if ( !value.empty() )
	{
		amx_SetString( dest, value.c_str(), 1, 0, value.length() + 1 );
	}

	else
		i = 0;

	return i;
}

cell PAWN::vaultmp_AxisToString( AMX* amx, const cell* params )
{
	cell i = 1, index;
	cell* dest;

	index = params[1];
	dest = amx_Address( amx, params[2] );

	string axis = API::RetrieveAxis_Reverse( ( unsigned char ) index );

	if ( !axis.empty() )
		amx_SetString( dest, axis.c_str(), 1, 0, axis.length() + 1 );

	else
		i = 0;

	return i;
}

cell PAWN::vaultmp_AnimToString( AMX* amx, const cell* params )
{
	cell i = 1, index;
	cell* dest;

	index = params[1];
	dest = amx_Address( amx, params[2] );

	string anim = API::RetrieveAnim_Reverse( ( unsigned char ) index );

	if ( !anim.empty() )
		amx_SetString( dest, anim.c_str(), 1, 0, anim.length() + 1 );

	else
		i = 0;

	return i;
}

cell PAWN::vaultmp_GetReference( AMX* amx, const cell* params )
{
	cell i = 1, id;

	id = params[1];

	unsigned int value = Script::GetReference( id );
	i = ( cell ) value;

	return i;
}

cell PAWN::vaultmp_GetBase( AMX* amx, const cell* params )
{
	cell i = 1, id;

	id = params[1];

	unsigned int value = Script::GetBase( id );
	i = ( cell ) value;

	return i;
}

cell PAWN::vaultmp_GetName( AMX* amx, const cell* params )
{
	cell i = 1, id;
	cell* dest;

	id = params[1];
	dest = amx_Address( amx, params[2] );

	string name = Script::GetName( id );

	if ( !name.empty() )
	{
		amx_SetString( dest, name.c_str(), 1, 0, name.length() + 1 );
	}

	else
		i = 0;

	return i;
}

cell PAWN::vaultmp_GetPos( AMX* amx, const cell* params )
{
	cell i = 1, id;
	cell* X;
	cell* Y;
	cell* Z;

	id = params[1];
	X = amx_Address( amx, params[2] );
	Y = amx_Address( amx, params[3] );
	Z = amx_Address( amx, params[4] );

	double dX, dY, dZ;
	Script::GetPos( id, dX, dY, dZ );
	*X = amx_ftoc( dX );
	*Y = amx_ftoc( dY );
	*Z = amx_ftoc( dZ );

	return i;
}

cell PAWN::vaultmp_GetAngle( AMX* amx, const cell* params )
{
	cell i = 1, id;
	cell* X;
	cell* Y;
	cell* Z;

	id = params[1];
	X = amx_Address( amx, params[2] );
	Y = amx_Address( amx, params[3] );
	Z = amx_Address( amx, params[4] );

	double dX, dY, dZ;
	Script::GetAngle( id, dX, dY, dZ );
	*X = amx_ftoc( dX );
	*Y = amx_ftoc( dY );
	*Z = amx_ftoc( dZ );

	return i;
}

cell PAWN::vaultmp_GetCell( AMX* amx, const cell* params )
{
	cell i = 1, id;

	id = params[1];

	unsigned int value = Script::GetCell( id );
	i = ( cell ) value;

	return i;
}

cell PAWN::vaultmp_GetActorValue( AMX* amx, const cell* params )
{
	cell i = 1, id, index;

	id = params[1];
	index = params[2];

	double value = Script::GetActorValue( id, ( unsigned char ) index );
	i = amx_ftoc( value );

	return i;
}

cell PAWN::vaultmp_GetActorBaseValue( AMX* amx, const cell* params )
{
	cell i = 1, id, index;

	id = params[1];
	index = params[2];

	double value = Script::GetActorBaseValue( id, ( unsigned char ) index );
	i = amx_ftoc( value );

	return i;
}

cell PAWN::vaultmp_GetActorMovingAnimation( AMX* amx, const cell* params )
{
	cell i = 1, id;

	id = params[1];

	unsigned char index = Script::GetActorMovingAnimation( id );
	i = ( cell ) index;

	return i;
}

cell PAWN::vaultmp_GetActorAlerted( AMX* amx, const cell* params )
{
	cell i = 1, id;

	id = params[1];

	bool state = Script::GetActorAlerted( id );
	i = ( cell ) state;

	return i;
}

cell PAWN::vaultmp_GetActorSneaking( AMX* amx, const cell* params )
{
	cell i = 1, id;

	id = params[1];

	bool state = Script::GetActorSneaking( id );
	i = ( cell ) state;

	return i;
}

cell PAWN::vaultmp_GetActorDead( AMX* amx, const cell* params )
{
	cell i = 1, id;

	id = params[1];

	bool state = Script::GetActorDead( id );
	i = ( cell ) state;

	return i;
}

cell PAWN::vaultmp_IsActorJumping( AMX* amx, const cell* params )
{
	cell i = 1, id;

	id = params[1];

	bool state = Script::IsActorJumping( id );
	i = ( cell ) state;

	return i;
}

cell PAWN::vaultmp_AddItem( AMX* amx, const cell* params )
{
	cell i = 1, id, base, count;
	double value;

	id = params[1];
	base = params[2];
	count = params[3];
	value = amx_ctof(params[4]);

    Script::AddItem(id, base, count, value);

	return i;
}

cell PAWN::vaultmp_RemoveItem( AMX* amx, const cell* params )
{
	cell i = 1, id, base, count;

	id = params[1];
	base = params[2];
	count = params[3];

    i = (cell) Script::RemoveItem(id, base, count);

	return i;
}

cell PAWN::vaultmp_SetActorValue( AMX* amx, const cell* params )
{
	cell i = 1, id, index;
	double value;

	id = params[1];
	index = params[2];
	value = amx_ctof(params[3]);

    Script::SetActorValue(id, index, value);

	return i;
}

cell PAWN::vaultmp_SetActorBaseValue( AMX* amx, const cell* params )
{
	cell i = 1, id, index;
	double value;

	id = params[1];
	index = params[2];
	value = amx_ctof(params[3]);

    Script::SetActorBaseValue(id, index, value);

	return i;
}

int PAWN::LoadProgram( AMX* amx, char* filename, void* memblock )
{
	return aux_LoadProgram( amx, filename, memblock );
}

int PAWN::Register( AMX* amx, const AMX_NATIVE_INFO* list, int number )
{
	return amx_Register( amx, list, number );
}

int PAWN::Exec( AMX* amx, cell* retval, int index )
{
	return amx_Exec( amx, retval, index );
}

int PAWN::FreeProgram( AMX* amx )
{
	return aux_FreeProgram( amx );
}

bool PAWN::IsCallbackPresent(AMX* amx, const char* name)
{
    int idx = 0;
    int err = 0;
    err = amx_FindPublic( amx, name, &idx );
    return (err == AMX_ERR_NONE);
}

cell PAWN::Call( AMX* amx, const char* name, const char* argl, int buf, ... )
{
	va_list args;
	va_start( args, buf );
	cell ret = 0;
	vector<pair<cell*, char*> > strings;

	try
	{
		int idx = 0;
		int err = 0;

		err = amx_FindPublic( amx, name, &idx );

		if ( err != AMX_ERR_NONE )
			throw VaultException( "PAWN runtime error (%d): \"%s\"", err, aux_StrError( err ) );

		for ( int i = 0; i < strlen( argl ); i++ )
		{
			switch ( argl[i] )
			{
				case 'i':
					{
						cell value = ( cell ) va_arg( args, unsigned int );
						amx_Push( amx, value );
						break;
					}

				case 'l':
					{
						cell value = ( cell ) va_arg( args, unsigned long long );
						amx_Push( amx, value );
						break;
					}

				case 'f':
					{
						double value = va_arg( args, double );
						amx_Push( amx, amx_ftoc( value ) );
						break;
					}

				case 'p':
					{
						cell value = ( cell ) va_arg( args, void* );
						amx_Push( amx, value );
						break;
					}

				case 's':
					{
						char* string = va_arg( args, char* );
						cell* store;
						amx_PushString( amx, &store, string, 1, 0 );
						strings.push_back( pair<cell*, char*>( store, string ) );
						break;
					}

				default:
					throw VaultException( "PAWN call: Unknown argument identifier %02X", argl[i] );
			}
		}

		err = amx_Exec( amx, &ret, idx );

		if ( err != AMX_ERR_NONE )
			throw VaultException( "PAWN runtime error (%d): \"%s\"", err, aux_StrError( err ) );

		if ( buf != 0 )
		{
			for ( vector<pair<cell*, char*> >::iterator it = strings.begin(); it != strings.end(); ++it )
			{
				int length;
				amx_StrLen( it->first, &length );

				if ( buf >= length )
				{
					ZeroMemory( it->second, buf );
					amx_GetString( it->second, it->first, 0, UNLIMITED );
				}
			}
		}

		if ( !strings.empty() )
			amx_Release( amx, strings.at( 0 ).first );
	}

	catch ( ... )
	{
		va_end( args );

		if ( !strings.empty() )
			amx_Release( amx, strings.at( 0 ).first );

		throw;
	}

	return ret;
}

cell PAWN::Call( AMX* amx, const char* name, const char* argl, const vector<boost::any>& args )
{
	cell ret = 0;
	cell* str = NULL;

	try
	{
		int idx = 0;
		int err = 0;

		err = amx_FindPublic( amx, name, &idx );

		if ( err != AMX_ERR_NONE )
			throw VaultException( "PAWN runtime error (%d): \"%s\"", err, aux_StrError( err ) );

		for ( int i = strlen( argl ) - 1; i >= 0; i-- )
		{
			switch ( argl[i] )
			{
				case 'i':
					{
						cell value = ( cell ) boost::any_cast<unsigned int>( args.at( i ) );
						amx_Push( amx, value );
						break;
					}

				case 'l':
					{
						cell value = ( cell ) boost::any_cast<unsigned long long>( args.at( i ) );
						amx_Push( amx, value );
						break;
					}

				case 'f':
					{
						double value = boost::any_cast<double>( args.at( i ) );
						amx_Push( amx, amx_ftoc( value ) );
						break;
					}

				case 'p':
					{
						cell value = ( cell ) boost::any_cast<void*>( args.at( i ) );
						amx_Push( amx, value );
						break;
					}

				case 's':
					{
						string _string = boost::any_cast<string>( args.at( i ) );
						const char* string = _string.c_str();
						cell* store;
						amx_PushString( amx, &store, string, 1, 0 );

						if ( !str )
							str = store;

						break;
					}

				default:
					throw VaultException( "PAWN call: Unknown argument identifier %02X", argl[i] );
			}
		}

		err = amx_Exec( amx, &ret, idx );

		if ( err != AMX_ERR_NONE )
			throw VaultException( "PAWN runtime error (%d): \"%s\"", err, aux_StrError( err ) );

		if ( str )
			amx_Release( amx, str );
	}

	catch ( ... )
	{
		if ( str )
			amx_Release( amx, str );

		throw;
	}

	return ret;
}

int PAWN::CoreInit( AMX* amx )
{
	return amx_CoreInit( amx );
}

int PAWN::ConsoleInit( AMX* amx )
{
	return amx_ConsoleInit( amx );
}

int PAWN::FloatInit( AMX* amx )
{
	return amx_FloatInit( amx );
}

int PAWN::TimeInit( AMX* amx )
{
	return amx_TimeInit( amx );
}

int PAWN::StringInit( AMX* amx )
{
	return amx_StringInit( amx );
}

int PAWN::FileInit( AMX* amx )
{
	return amx_FileInit( amx );
}
