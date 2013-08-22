#include "Reference.hpp"
#include "API.hpp"
#include "Utils.hpp"
#include "sqlite/sqlite3.h"

#include <cmath>

using namespace std;
using namespace DB;
using namespace Values;

unordered_map<unsigned int, Reference*> Reference::refs;

Reference::Reference(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 17)
		throw VaultException("Malformed input database (references): %s", table.c_str()).stacktrace();

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 16));
	// if DLC enabled

	dlc <<= 24;

	constexpr double degrees = 180.0 / M_PI;

	type = Utils::str_replace(table, "refs_", "");
	editor = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
	refID = static_cast<unsigned int>(sqlite3_column_int(stmt, 1));
	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 2));
	count = static_cast<unsigned int>(sqlite3_column_int(stmt, 3));
	health = sqlite3_column_double(stmt, 4);
	cell = static_cast<unsigned int>(sqlite3_column_int(stmt, 5));
	pos = make_tuple(sqlite3_column_double(stmt, 6), sqlite3_column_double(stmt, 7), sqlite3_column_double(stmt, 8));
	angle = make_tuple(sqlite3_column_double(stmt, 9) * degrees, sqlite3_column_double(stmt, 10) * degrees, sqlite3_column_double(stmt, 11) * degrees);
	flags = static_cast<unsigned int>(sqlite3_column_int(stmt, 12));
	lock = static_cast<unsigned int>(sqlite3_column_int(stmt, 13));

	if (lock == UINT_MAX) // -1
		lock = Lock_Impossible; // requires key
	else if (lock == UINT_MAX - 1) // -2
		lock = Lock_Unlocked; // unlocked

	key = static_cast<unsigned int>(sqlite3_column_int(stmt, 14));
	link = static_cast<unsigned int>(sqlite3_column_int(stmt, 15));

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

	if (key & 0xFF000000)
	{
		key &= 0x00FFFFFF;
		key |= dlc;
	}

	if (link & 0xFF000000)
	{
		link &= 0x00FFFFFF;
		link |= dlc;
	}

	refs.emplace(refID, this);
}

Expected<Reference*> Reference::Lookup(unsigned int refID)
{
	auto it = refs.find(refID);

	if (it != refs.end())
		return it->second;

	return VaultException("No weapon with refID %08X found", refID);
}

vector<Reference*> Reference::Lookup(const std::string& type)
{
	vector<Reference*> data;

	for (const auto& ref : refs)
		if (!ref.second->GetType().compare(type))
			data.emplace_back(ref.second);

	return data;
}

const string& Reference::GetType() const
{
	return type;
}

const string& Reference::GetEditor() const
{
	return editor;
}

unsigned int Reference::GetReference() const
{
	return refID;
}

unsigned int Reference::GetBase() const
{
	return baseID;
}

unsigned int Reference::GetCount() const
{
	return count;
}

double Reference::GetHealth() const
{
	return health;
}

unsigned int Reference::GetCell() const
{
	return cell;
}

const std::tuple<double, double, double>& Reference::GetPos() const
{
	return pos;
}

const std::tuple<double, double, double>& Reference::GetAngle() const
{
	return angle;
}

unsigned int Reference::GetFlags() const
{
	return flags;
}

unsigned int Reference::GetLock() const
{
	return lock;
}

unsigned int Reference::GetKey() const
{
	return key;
}

unsigned int Reference::GetLink() const
{
	return link;
}
