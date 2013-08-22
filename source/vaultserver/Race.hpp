#ifndef RACEDB_H
#define RACEDB_H

#include "vaultserver.hpp"
#include "Expected.hpp"

#include <unordered_map>

class sqlite3_stmt;

/**
 * \brief Represents a race
 */

namespace DB
{
	class Race
	{
		private:
			static std::unordered_map<unsigned int, Race*> races;

			unsigned int baseID;
			bool child;
			unsigned int younger;
			unsigned int older;

			Race(const Race&) = delete;
			Race& operator=(const Race&) = delete;

		public:
			static Expected<Race*> Lookup(unsigned int baseID);

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
}

#endif
