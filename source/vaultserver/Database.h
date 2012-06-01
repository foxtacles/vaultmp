#ifndef DATABASE_H
#define DATABASE_H

#ifdef __WIN32__
#include <winsock2.h>
#endif

#include <unordered_map>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Debug.h"

#include "sqlite/sqlite3.h"

using namespace std;

/**
 * \brief Used to access vaultmp SQLite databases
 */

class Database
{
		friend class GameFactory;

	private:
#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		struct Record {
			string name;
			unsigned char dlc;

			Record(string name, unsigned char dlc) : name(name), dlc(dlc) {}
		};

		sqlite3* db;
		unordered_map<unsigned int, Record> data;

		Database(string file, string table);
		Database(const Database&) = delete;
		Database& operator=(const Database&) = delete;
		~Database();

		pair<string, unsigned char> Lookup(unsigned int baseID);

	public:

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

};

#endif
