#ifndef CELL_H
#define CELL_H

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <cmath>

#include "vaultmp.h"
#include "vaultserver.h"
#include "Database.h"
#include "Record.h"

#include "sqlite/sqlite3.h"

/**
 * \brief Represents a game exterior cell
 */

class Exterior
{
	private:
		static constexpr double size = 4096.0;

		static std::unordered_map<unsigned int, const Exterior*> cells;
		static std::unordered_map<unsigned int, std::vector<const Exterior*>> worlds;

		unsigned int baseID;
		unsigned int world;
		signed int x;
		signed int y;

		Exterior(const Exterior&) = delete;
		Exterior& operator=(const Exterior& p) = delete;

	public:
		static const Exterior& Lookup(unsigned int baseID);
		static const Exterior& Lookup(unsigned int world, double X, double Y);

		unsigned int GetBase() const;
		unsigned int GetWorld() const;
		signed int GetX() const;
		signed int GetY() const;

		Exterior(const std::string& table, sqlite3_stmt* stmt);
		~Exterior() = default;
		// must never be called. only defined because vector requires it
		Exterior(Exterior&&) { std::terminate(); }
		Exterior& operator=(Exterior&&) = delete;
};

#endif
