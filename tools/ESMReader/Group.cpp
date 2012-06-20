#include "Group.h"

namespace ESMLoader
{

	CGroup::CGroup()
	{

	}

	CGroup::~CGroup()
	{

	}

	void CGroup::Load(char* d)
	{
		memcpy(&group,d,sizeof(Group));
		dataStart=d+sizeof(Group);
		dataEnd=d+sizeof(Group)+group.groupSize-sizeof(Group);
	}

	int CGroup::Size()
	{
		return group.groupSize;
	}



	void CGroup::Dump()
	{
		cout<<"Type:"<<group.type[0]<<group.type[1]<<group.type[2]<<group.type[3]<<endl;
		cout<<"Size:"<<group.groupSize<<endl;
		cout<<"Group type:"<<group.groupType<<endl;
		if(group.groupType==6)
		{
			cout<<"Group parent:"<<"0x"<<hex<<*((int*)group.label)<<dec<<endl;
		}
	}

	char* CGroup::GetDataStart()
	{
		return dataStart;
	}

	char* CGroup::GetDataEnd()
	{
		return dataEnd;
	}

};