#pragma once

#include "Common.h"

namespace ESMLoader
{

	struct Record
	{
		char type[4];
		unsigned long dataSize;
		unsigned long flags;
		unsigned int formID;
		unsigned int revision;
		unsigned short version;
		unsigned short unknown;
		//Data
	};

	class CRecord
	{
		Record record;

		char* dataStart;
		char* dataEnd;
	public:
		//Public fields
		int paramFormID;
		char editorID[64];
		float pos[3];
		int grid[2];

		CRecord();
		~CRecord();
		void Load(char*);
		int Size();

		char* GetType();
		int GetFormID();

		void Dump();
		char* GetDataStart();
		char* GetDataEnd();

		bool DataCompressed();
	};

};