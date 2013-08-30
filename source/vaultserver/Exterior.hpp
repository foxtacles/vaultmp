#ifndef EXTERIORDB_H
#define EXTERIORDB_H

#include "vaultserver.hpp"
#include "Expected.hpp"

#include <vector>
#include <array>
#include <unordered_map>

class sqlite3_stmt;

/**
 * \brief Represents a game exterior cell
 */

namespace DB
{
	class Exterior
	{
		private:
			static std::unordered_map<unsigned int, Exterior*> cells;
			static std::unordered_map<unsigned int, std::vector<Exterior*>> worlds;

			unsigned int baseID;
			unsigned int world;
			signed int x;
			signed int y;

			Exterior(const Exterior&) = delete;
			Exterior& operator=(const Exterior&) = delete;

		public:
			static constexpr float SIZE = 4096.0f;

			static Expected<Exterior*> Lookup(unsigned int baseID);
			static Expected<Exterior*> Lookup(unsigned int world, float X, float Y);

			unsigned int GetBase() const;
			unsigned int GetWorld() const;
			signed int GetX() const;
			signed int GetY() const;
			std::array<unsigned int, 9> GetAdjacents() const;
			bool IsValidCoordinate(float X, float Y) const;

			Exterior(const std::string& table, sqlite3_stmt* stmt);
			~Exterior() = default;
			// must never be called. only defined because vector requires it
			Exterior(Exterior&&) { std::terminate(); }
			Exterior& operator=(Exterior&&) = delete;
	};
}

#endif
