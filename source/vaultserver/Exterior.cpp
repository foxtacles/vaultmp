#include "Exterior.h"

using namespace std;

unordered_map<unsigned int, const Exterior*> Exterior::cells;
unordered_map<unsigned int, vector<const Exterior*>> Exterior::worlds;

Exterior::Exterior(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 5)
		throw VaultException("Malformed input database (exteriors): %s", table.c_str());

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
		worlds[world].erase(remove_if(worlds[world].begin(), worlds[world].end(), [=](const Exterior* cell) { return cell->GetBase() == baseID; }), worlds[world].end());
	}

	cells.emplace(baseID, this);
	worlds[world].emplace_back(this);

	//All exteriors are also CELLs
	//const Record& record = Record::Lookup(baseID, "CELL");
}

Expected<const Exterior*> Exterior::Lookup(unsigned int baseID)
{
	auto it = cells.find(baseID);

	if (it != cells.end())
		return it->second;

	return VaultException("No cell with baseID %08X found", baseID);
}

Expected<const Exterior*> Exterior::Lookup(unsigned int world, double X, double Y)
{
	signed int x = floor(X / size);
	signed int y = floor(Y / size);

	auto it = find_if(worlds[world].begin(), worlds[world].end(), [=](const Exterior* cell) { return cell->x == x && cell->y == y; });

	if (it != worlds[world].end())
		return *it;

	return VaultException("No cell with coordinates (%f, %f) in world %08X found", static_cast<float>(X), static_cast<float>(Y), world);
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
