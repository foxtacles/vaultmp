#include "AcReference.hpp"
#include "sqlite/sqlite3.h"

#include <cmath>

using namespace std;
using namespace DB;

unordered_map<unsigned int, AcReference*> AcReference::refs;

AcReference::AcReference(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 12)
		throw VaultException("Malformed input database (actor / creature references): %s", table.c_str()).stacktrace();

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

Expected<AcReference*> AcReference::Lookup(unsigned int refID)
{
	auto it = refs.find(refID);

	if (it != refs.end())
		return it->second;

	return VaultException("No actor / creature reference with refID %08X found", refID);
}

const string& AcReference::GetEditor() const
{
	return editor;
}

unsigned int AcReference::GetReference() const
{
	return refID;
}

unsigned int AcReference::GetBase() const
{
	return baseID;
}

unsigned int AcReference::GetCell() const
{
	return cell;
}

const std::tuple<float, float, float>& AcReference::GetPos() const
{
	return pos;
}

const std::tuple<float, float, float>& AcReference::GetAngle() const
{
	return angle;
}

unsigned int AcReference::GetFlags() const
{
	return flags;
}
