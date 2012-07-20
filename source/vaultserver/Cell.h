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

using namespace std;

/**
 * \brief Represents a game exterior cell
 */

class Cell
{
	private:
		static constexpr double size = 4096.0;

		static unordered_map<unsigned int, const Cell*> cells;
		static unordered_map<unsigned int, vector<const Cell*>> worlds;

		unsigned int baseID;
		unsigned int world;
		signed int x;
		signed int y;
		string name;

		Cell(const Cell&) = delete;
		Cell& operator=(const Cell& p) = delete;

	public:
		static const Cell& Lookup(unsigned int baseID);
		static const Cell& Lookup(const Cell& cell, double X, double Y);
		static const Cell& Lookup(unsigned int world, double X, double Y);
		static bool IsValidCell(unsigned int baseID);

		unsigned int GetBase() const;
		unsigned int GetWorld() const;
		signed int GetX() const;
		signed int GetY() const;
		const string& GetName() const;

		Cell(const string& table, sqlite3_stmt* stmt);
		// must never be called. only defined because vector requires it
		Cell(Cell&&) { terminate(); }
		Cell& operator=(Cell&&) = delete;
		~Cell();
};

#endif
