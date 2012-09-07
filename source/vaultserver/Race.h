#ifndef RACE_H
#define RACE_H

#include <unordered_map>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Database.h"
#include "Record.h"

#include "sqlite/sqlite3.h"

using namespace std;

/**
 * \brief Represents a race
 */

class Race
{
	private:
		static unordered_map<unsigned int, const Race*> races;

		unsigned int baseID;
		bool child;
		unsigned int younger;
		unsigned int older;

		Race(const Race&) = delete;
		Race& operator=(const Race& p) = delete;

	public:
		static const Race& Lookup(unsigned int baseID);

		unsigned int GetBase() const;
		bool IsChild() const;
		unsigned int GetYounger() const;
		unsigned int GetOlder() const;

		Race(const string& table, sqlite3_stmt* stmt);
		// must never be called. only defined because vector requires it
		Race(Race&&) { terminate(); }
		Race& operator=(Race&&) = delete;
		~Race();
};

#endif
