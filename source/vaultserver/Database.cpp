#include "Database.h"

#ifdef VAULTMP_DEBUG
Debug* Database::debug = NULL;
#endif

#ifdef VAULTMP_DEBUG
void Database::SetDebugHandler(Debug* debug)
{
	Database::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Database class", true);
}
#endif

Database::Database(string file, string table)
{
	char base[MAX_PATH];
	_getcwd(base, sizeof(base));

	char _file[MAX_PATH];
	snprintf(_file, sizeof(_file), "%s/%s/%s", base, DATA_PATH, file.c_str());

	sqlite3* db;

	if (sqlite3_open_v2(_file, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
	{
		sqlite3_close(db);
		throw VaultException("Could not open SQLite3 database: %s", sqlite3_errmsg(db));
	}

	sqlite3_stmt* stmt;
	string query = "SELECT * FROM " + table;

	if (sqlite3_prepare_v2(db, query.c_str(), query.length() + 1, &stmt, NULL) != SQLITE_OK)
	{
		sqlite3_close(db);
		throw VaultException("Could not prepare query: %s", sqlite3_errmsg(db));
	}

	int ret = sqlite3_step(stmt);

	if (sqlite3_column_count(stmt) != 3)
	{
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		throw VaultException("Malformed input database: %s", _file);
	}

	if (ret == SQLITE_DONE)
	{
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		return;
	}

	do
	{
		if (ret != SQLITE_ROW)
		{
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			throw VaultException("Failed processing query: %s", sqlite3_errmsg(db));
		}

		unsigned int baseID = sqlite3_column_int(stmt, 0);
		const unsigned char* name = sqlite3_column_text(stmt, 1);
		unsigned int dlc = sqlite3_column_int(stmt, 2) - 1;

		baseID = (baseID & 0x00FFFFFF) | (dlc << 24);

		data.insert(make_pair(baseID, Record(baseID, reinterpret_cast<const char*>(name))));
	} while ((ret = sqlite3_step(stmt)) != SQLITE_DONE);

	sqlite3_finalize(stmt);
	sqlite3_close(db);

#ifdef VAULTMP_DEBUG
	if (debug)
		debug->PrintFormat("Successfully read %d records from %s", true, data.size(), _file);
#endif
}

Database::~Database()
{

}

const Database::Record& Database::Lookup(unsigned int baseID)
{
	unordered_map<unsigned int, Record>::iterator it = data.find(baseID);

	if (it != data.end())
		return it->second;

	throw VaultException("No database record with baseID %08X found", baseID);
}

const Database::Record& Database::GetRecordNotIn(const unordered_set<unsigned int>& _set)
{
	unordered_map<unsigned int, Record>::iterator it = find_if_not(data.begin(), data.end(), [&](const pair<unsigned int, Record>& data) { return _set.count(data.first); });

	if (it != data.end())
		return it->second;

	throw VaultException("No database record found which is not in the given set");
}
