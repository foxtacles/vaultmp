#ifndef INTERIORDB_H
#define INTERIORDB_H

#include "vaultserver.hpp"
#include "Expected.hpp"

#include <unordered_map>
#include <array>

class sqlite3_stmt;

/**
 * \brief Represents a game interior cell
 */

namespace DB
{
	class Interior
	{
		private:
			static std::unordered_map<unsigned int, Interior*> cells;

			unsigned int baseID;
			double x1, y1, z1;
			double x2, y2, z2;

			Interior(const Interior&) = delete;
			Interior& operator=(const Interior&) = delete;

		public:
			static Expected<Interior*> Lookup(unsigned int baseID);

			unsigned int GetBase() const;
			std::array<double, 6> GetBounds() const;
			bool IsValidCoordinate(double X, double Y, double Z) const;

			Interior(const std::string& table, sqlite3_stmt* stmt);
			~Interior() = default;
			// must never be called. only defined because vector requires it
			Interior(Interior&&) { std::terminate(); }
			Interior& operator=(Interior&&) = delete;
	};
}

#endif
