#include "Utils.h"

namespace ESMLoader
{

	void ParseCELL(CRecord* r,int* formID,char**editorID,int*gridx,int*gridy)
	{
		(*formID)=r->GetFormID();
		(*editorID)=r->editorID;
		(*gridx)=r->grid[0];
		(*gridy)=r->grid[1];
	}

	void ParseREFR(CRecord* r,int* formID,int* relFormID,float* x,float* y,float* z)
	{
		(*formID)=r->GetFormID();
		(*relFormID)=r->paramFormID;
		(*x)=r->pos[0];
		(*y)=r->pos[1];
		(*z)=r->pos[2];
	}

	void ParseACHR(CRecord* r,int* formID,char**editorID,float* x,float* y,float* z)	//Actor
	{
		(*formID)=r->GetFormID();
		(*editorID)=r->editorID;
		(*x)=r->pos[0];
		(*y)=r->pos[1];
		(*z)=r->pos[2];
	}

	void ParseAMMO(CRecord* r,int* formID,char**editorID)
	{
		(*formID)=r->GetFormID();
		(*editorID)=r->editorID;
	}

	void ParseWEAP(CRecord* r,int* formID,char**editorID)
	{
		(*formID)=r->GetFormID();
		(*editorID)=r->editorID;
	}

	void ParseNPC_(CRecord* r,int* formID,char**editorID)
	{
		(*formID)=r->GetFormID();
		(*editorID)=r->editorID;
	}

};