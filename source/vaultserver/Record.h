#ifndef RECORD_H
#define RECORD_H

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Database.h"

#include "sqlite/sqlite3.h"

using namespace std;

/**
 * \brief Represents a game record
 */

class Record
{
		friend class Database<Record>;

	private:
		static unordered_map<unsigned int, const Record*> data;

		unsigned int baseID;
		string name;
		string description;
		string type;

		Record(const string& table, sqlite3_stmt* stmt);

	public:
		static const Record& Lookup(unsigned int baseID);
		static const Record& GetRecordNotIn(const unordered_set<unsigned int>& _set);

		unsigned int GetBase() const;
		const string& GetName() const;
		const string& GetDescription() const;
		const string& GetType() const;

		~Record();
};

#endif
