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
#define GetScriptCallback(a,b) (b = (decltype(b)) GetProcAddress((HINSTANCE)this->handle,a))
#define SetScriptFunction(a,b) *((unsigned int*)(GetProcAddress((HINSTANCE)this->handle,a)?GetProcAddress((HINSTANCE)this->handle,a):throw VaultException("Script function pointer not found: %s", a)))=(unsigned int)b;
#else
#define GetScriptCallback(a,b) (b = (decltype(b)) dlsym(this->handle,a))
#define SetScriptFunction(a,b) *((unsigned int*)(dlsym(this->handle,a)?dlsym(this->handle,a):throw VaultException("Script function pointer not found: %s", a)))=(unsigned int)b;
#endif

using namespace std;
using namespace Values;

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

        typedef void ( *fexec )(); fexec exec;
        typedef void ( *fOnSpawn )( NetworkID ); fOnSpawn _OnSpawn;
        typedef void ( *fOnCellChange )( NetworkID, unsigned int ); fOnCellChange _OnCellChange;
        typedef void ( *fOnContainerItemChange )( NetworkID, unsigned int, signed int, double ); fOnContainerItemChange _OnContainerItemChange;
        typedef void ( *fOnActorValueChange )( NetworkID, unsigned char, double ); fOnActorValueChange _OnActorValueChange;
        typedef void ( *fOnActorBaseValueChange )( NetworkID, unsigned char, double ); fOnActorBaseValueChange _OnActorBaseValueChange;
        typedef void ( *fOnActorAlert )( NetworkID, bool ); fOnActorAlert _OnActorAlert;
        typedef void ( *fOnActorSneak )( NetworkID, bool ); fOnActorSneak _OnActorSneak;
        typedef void ( *fOnActorDeath )( NetworkID ); fOnActorDeath _OnActorDeath;
        typedef void ( *fOnActorEquipItem )( NetworkID, unsigned int, double ); fOnActorEquipItem _OnActorEquipItem;
        typedef void ( *fOnActorUnequipItem )( NetworkID, unsigned int, double ); fOnActorUnequipItem _OnActorUnequipItem;
        typedef void ( *fOnPlayerDisconnect )( NetworkID, unsigned char ); fOnPlayerDisconnect _OnPlayerDisconnect;
        typedef unsigned int ( *fOnPlayerRequestGame )( NetworkID ); fOnPlayerRequestGame _OnPlayerRequestGame;
        typedef bool ( *fOnClientAuthenticate )( string, string ); fOnClientAuthenticate _OnClientAuthenticate;

		Script( const Script& );
		Script& operator=( const Script& );

	public:

		static void LoadScripts( char* scripts, char* base );
		static void UnloadScripts();

		static NetworkID CreateTimer( ScriptFunc timer, unsigned int interval );
		static NetworkID CreateTimerEx( ScriptFunc timer, unsigned int interval, string def, ... );
		static NetworkID CreateTimerPAWN( ScriptFuncPAWN timer, AMX* amx, unsigned int interval );
		static NetworkID CreateTimerPAWNEx( ScriptFuncPAWN timer, AMX* amx, unsigned int interval, string def, const vector<boost::any>& args );
		static void KillTimer( NetworkID id = 0 );
		static void MakePublic( ScriptFunc _public, string name, string def );
		static void MakePublicPAWN( ScriptFuncPAWN _public, AMX* amx, string name, string def );
		static unsigned long long CallPublic( string name, ... );
		static unsigned long long CallPublicPAWN( string name, const vector<boost::any>& args );

        static unsigned long long Timer_Respawn(NetworkID id);

        static void OnSpawn( FactoryObject reference );
		static void OnCellChange( FactoryObject reference, unsigned int cell );
		static void OnContainerItemChange( FactoryObject reference, unsigned int baseID, signed int count, double condition );
		static void OnActorValueChange( FactoryObject reference, unsigned char index, bool base, double value );
		static void OnActorAlert( FactoryObject reference, bool alerted );
		static void OnActorSneak( FactoryObject reference, bool sneaking );
		static void OnActorDeath( FactoryObject reference );
		static void OnActorEquipItem( FactoryObject reference, unsigned int baseID, double condition );
		static void OnActorUnequipItem( FactoryObject reference, unsigned int baseID, double condition );
		static void OnPlayerDisconnect( FactoryObject reference, unsigned char reason );
		static unsigned int OnPlayerRequestGame( FactoryObject reference );
		static bool OnClientAuthenticate( string name, string pwd );

        static bool UIMessage(NetworkID id, string message);
        static void SetRespawn(unsigned int respawn);
		static bool IsValid( NetworkID id );
		static bool IsObject( NetworkID id );
		static bool IsItem( NetworkID id );
		static bool IsContainer( NetworkID id );
		static bool IsActor( NetworkID id );
		static bool IsPlayer( NetworkID id );

		static unsigned int GetReference( NetworkID id );
		static unsigned int GetBase( NetworkID id );
		static string GetName( NetworkID id );
		static void GetPos( NetworkID id, double& X, double& Y, double& Z );
		static void GetAngle( NetworkID id, double& X, double& Y, double& Z );
		static unsigned int GetCell( NetworkID id );
		static unsigned int GetContainerItemCount( NetworkID id, unsigned int baseID );
		static double GetActorValue( NetworkID id, unsigned char index );
		static double GetActorBaseValue( NetworkID id, unsigned char index );
		static unsigned char GetActorMovingAnimation( NetworkID id );
		static bool GetActorAlerted( NetworkID id );
		static bool GetActorSneaking( NetworkID id );
		static bool GetActorDead( NetworkID id );
		static bool IsActorJumping( NetworkID id );

        static void AddItem( NetworkID id, unsigned int baseID, unsigned int count, double condition );
        static unsigned int RemoveItem( NetworkID id, unsigned int baseID, unsigned int count );
        static void RemoveAllItems( NetworkID id );
		static void SetActorValue( NetworkID id, unsigned char index, double value );
		static void SetActorBaseValue( NetworkID id, unsigned char index, double value );

		static void SetPlayerRespawn( NetworkID id, unsigned int respawn );

};

#endif
