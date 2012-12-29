#include "Record.h"

using namespace std;
using namespace DB;

unordered_map<unsigned int, const Record*> Record::data;

Record::Record(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 4)
		throw VaultException("Malformed input database (records): %s", table.c_str());

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 3));
	// if DLC enabled

	dlc <<= 24;

	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 0));
	name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
	description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
	type = table;

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}
	else
		data.erase(baseID);

	data.insert(make_pair(baseID, this));
}

Expected<const Record*> Record::Lookup(unsigned int baseID)
{
	auto it = data.find(baseID);

	if (it != data.end())
		return it->second;

	return VaultException("No record with baseID %08X found", baseID);
}

Expected<const Record*> Record::Lookup(unsigned int baseID, const string& type)
{
	auto it = data.find(baseID);

	if (it != data.end() && !it->second->GetType().compare(type))
		return it->second;

	return VaultException("No record with baseID %08X and type %s found", baseID, type.c_str());
}

Expected<const Record*> Record::Lookup(unsigned int baseID, const vector<string>& types)
{
	auto it = data.find(baseID);

	if (it != data.end())
	{
		const string& type = it->second->GetType();

		if (find(types.begin(), types.end(), type) != types.end())
			return it->second;
	}

	return VaultException("No record with baseID %08X found", baseID);
}

Expected<const Record*> Record::GetRecordNotIn(const unordered_set<unsigned int>& _set, const function<bool(const Record&)>& pred)
{
	auto it = find_if(data.begin(), data.end(), [&](const pair<const unsigned int, const Record*>& data) { return !_set.count(data.first) && pred(*data.second); });

	if (it != data.end())
		return it->second;

	return VaultException("No record found which is not in the given set");
}

bool Record::IsValidCell(unsigned int baseID)
{
	return Record::Lookup(baseID, "CELL").operator bool();
}

bool Record::IsValidWeather(unsigned int baseID)
{
	return Record::Lookup(baseID, "WTHR").operator bool();
}

unsigned int Record::GetBase() const
{
	return baseID;
}

const string& Record::GetName() const
{
	return name;
}

const string& Record::GetDescription() const
{
	return description;
}

const string& Record::GetType() const
{
	return type;
}
