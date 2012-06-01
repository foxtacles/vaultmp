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
	sqlite3_close(db);
}

pair<string, unsigned char> Database::Lookup(unsigned int baseID)
{
	unordered_map<unsigned int, Record>::iterator it = data.find(baseID);

	if (it != data.end())
		return make_pair(it->second.name, it->second.dlc);
}
