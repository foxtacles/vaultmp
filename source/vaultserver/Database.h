#ifndef DATABASE_H
#define DATABASE_H

#ifdef __WIN32__
	#include <winsock2.h>
	#include <io.h>
#else
	#include <climits>
	#include <unistd.h>
#endif

#include <list>
#include <vector>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Debug.h"
#include "Utils.h"
#include "VaultException.h"

#include "sqlite/sqlite3.h"

/**
 * \brief Used to access vaultmp SQLite3 databases
 */

template<typename T>
class Database
{
		friend class GameFactory;

	private:
#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		std::vector<T> data;

		Database() = default;
		~Database() = default;

		unsigned int initialize(const std::string& file, const std::vector<std::string>& tables);

	public:
#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

};

#endif
