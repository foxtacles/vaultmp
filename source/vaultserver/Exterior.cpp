#include "Exterior.h"

unordered_map<unsigned int, const Exterior*> Exterior::cells;
unordered_map<unsigned int, vector<const Exterior*>> Exterior::worlds;

Exterior::Exterior(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 6)
		throw VaultException("Malformed input database (cells): %s", table.c_str());

	unsigned char dlc = static_cast<unsigned char>(sqlite3_column_int(stmt, 5));
	// if DLC enabled

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	world = static_cast<unsigned int>(sqlite3_column_int(stmt, 4));
	name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
	x = sqlite3_column_int(stmt, 2);
	y = sqlite3_column_int(stmt, 3);

	if (world & 0xFF000000)
	{
		world &= 0x00FFFFFF;
		world |= (static_cast<unsigned int>(dlc) << 24);
	}

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= (static_cast<unsigned int>(dlc) << 24);
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

Exterior::~Exterior()
{
	cells.erase(baseID);

	auto it = find(worlds[world].begin(), worlds[world].end(), this);

	if (it != worlds[world].end())
		worlds[world].erase(it);
}

const Exterior& Exterior::Lookup(unsigned int baseID)
{
	unordered_map<unsigned int, const Exterior*>::iterator it = cells.find(baseID);

	if (it != cells.end())
		return *it->second;

	throw VaultException("No cell with baseID %08X found", baseID);
}

const Exterior& Exterior::Lookup(unsigned int world, double X, double Y)
{
	signed int x = floor(X / size);
	signed int y = floor(Y / size);

	vector<const Exterior*>::iterator it = find_if(worlds[world].begin(), worlds[world].end(), [=](const Exterior* cell) { return cell->x == x && cell->y == y; });

	if (it != worlds[world].end())
		return **it;

	throw VaultException("No cell with coordinates (%f, %f) in world %08X found", static_cast<float>(X), static_cast<float>(Y), world);
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

const string& Exterior::GetName() const
{
	return name;
}
