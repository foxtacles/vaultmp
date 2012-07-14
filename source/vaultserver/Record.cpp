#include "Record.h"

unordered_map<unsigned int, const Record*> Record::data;

Record::Record(const string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 4)
		throw VaultException("Malformed input database (records): %s", table.c_str());

	baseID = (static_cast<unsigned int>(sqlite3_column_int(stmt, 0)) & 0x00FFFFFF) | (static_cast<unsigned int>(sqlite3_column_int(stmt, 3)) << 24);
	name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
	description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
	type = table;

	data.insert(make_pair(baseID, this));
}

Record::~Record()
{
	data.erase(baseID);
}

const Record& Record::Lookup(unsigned int baseID)
{
	unordered_map<unsigned int, const Record*>::iterator it = data.find(baseID);

	if (it != data.end())
		return *it->second;

	throw VaultException("No record with baseID %08X found", baseID);
}

const Record& Record::GetRecordNotIn(const unordered_set<unsigned int>& _set)
{
	unordered_map<unsigned int, const Record*>::iterator it = find_if_not(data.begin(), data.end(), [&](const pair<unsigned int, const Record*>& data) { return _set.count(data.first); });

	if (it != data.end())
		return *it->second;

	throw VaultException("No record found which is not in the given set");
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
