#ifndef DATABASE_H
#define DATABASE_H

#ifdef __WIN32__
#include <winsock2.h>
#include <io.h>
#else
#include <climits>
#include <unistd.h>
#endif

#include <vector>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Debug.h"
#include "VaultException.h"

#include "sqlite/sqlite3.h"

using namespace std;

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

		vector<T> data;

		Database();
		~Database();

		unsigned int initialize(const string& file, const vector<string>& tables);
		void clear();

	public:
#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

};

#endif
