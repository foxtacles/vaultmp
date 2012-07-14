#include "Database.h"
#include "Record.h"

#ifdef VAULTMP_DEBUG
template <typename T>
Debug* Database<T>::debug = NULL;
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
Database<T>::Database()
{

}

template <typename T>
unsigned int Database<T>::initialize(const string& file, const vector<string>& tables)
{
	data.clear();

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

	for (const string& table : tables)
	{
		sqlite3_stmt* stmt;
		string query = "SELECT * FROM " + table;

		if (sqlite3_prepare_v2(db, query.c_str(), query.length() + 1, &stmt, NULL) != SQLITE_OK)
		{
			sqlite3_close(db);
			throw VaultException("Could not prepare query: %s", sqlite3_errmsg(db));
		}

		int ret = sqlite3_step(stmt);

		if (ret == SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			sqlite3_close(db);
			return 0;
		}

		do
		{
			if (ret != SQLITE_ROW)
			{
				sqlite3_finalize(stmt);
				sqlite3_close(db);
				throw VaultException("Failed processing query: %s", sqlite3_errmsg(db));
			}

			try
			{
				data.push_back(T(table, stmt));
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
		debug->PrintFormat("Successfully read %d records from %s", true, data.size(), _file);
#endif

	return data.size();
}

template <typename T>
Database<T>::~Database()
{

}

template class Database<Record>;
