#include "Race.hpp"
#include "sqlite/sqlite3.h"

using namespace std;
using namespace DB;

unordered_map<unsigned int, Race*> Race::races;

Race::Race(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 5)
		throw VaultException("Malformed input database (races): %s", table.c_str()).stacktrace();

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

Expected<Race*> Race::Lookup(unsigned int baseID)
{
	auto it = races.find(baseID);

	if (it != races.end())
		return it->second;

	return VaultException("No race with baseID %08X found", baseID);
}

unsigned int Race::GetBase() const
{
	return baseID;
}

bool Race::IsChild() const
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

unsigned int Race::GetAge() const
{
	unsigned int age = 0;
	const Race* race = this;

	while (race->younger)
	{
		++age;
		race = *Lookup(race->younger);
	}

	return age;
}

unsigned int Race::GetMaxAge() const
{
	unsigned int age = GetAge();
	const Race* race = this;

	while (race->older)
	{
		++age;
		race = *Lookup(race->older);
	}

	return age;
}

signed int Race::GetAgeDifference(unsigned int race) const
{
	const Race* other_race = *Lookup(race);

	unsigned int age = GetAge();

	if (age > other_race->GetMaxAge())
		return 0;

	return other_race->GetAge() - age;
}
