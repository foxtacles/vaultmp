#include "Database.h"
#include "Record.h"
#include "Exterior.h"
#include "Weapon.h"
#include "Race.h"
#include "NPC.h"
#include "BaseContainer.h"

using namespace std;

#ifdef VAULTMP_DEBUG
template <typename T>
Debug* Database<T>::debug = nullptr;
#endif

#ifdef VAULTMP_DEBUG
template <typename T>
void Database<T>::SetDebugHandler(Debug* debug)
{
	Database::debug = debug;

	if (debug)
		debug->PrintFormat("Attached debug handler to Database<%s> class", true, typeid(T).name());
}
#endif

template <typename T>
unsigned int Database<T>::initialize(const string& file, const vector<string>& tables)
{
	data.clear();

	if (tables.empty())
		return 0;

	char base[MAX_PATH];
	_getcwd(base, sizeof(base));

	char _file[MAX_PATH];
	snprintf(_file, sizeof(_file), "%s/%s/%s", base, DATA_PATH, file.c_str());

	sqlite3* db;

	if (sqlite3_open_v2(_file, &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
	{
		sqlite3_close(db);
		throw VaultException("Could not open SQLite3 database: %s", sqlite3_errmsg(db));
	}

	sqlite3_stmt* stmt;
	string query = "SELECT (0) ";

	for (const string& table : tables)
		query += "+ (SELECT COUNT(*) FROM " + table + ")";

	if (sqlite3_prepare_v2(db, query.c_str(), query.length() + 1, &stmt, nullptr) != SQLITE_OK)
	{
		sqlite3_close(db);
		throw VaultException("Could not prepare query: %s", sqlite3_errmsg(db));
	}

	int ret = sqlite3_step(stmt);

	if (ret != SQLITE_ROW)
	{
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		throw VaultException("Could not determine count of tables: %s", query.c_str());
	}

	unsigned int count = sqlite3_column_int(stmt, 0);
	sqlite3_finalize(stmt);

	if (!count)
	{
		sqlite3_close(db);
		return 0;
	}

	data.reserve(count);

	double end = 0.0;
	double delta = 100.0 / count;
	printf("Reading database %s (%s, ...)\n", file.c_str(), tables.front().c_str());

	for (const string& table : tables)
	{
		query = "SELECT * FROM " + table;

		if (sqlite3_prepare_v2(db, query.c_str(), query.length() + 1, &stmt, nullptr) != SQLITE_OK)
		{
			sqlite3_close(db);
			throw VaultException("Could not prepare query: %s", sqlite3_errmsg(db));
		}

		int ret = sqlite3_step(stmt);

		if (ret == SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			continue;
		}

		do
		{
			if (static_cast<unsigned int>(end + delta) > static_cast<unsigned int>(end))
				Utils::progress_func(100.0, end);

			end += delta;

			if (ret != SQLITE_ROW)
			{
				sqlite3_finalize(stmt);
				sqlite3_close(db);
				throw VaultException("Failed processing query: %s", sqlite3_errmsg(db));
			}

			try
			{
				data.emplace_back(table, stmt);
			}
			catch (...)
			{
				sqlite3_finalize(stmt);
				sqlite3_close(db);
				throw;
			}
		} while ((ret = sqlite3_step(stmt)) != SQLITE_DONE);

		sqlite3_finalize(stmt);
	}

	sqlite3_close(db);

#ifdef VAULTMP_DEBUG
	if (debug)
		debug->PrintFormat("Successfully read %d records (%s) from %s", true, data.size(), typeid(T).name(), _file);
#endif

	return data.size();
}

template class Database<Record>;
template class Database<Exterior>;
template class Database<Weapon>;
template class Database<Race>;
template class Database<NPC>;
template class Database<BaseContainer>;
