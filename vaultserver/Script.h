#ifndef SCRIPT_H
#define SCRIPT_H

#ifdef __WIN32__
#include <winsock2.h>
#else
#include <dlfcn.h>
#endif
#include <vector>
#include <string>

#include "boost/any.hpp"

#include "ScriptFunction.h"
#include "Public.h"
#include "PAWN.h"
#include "Dedicated.h"
#include "../API.h"
#include "../Utils.h"
#include "../vaultmp.h"
#include "../VaultException.h"

#ifdef __WIN32__
#define GetScriptCallback(a,b,c) c = (a) GetProcAddress((HINSTANCE)this->handle,b); \
	if (!c) throw VaultException("Script callback not found: %s", b);
#define SetScriptFunction(a,b) *((unsigned int*)(GetProcAddress((HINSTANCE)this->handle,a)?GetProcAddress((HINSTANCE)this->handle,a):throw VaultException("Script function pointer not found: %s", a)))=(unsigned int)b;
#else
#define GetScriptCallback(a,b,c) c = (a) dlsym(this->handle,b); \
	if (!c) throw VaultException("Script callback not found: %s", b);
#define SetScriptFunction(a,b) *((unsigned int*)(dlsym(this->handle,a)?dlsym(this->handle,a):throw VaultException("Script function pointer not found: %s", a)))=(unsigned int)b;
#endif

using namespace std;
using namespace Values;

typedef void ( *fexec )();
typedef bool ( *fOnClientAuthenticate )( string, string );
typedef void ( *fOnPlayerDisconnect )( NetworkID, unsigned char );
typedef unsigned int ( *fOnPlayerRequestGame )( NetworkID );
typedef void ( *fOnSpawn )( NetworkID );
typedef void ( *fOnCellChange )( NetworkID, unsigned int );
typedef void ( *fOnActorValueChange )( NetworkID, unsigned char, double );
typedef void ( *fOnActorBaseValueChange )( NetworkID, unsigned char, double );
typedef void ( *fOnActorAlert )( NetworkID, bool );
typedef void ( *fOnActorSneak )( NetworkID, bool );
typedef void ( *fOnActorDeath )( NetworkID );

/**
 * \brief Maintains communication with a script
 *
 * A script can be either a C++ or PAWN script
 */

class Script
{
	private:

		Script( char* path );
		~Script();

		static vector<Script*> scripts;

		static void GetArguments( vector<boost::any>& params, va_list args, string def );

		void* handle;
		bool cpp_script;

		fexec exec;
		fOnClientAuthenticate _OnClientAuthenticate;
		fOnPlayerDisconnect _OnPlayerDisconnect;
		fOnPlayerRequestGame _OnPlayerRequestGame;
		fOnSpawn _OnSpawn;
		fOnCellChange _OnCellChange;
		fOnActorValueChange _OnActorValueChange;
		fOnActorBaseValueChange _OnActorBaseValueChange;
		fOnActorAlert _OnActorAlert;
		fOnActorSneak _OnActorSneak;
		fOnActorDeath _OnActorDeath;

		Script( const Script& );
		Script& operator=( const Script& );

	public:

		static void LoadScripts( char* scripts, char* base );
		static void UnloadScripts();

		static NetworkID CreateTimer( ScriptFunc timer, unsigned int interval );
		static NetworkID CreateTimerEx( ScriptFunc timer, unsigned int interval, string def, ... );
		static NetworkID CreateTimerPAWN( ScriptFuncPAWN timer, AMX* amx, unsigned int interval );
		static NetworkID CreateTimerPAWNEx( ScriptFuncPAWN timer, AMX* amx, unsigned int interval, string def, const vector<boost::any>& args );
		static void KillTimer( NetworkID id );
		static void MakePublic( ScriptFunc _public, string name, string def );
		static void MakePublicPAWN( ScriptFuncPAWN _public, AMX* amx, string name, string def );
		static unsigned long long CallPublic( string name, ... );
		static unsigned long long CallPublicPAWN( string name, const vector<boost::any>& args );

		static bool OnClientAuthenticate( string name, string pwd );
		static void OnPlayerDisconnect( FactoryObject reference, unsigned char reason );
		static unsigned int OnPlayerRequestGame( FactoryObject reference );
		static void OnCellChange( FactoryObject reference, unsigned int cell );
		static void OnActorValueChange( FactoryObject reference, unsigned char index, bool base, double value );
		static void OnActorAlert( FactoryObject reference, bool alerted );
		static void OnActorSneak( FactoryObject reference, bool sneaking );

		static unsigned int GetReference( NetworkID id );
		static unsigned int GetBase( NetworkID id );
		static string GetName( NetworkID id );
		static void GetPos( NetworkID id, double& X, double& Y, double& Z );
		static void GetAngle( NetworkID id, double& X, double& Y, double& Z );
		static unsigned int GetCell( NetworkID id );

		static double GetActorValue( NetworkID id, unsigned char index );
		static double GetActorBaseValue( NetworkID id, unsigned char index );
		static unsigned char GetActorMovingAnimation( NetworkID id );
		static bool GetActorAlerted( NetworkID id );
		static bool GetActorSneaking( NetworkID id );
		static bool GetActorDead( NetworkID id );
		static bool IsActorJumping( NetworkID id );

};

#endif
