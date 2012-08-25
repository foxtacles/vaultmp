#include "Race.h"

unordered_map<unsigned int, const Race*> Race::races;

Race::Race(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 5)
		throw VaultException("Malformed input database (races): %s", table.c_str());

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 4));
	// if DLC enabled

	dlc <<= 24;

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	child = static_cast<bool>(sqlite3_column_int(stmt, 1));
	younger = static_cast<unsigned int>(sqlite3_column_int(stmt, 2));
	older = static_cast<unsigned int>(sqlite3_column_int(stmt, 3));

	if (younger & 0xFF000000)
	{
		younger &= 0x00FFFFFF;
		younger |= dlc;
	}

	if (older & 0xFF000000)
	{
		older &= 0x00FFFFFF;
		older |= dlc;
	}

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}
	else
		races.erase(baseID);

	races.emplace(baseID, this);
}

Race::~Race()
{
	races.erase(baseID);
}

const Race& Race::Lookup(unsigned int baseID)
{
	auto it = races.find(baseID);

	if (it != races.end())
		return *it->second;

	throw VaultException("No race with baseID %08X found", baseID);
}

unsigned int Race::GetBase() const
{
	return baseID;
}

bool Race::GetChild() const
{
	return child;
}

unsigned int Race::GetYounger() const
{
	return younger;
}

unsigned int Race::GetOlder() const
{
	return older;
}
