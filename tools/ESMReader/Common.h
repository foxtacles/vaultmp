#pragma once

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "zlib/zlib.h"

using namespace std;

#define byte char

struct MNAM
{
	unsigned int width;
	unsigned int height;
	unsigned short nwCellX,nwCellY,seCellX,seCellY;
};

struct standardRecord
{
	int type;
	char* start;
	char* end;
	int parent;

	int groupType;

	int formID;
	char EDID[64];
	char FULL[128];
	int XCLC[2];

	struct  
	{
		unsigned int width;
		unsigned int height;
		unsigned short NW_cell_X;	
		unsigned short NW_cell_Y;
		unsigned short SE_cell_X;
		unsigned short SE_cell_Y;
	} MNAM;

	float ONAM[3];
};

struct RECORD_NPC
{
	unsigned int formID;
	char fullName[128];
	char editorID[64];
};

struct RECORD_WEAP
{
	unsigned int formID;
	char fullName[128];
	char editorID[64];
};

struct RECORD_AMMO
{
	unsigned int formID;
	char fullName[128];
	char editorID[64];
};


#define RECORD_GROUP 1
#define RECORD_RECORD 2
#define RECORD_FIELD 3