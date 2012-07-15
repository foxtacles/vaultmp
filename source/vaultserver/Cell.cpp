#include "Cell.h"

unordered_map<unsigned int, const Cell*> Cell::cells;
unordered_map<unsigned int, vector<const Cell*>> Cell::worlds;

Cell::Cell(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 6)
		throw VaultException("Malformed input database (cells): %s", table.c_str());

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 5)) << 24;

	baseID = (static_cast<unsigned int>(sqlite3_column_int(stmt, 0)) & 0x00FFFFFF) | dlc;
	world = (static_cast<unsigned int>(sqlite3_column_int(stmt, 4)) & 0x00FFFFFF) | dlc;
	name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
	x = sqlite3_column_int(stmt, 2);
	y = sqlite3_column_int(stmt, 3);

	cells.insert(make_pair(baseID, this));
	worlds[world].push_back(this);
}

Cell::~Cell()
{
	cells.erase(baseID);
	worlds[world].erase(find(worlds[world].begin(), worlds[world].end(), this));
}

const Cell& Cell::Lookup(unsigned int baseID)
{
	unordered_map<unsigned int, const Cell*>::iterator it = cells.find(baseID);

	if (it != cells.end())
		return *it->second;

	throw VaultException("No cell with baseID %08X found", baseID);
}

const Cell& Cell::Lookup(unsigned int world, double X, double Y)
{
	signed int x = floor(X / size);
	signed int y = floor(Y / size);

	vector<const Cell*>::iterator it = find_if(worlds[world].begin(), worlds[world].end(), [=](const Cell* cell) { return cell->x == x && cell->y == y; });

	if (it != worlds[world].end())
		return **it;

	throw VaultException("No cell with coordinates (%f, %f) in world %08X found", static_cast<float>(X), static_cast<float>(Y), world);
}

unsigned int Cell::GetBase() const
{
	return baseID;
}

unsigned int Cell::GetWorld() const
{
	return world;
}

signed int Cell::GetX() const
{
	return x;
}

signed int Cell::GetY() const
{
	return y;
}

const string& Cell::GetName() const
{
	return name;
}
