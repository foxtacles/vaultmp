#include "Item.hpp"
#include "sqlite/sqlite3.h"

using namespace std;
using namespace DB;

unordered_map<unsigned int, Item*> Item::items;

Item::Item(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 6)
		throw VaultException("Malformed input database (items): %s", table.c_str()).stacktrace();

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 5));
	// if DLC enabled

	dlc <<= 24;

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	value = sqlite3_column_int(stmt, 1);
	health = sqlite3_column_int(stmt, 2);
	weight = sqlite3_column_double(stmt, 3);
	slot = sqlite3_column_int(stmt, 4);

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}
	else
		items.erase(baseID);

	items.emplace(baseID, this);
}

Expected<Item*> Item::Lookup(unsigned int baseID)
{
	auto it = items.find(baseID);

	if (it != items.end())
		return it->second;

	return VaultException("No item with baseID %08X found", baseID);
}

unsigned int Item::GetBase() const
{
	return baseID;
}

unsigned int Item::GetValue() const
{
	return value;
}

unsigned int Item::GetHealth() const
{
	return health;
}

float Item::GetWeight() const
{
	return weight;
}

unsigned int Item::GetSlot() const
{
	return slot;
}
