#include "Record.h"
#include "Group.h"
#include "Fields.h"
#include "Utils.h"

#include <vector>

#include <stdio.h>

namespace ESMLoader
{

	extern vector<CRecord*> records;

	int ae_load_file_to_memory(const char *filename, char **result);

	void ParseSubRecord(CField* f,CRecord *r);

	void ParseRecord(char** cursor,CRecord* r);

	void ParseGroup(char** cursor,CGroup* grp);

	void Load(char* f);
	
	void Unload();

};