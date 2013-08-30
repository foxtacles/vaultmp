#include "Exterior.hpp"
#include "sqlite/sqlite3.h"

#include <algorithm>

using namespace std;
using namespace DB;

unordered_map<unsigned int, Exterior*> Exterior::cells;
unordered_map<unsigned int, vector<Exterior*>> Exterior::worlds;

Exterior::Exterior(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 5)
		throw VaultException("Malformed input database (exteriors): %s", table.c_str()).stacktrace();

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 4));
	// if DLC enabled

	dlc <<= 24;

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	x = sqlite3_column_int(stmt, 1);
	y = sqlite3_column_int(stmt, 2);
	world = static_cast<unsigned int>(sqlite3_column_int(stmt, 3));

	if (world & 0xFF000000)
	{
		world &= 0x00FFFFFF;
		world |= dlc;
	}

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}
	else
	{
		cells.erase(baseID);
		worlds[world].erase(remove_if(worlds[world].begin(), worlds[world].end(), [this](const Exterior* cell) { return cell->GetBase() == baseID; }), worlds[world].end());
	}

	cells.emplace(baseID, this);
	worlds[world].emplace_back(this);

	//All exteriors are also CELLs
	//const Record& record = Record::Lookup(baseID, "CELL");
}

Expected<Exterior*> Exterior::Lookup(unsigned int baseID)
{
	auto it = cells.find(baseID);

	if (it != cells.end())
		return it->second;

	return VaultException("No cell with baseID %08X found", baseID);
}

Expected<Exterior*> Exterior::Lookup(unsigned int world, float X, float Y)
{
	signed int x = floor(X / SIZE);
	signed int y = floor(Y / SIZE);

	auto it = find_if(worlds[world].begin(), worlds[world].end(), [&x, &y](const Exterior* cell) { return cell->x == x && cell->y == y; });

	if (it != worlds[world].end())
		return *it;

	return VaultException("No cell with coordinates (%f, %f) in world %08X found", X, Y, world);
}

unsigned int Exterior::GetBase() const
{
	return baseID;
}

unsigned int Exterior::GetWorld() const
{
	return world;
}

signed int Exterior::GetX() const
{
	return x;
}

signed int Exterior::GetY() const
{
	return y;
}

array<unsigned int, 9> Exterior::GetAdjacents() const
{
	float X = GetX() * SIZE;
	float Y = GetY() * SIZE;

	Expected<Exterior*> next_exterior;

	return
		{{GetBase(),
		(next_exterior = DB::Exterior::Lookup(world, X, Y + SIZE)) ? next_exterior->GetBase() : 0u,
		(next_exterior = DB::Exterior::Lookup(world, X + SIZE, Y + SIZE)) ? next_exterior->GetBase() : 0u,
		(next_exterior = DB::Exterior::Lookup(world, X + SIZE, Y)) ? next_exterior->GetBase() : 0u,
		(next_exterior = DB::Exterior::Lookup(world, X + SIZE, Y - SIZE)) ? next_exterior->GetBase() : 0u,
		(next_exterior = DB::Exterior::Lookup(world, X, Y - SIZE)) ? next_exterior->GetBase() : 0u,
		(next_exterior = DB::Exterior::Lookup(world, X - SIZE, Y - SIZE)) ? next_exterior->GetBase() : 0u,
		(next_exterior = DB::Exterior::Lookup(world, X - SIZE, Y)) ? next_exterior->GetBase() : 0u,
		(next_exterior = DB::Exterior::Lookup(world, X - SIZE, Y + SIZE)) ? next_exterior->GetBase() : 0u}};
}

bool Exterior::IsValidCoordinate(float X, float Y) const
{
	float x1 = x * SIZE;
	float y1 = y * SIZE;
	float x2 = x1 + SIZE;
	float y2 = y1 + SIZE;
	return ((X >= x1 && X < x2) && (Y >= y1 && Y < y2));
}
