#ifndef RACE_H
#define RACE_H

#include <unordered_map>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Database.h"
#include "Record.h"
#include "Expected.h"

#include "sqlite/sqlite3.h"

/**
 * \brief Represents a race
 */

class Race
{
	private:
		static std::unordered_map<unsigned int, const Race*> races;

		unsigned int baseID;
		bool child;
		unsigned int younger;
		unsigned int older;

		Race(const Race&) = delete;
		Race& operator=(const Race& p) = delete;

	public:
		static Expected<const Race*> Lookup(unsigned int baseID);

		unsigned int GetBase() const;
		bool IsChild() const;
		unsigned int GetYounger() const;
		unsigned int GetOlder() const;
		unsigned int GetAge() const;
		unsigned int GetMaxAge() const;
		signed int GetAgeDifference(unsigned int race) const;

		Race(const std::string& table, sqlite3_stmt* stmt);
		~Race() = default;
		// must never be called. only defined because vector requires it
		Race(Race&&) { std::terminate(); }
		Race& operator=(Race&&) = delete;
};

#endif
