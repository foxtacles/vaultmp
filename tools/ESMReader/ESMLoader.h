#include <iostream>

#include <vector>

#include <stdio.h>

#include "Common.h"

using namespace std;

namespace ESMLoader
{

	extern vector<standardRecord> recordPointers;

	int ae_load_file_to_memory(const char *filename, char **result);

	void Load(char* f);

};