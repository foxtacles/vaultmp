#ifndef PUBLIC_H
#define PUBLIC_H

#include "vaultserver.hpp"
#include "ScriptFunction.hpp"

#include <unordered_map>

/**
 * \brief Create publics to use across all scripts
 */

class Public : public ScriptFunction
{
	private:
		~Public();

		static std::unordered_map<std::string, Public*> publics;

		Public(ScriptFunc _public, const std::string& name, const std::string& def);
		Public(ScriptFuncPAWN _public, AMX* amx, const std::string& name, const std::string& def);

	public:
		template<typename... Args>
		static void MakePublic(Args&&... args) { new Public(std::forward<Args>(args)...); }

		/**
		 * \brief Calls a public
		 */
		static unsigned long long Call(const std::string& name, const std::vector<boost::any>& args);
		/**
		 * \brief Retrieves the definition of a public
		 */
		static const std::string& GetDefinition(const std::string& name);
		/**
		 * \brief Checks whether public function is a PAWN function
		 */
		static bool IsPAWN(const std::string& name);
		/**
		 * \brief Deletes all publics
		 */
		static void DeleteAll();
};

#endif
