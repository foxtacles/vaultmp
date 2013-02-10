#ifndef PUBLIC_H
#define PUBLIC_H

#include <unordered_map>
#include <string>

#include "boost/any.hpp"

#include "ScriptFunction.h"
#include "Script.h"
#include "PAWN.h"
#include "vaultmp.h"
#include "Debug.h"
#include "Network.h"

/**
 * \brief Create publics to use across all scripts
 */

class Public : public ScriptFunction
{
	private:
		~Public();

		static std::unordered_map<std::string, Public*> publics;

	public:
		Public(ScriptFunc _public, const std::string& name, const std::string& def);
		Public(ScriptFuncPAWN _public, AMX* amx, const std::string& name, const std::string& def);

		/**
		 * \brief Calls a public
		 */
		static unsigned long long Call(const std::string& name, const std::vector<boost::any>& args);
		/**
		 * \brief Retrieves the definition of a public
		 */
		static std::string GetDefinition(const std::string& name);
		/**
		 * \brief Deletes all publics
		 */
		static void DeleteAll();
};

#endif
