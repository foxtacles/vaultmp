#ifndef CREATUREREFERENCEDB_H
#define CREATUREREFERENCEDB_H

#include "vaultserver.hpp"
#include "Expected.hpp"

#include <string>
#include <tuple>
#include <unordered_map>

class sqlite3_stmt;

/**
 * \brief Represents a game creature reference
 */

namespace DB
{
	class CreatureReference
	{
		private:
			static std::unordered_map<unsigned int, CreatureReference*> refs;

			std::string editor;
			unsigned int refID;
			unsigned int baseID;
			unsigned int cell;
			std::tuple<double, double, double> pos, angle;
			unsigned int flags;

			CreatureReference(const CreatureReference&) = delete;
			CreatureReference& operator=(const CreatureReference&) = delete;

		public:
			static const std::unordered_map<unsigned int, CreatureReference*>& Get() { return refs; }
			static Expected<CreatureReference*> Lookup(unsigned int refID);

			const std::string& GetEditor() const;
			unsigned int GetReference() const;
			unsigned int GetBase() const;
			unsigned int GetCell() const;
			const std::tuple<double, double, double>& GetPos() const;
			const std::tuple<double, double, double>& GetAngle() const;
			unsigned int GetFlags() const;

			CreatureReference(const std::string& table, sqlite3_stmt* stmt);
			~CreatureReference() = default;
			// must never be called. only defined because vector requires it
			CreatureReference(CreatureReference&&) { std::terminate(); }
			CreatureReference& operator=(CreatureReference&&) = delete;
	};
}

#endif
