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
	private:
		static unordered_map<unsigned int, const Record*> data;

		unsigned int baseID;
		string name;
		string description;
		string type;

		Record(const Record&) = delete;
		Record& operator=(const Record& p) = delete;

	public:
		static const Record& Lookup(unsigned int baseID);
		static const Record& Lookup(unsigned int baseID, const string& type);
		static const Record& Lookup(unsigned int baseID, const vector<string>& type);
		static const Record& GetRecordNotIn(const unordered_set<unsigned int>& _set, const function<bool(const Record&)>& pred);

		static bool IsValidCell(unsigned int baseID);
		static bool IsValidWeather(unsigned int baseID);

		unsigned int GetBase() const;
		const string& GetName() const;
		const string& GetDescription() const;
		const string& GetType() const;

		Record(const string& table, sqlite3_stmt* stmt);
		// must never be called. only defined because vector requires it
		Record(Record&&) { terminate(); }
		Record& operator=(Record&&) = delete;
		~Record();
};

#endif
