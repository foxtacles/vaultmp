#include "Record.h"

namespace ESMLoader
{

	CRecord::CRecord()
	{
		editorID[0]=0;
		paramFormID=0;
	}

	CRecord::~CRecord()
	{

	}

	void CRecord::Load(char* d)
	{
		memcpy(&record,d,sizeof(Record));
		dataStart=d+sizeof(Record);
		dataEnd=dataStart+record.dataSize;
	}

	int CRecord::Size()
	{
		int tmp=sizeof(Record)+record.dataSize;
		return tmp;
	}

	void CRecord::Dump()
	{
		cout<<"\tType:"<<record.type[0]<<record.type[1]<<record.type[2]<<record.type[3]<<endl;
		cout<<"\tSize:"<<record.dataSize<<endl;
		cout<<"\tVersion:"<<record.version<<endl;
		cout<<"\tFormID:"<<hex<<"0x"<<record.formID<<dec<<endl;
		cout<<"\tParamFormID:"<<hex<<"0x"<<paramFormID<<dec<<endl;
		cout<<"\tFlag:"<<record.flags<<endl;
	}

	char* CRecord::GetType()
	{
		return record.type;
	}

	int CRecord::GetFormID()
	{
		return (record.formID);
	}

	char* CRecord::GetDataStart()
	{
		return dataStart;
	}

	char* CRecord::GetDataEnd()
	{
		return dataEnd;
	}

	bool CRecord::DataCompressed()
	{
		return (record.flags&0x00040000);
	}

};