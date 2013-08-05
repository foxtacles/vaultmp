#ifndef REFERENCEDB_H
#define REFERENCEDB_H

#include "vaultserver.h"
#include "Expected.h"

#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>

class sqlite3_stmt;

/**
 * \brief Represents a game weapon
 */

namespace DB
{
	class Reference
	{
		private:
			static std::unordered_map<unsigned int, Reference*> refs;

			std::string type;
			std::string editor;
			unsigned int refID;
			unsigned int baseID;
			unsigned int count;
			double health;
			unsigned int cell;
			std::tuple<double, double, double> pos, angle;
			unsigned int flags;
			unsigned int lock;
			unsigned int key;
			unsigned int link;

			Reference(const Reference&) = delete;
			Reference& operator=(const Reference&) = delete;

		public:
			static Expected<Reference*> Lookup(unsigned int refID);
			static std::vector<Reference*> Lookup(const std::string& type);

			const std::string& GetType() const;
			const std::string& GetEditor() const;
			unsigned int GetReference() const;
			unsigned int GetBase() const;
			unsigned int GetCount() const;
			double GetHealth() const;
			unsigned int GetCell() const;
			const std::tuple<double, double, double>& GetPos() const;
			const std::tuple<double, double, double>& GetAngle() const;
			unsigned int GetFlags() const;
			unsigned int GetLock() const;
			unsigned int GetKey() const;
			unsigned int GetLink() const;

			Reference(const std::string& table, sqlite3_stmt* stmt);
			~Reference() = default;
			// must never be called. only defined because vector requires it
			Reference(Reference&&) { std::terminate(); }
			Reference& operator=(Reference&&) = delete;
	};
}

#endif
