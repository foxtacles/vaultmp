#include "Interior.hpp"
#include "sqlite/sqlite3.h"

using namespace std;
using namespace DB;

unordered_map<unsigned int, Interior*> Interior::cells;

Interior::Interior(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 8)
		throw VaultException("Malformed input database (interiors): %s", table.c_str()).stacktrace();

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 7));
	// if DLC enabled

	dlc <<= 24;

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	x1 = sqlite3_column_double(stmt, 1);
	y1 = sqlite3_column_double(stmt, 2);
	z1 = sqlite3_column_double(stmt, 3);
	x2 = sqlite3_column_double(stmt, 4);
	y2 = sqlite3_column_double(stmt, 5);
	z2 = sqlite3_column_double(stmt, 6);

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}
	else
	{
		// recalculate boundaries
	}

	cells.emplace(baseID, this);
}

Expected<Interior*> Interior::Lookup(unsigned int baseID)
{
	auto it = cells.find(baseID);

	if (it != cells.end())
		return it->second;

	return VaultException("No cell with baseID %08X found", baseID);
}

unsigned int Interior::GetBase() const
{
	return baseID;
}

array<float, 6> Interior::GetBounds() const
{
	return {{x1, y1, z1, x2, y2, z2}};
}

bool Interior::IsValidCoordinate(float X, float Y, float Z) const
{
	return ((X >= x2 && X <= x1) && (Y >= y2 && Y <= y1) && (Z >= z2 && Z <= z1));
}
