#ifndef RECORDDB_H
#define RECORDDB_H

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Database.h"
#include "Expected.h"

#include "sqlite/sqlite3.h"

/**
 * \brief Represents a game record
 */

namespace DB
{
	class Record
	{
		private:
			static std::unordered_map<unsigned int, Record*> data;

			unsigned int baseID;
			std::string name;
			std::string description;
			std::string type;

			Record(const Record&) = delete;
			Record& operator=(const Record&) = delete;

		public:
			static Expected<Record*> Lookup(unsigned int baseID);
			static Expected<Record*> Lookup(unsigned int baseID, const std::string& type);
			static Expected<Record*> Lookup(unsigned int baseID, const std::vector<std::string>& type);
			static Expected<Record*> GetRecordNotIn(const std::unordered_set<unsigned int>& _set, const std::function<bool(const Record&)>& pred);

			static bool IsValidCell(unsigned int baseID);
			static bool IsValidWeather(unsigned int baseID);

			unsigned int GetBase() const;
			const std::string& GetName() const;
			const std::string& GetDescription() const;
			const std::string& GetType() const;

			void SetDescription(const std::string& description);

			Record(const std::string& table, sqlite3_stmt* stmt);
			~Record() = default;
			// must never be called. only defined because vector requires it
			Record(Record&&) { std::terminate(); }
			Record& operator=(Record&&) = delete;
	};
}

#endif
