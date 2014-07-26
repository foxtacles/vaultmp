#ifndef RECORDDB_H
#define RECORDDB_H

#include "vaultserver.hpp"
#include "Expected.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

class sqlite3_stmt;

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
			static const std::unordered_map<unsigned int, Record*>& Get() { return data; }
			static Expected<Record*> Lookup(unsigned int baseID);
			static Expected<Record*> Lookup(unsigned int baseID, const std::string& type);
			static Expected<Record*> Lookup(unsigned int baseID, const std::vector<std::string>& type);

			static bool IsValidCell(unsigned int baseID) noexcept;
			static bool IsValidWeather(unsigned int baseID) noexcept;
			static bool IsValidSound(unsigned int baseID) noexcept;
			static bool IsValidCoordinate(unsigned int baseID, float X, float Y, float Z) noexcept;

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
