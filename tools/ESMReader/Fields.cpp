#include "Fields.h"

namespace ESMLoader
{

	CField::CField()
	{

	}

	CField::~CField()
	{

	}

	void CField::Load(char* d)
	{
		memcpy(&field,d,sizeof(Field));
		dataStart=d+sizeof(Field);
		dataEnd=dataStart+field.dataSize;
	}

	int CField::Size()
	{
		int tmp=sizeof(Field)+field.dataSize;
		return tmp;
	}

	void CField::Dump()
	{
		cout<<"\t\tType:"<<field.type[0]<<field.type[1]<<field.type[2]<<field.type[3]<<endl;
		if(strncmp(field.type,"NAME",4)==0)
			cout<<"\t\tName:"<<hex<<"0x"<<*((int*)(this->dataStart))<<dec<<endl;
		cout<<"\t\tSize:"<<field.dataSize<<endl;
	}

	char* CField::GetType()
	{
		return field.type;
	}

	char* CField::GetDataStart()
	{
		return dataStart;
	}

	char* CField::GetDataEnd()
	{
		return dataEnd;
	}

};