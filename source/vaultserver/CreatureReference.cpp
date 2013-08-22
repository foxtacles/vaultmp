#include "CreatureReference.hpp"
#include "sqlite/sqlite3.h"

#include <cmath>

using namespace std;
using namespace DB;

unordered_map<unsigned int, CreatureReference*> CreatureReference::refs;

CreatureReference::CreatureReference(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 12)
		throw VaultException("Malformed input database (creature references): %s", table.c_str()).stacktrace();

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 11));
	// if DLC enabled

	dlc <<= 24;

	constexpr double degrees = 180.0 / M_PI;

	editor = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
	refID = static_cast<unsigned int>(sqlite3_column_int(stmt, 1));
	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 2));
	cell = static_cast<unsigned int>(sqlite3_column_int(stmt, 3));
	pos = make_tuple(sqlite3_column_double(stmt, 4), sqlite3_column_double(stmt, 5), sqlite3_column_double(stmt, 6));
	angle = make_tuple(sqlite3_column_double(stmt, 7) * degrees, sqlite3_column_double(stmt, 8) * degrees, sqlite3_column_double(stmt, 9) * degrees);
	flags = static_cast<unsigned int>(sqlite3_column_int(stmt, 10));

	if (refID & 0xFF000000)
	{
		refID &= 0x00FFFFFF;
		refID |= dlc;
	}
	else
		refs.erase(refID);

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}

	if (cell & 0xFF000000)
	{
		cell &= 0x00FFFFFF;
		cell |= dlc;
	}

	refs.emplace(refID, this);
}

Expected<CreatureReference*> CreatureReference::Lookup(unsigned int refID)
{
	auto it = refs.find(refID);

	if (it != refs.end())
		return it->second;

	return VaultException("No creature reference with refID %08X found", refID);
}

const string& CreatureReference::GetEditor() const
{
	return editor;
}

unsigned int CreatureReference::GetReference() const
{
	return refID;
}

unsigned int CreatureReference::GetBase() const
{
	return baseID;
}

unsigned int CreatureReference::GetCell() const
{
	return cell;
}

const std::tuple<double, double, double>& CreatureReference::GetPos() const
{
	return pos;
}

const std::tuple<double, double, double>& CreatureReference::GetAngle() const
{
	return angle;
}

unsigned int CreatureReference::GetFlags() const
{
	return flags;
}
