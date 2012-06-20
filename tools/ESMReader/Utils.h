#include "Common.h"
#include "Record.h"

namespace ESMLoader
{

	void ParseCELL(CRecord*,int*,char**,int*,int*);
	void ParseREFR(CRecord*,int*,int*,float*,float*,float*);
	void ParseACHR(CRecord* r,int* formID,char**editorID,float* x,float* y,float* z);
	void ParseAMMO(CRecord* r,int* formID,char**editorID);
	void ParseWEAP(CRecord* r,int* formID,char**editorID);
	void ParseNPC_(CRecord* r,int* formID,char**editorID);

};