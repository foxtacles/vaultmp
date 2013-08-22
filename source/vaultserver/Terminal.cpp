#include "Terminal.hpp"
#include "API.hpp"
#include "sqlite/sqlite3.h"

using namespace std;
using namespace DB;
using namespace Values;

unordered_map<unsigned int, Terminal*> Terminal::terminals;

Terminal::Terminal(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 4)
		throw VaultException("Malformed input database (terminals): %s", table.c_str()).stacktrace();

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 3));
	// if DLC enabled

	dlc <<= 24;

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	lock = static_cast<unsigned int>(sqlite3_column_int(stmt, 1));

	if (lock == UINT_MAX - 1) // -2
		lock = Lock_Unlocked;

	note = static_cast<unsigned int>(sqlite3_column_int(stmt, 2));

	if (note & 0xFF000000)
	{
		note &= 0x00FFFFFF;
		note |= dlc;
	}

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}
	else
		terminals.erase(baseID);

	terminals.emplace(baseID, this);
}

Expected<Terminal*> Terminal::Lookup(unsigned int baseID)
{
	auto it = terminals.find(baseID);

	if (it != terminals.end())
		return it->second;

	return VaultException("No terminal with baseID %08X found", baseID);
}

unsigned int Terminal::GetBase() const
{
	return baseID;
}

unsigned int Terminal::GetLock() const
{
	return lock;
}

unsigned int Terminal::GetNote() const
{
	return note;
}
