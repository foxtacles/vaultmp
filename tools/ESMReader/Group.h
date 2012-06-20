#pragma once

#include "Common.h"

namespace ESMLoader
{

	struct Group
	{
		char type[4];
		unsigned int groupSize;
		unsigned byte label[4];
		long groupType;
		unsigned short stamp;
		unsigned short unknown;
		unsigned short version;
		unsigned short unknown2;
	};

	class CGroup
	{
		Group group;

		//Extra data for processing
		char* dataStart;
		char* dataEnd;
	public:
		CGroup();
		~CGroup();
		void Load(char*);
		int Size();
		void Dump();

		char* GetDataStart();
		char* GetDataEnd();
	};

};