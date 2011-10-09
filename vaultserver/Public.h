#ifndef PUBLIC_H
#define PUBLIC_H

#include <map>
#include <string>

#include "boost/any.hpp"

#include "ScriptFunction.h"
#include "Script.h"
#include "PAWN.h"
#include "../vaultmp.h"
#include "../Debug.h"
#include "../Network.h"

using namespace std;
using namespace RakNet;

/**
 * \brief Create publics to use across all scripts
 */

class Public : public ScriptFunction
{
	private:
		~Public();

		static map<string, Public*> publics;

	public:

		Public( ScriptFunc _public, string name, string def );
		Public( ScriptFuncPAWN _public, AMX* amx, string name, string def );

		/**
		 * \brief Calls a public
		 */
		static unsigned long long Call( string name, const vector<boost::any>& args );
		/**
		 * \brief Retrieves the definition of a public
		 */
		static string GetDefinition( string name );
		/**
		 * \brief Deletes all publics
		 */
		static void DeleteAll();
};

#endif
