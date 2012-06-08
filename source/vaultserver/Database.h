#ifndef DATABASE_H
#define DATABASE_H

#ifdef __WIN32__
#include <winsock2.h>
#include <io.h>
#else
#include <limits.h>
#endif

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Debug.h"
#include "VaultException.h"

#include "sqlite/sqlite3.h"

using namespace std;

/**
 * \brief Used to access vaultmp SQLite databases
 */

class Database
{
		friend class GameFactory;

	public:
		struct Record {
			unsigned int baseID;
			string name;

			Record(unsigned int baseID, string name) : baseID(baseID), name(name) {}
		};

	private:
#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		unordered_map<unsigned int, Record> data;

		Database(string file, string table);
		~Database();

	public:
		const Record& Lookup(unsigned int baseID);
		const Record& GetRecordNotIn(const unordered_set<unsigned int>& _set);

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

};

#endif
