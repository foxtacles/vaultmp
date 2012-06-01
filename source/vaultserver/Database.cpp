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
