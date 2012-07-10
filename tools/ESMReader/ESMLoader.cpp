#include "ESMLoader.h"

namespace ESMLoader
{
	vector<standardRecord> recordPointers;

	vector<RECORD_NPC> NPCList;
	vector<RECORD_WEAP> WeaponList;
	vector<RECORD_AMMO> AmmoList;
	
	int indent=0;

	int load_file_to_memory(const char *filename, char **result) 
	{ 
		int size = 0;
		FILE *f = fopen(filename, "rb");
		if (f == NULL) 
		{ 
			*result = NULL;
			return -1; // -1 means file opening fail 
		} 
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
		*result = (char *)malloc(size+1);
		if (size != fread(*result, sizeof(char), size, f)) 
		{ 
			free(*result);
			return -2; // -2 means file reading fail 
		} 
		fclose(f);
		(*result)[size] = 0;
		return size;
	}

	char* LoopSubRecord(char* ptr,int parent)
	{

		if(strncmp(ptr,"EDID",4)==0)
		{
			if(sizeof((recordPointers[parent]).EDID)<(*(unsigned short*)(ptr+4)))
				cout<<"Editor ID overflow!"<<endl;
			else
				strncpy((recordPointers[parent]).EDID,ptr+6,*(unsigned short*)(ptr+4));
		}

		if(strncmp(ptr,"FULL",4)==0)
		{
			if(sizeof((recordPointers[parent]).FULL)<(*(unsigned short*)(ptr+4)))
				cout<<"Full Name overflow!("<<*(unsigned short*)(ptr+4)<<")"<<endl;
			else
				strncpy((recordPointers[parent]).FULL,ptr+6,*(unsigned short*)(ptr+4));
		}

		if(strncmp(ptr,"XCLC",4)==0)
			memcpy((recordPointers[parent]).XCLC,ptr+6,8);

		if(strncmp(ptr,"MNAM",4)==0)
			memcpy(&((recordPointers[parent]).MNAM),ptr+6,16);

		if(strncmp(ptr,"ONAM",4)==0)
			memcpy(&((recordPointers[parent]).ONAM),ptr+6,(sizeof(float)*3));

		return ptr+*(unsigned short*)(ptr+4)+6;
	}

	char* LoopRECORD(char* pt,int deep,int pparent)
	{
		standardRecord tmp;
		tmp.EDID[0]=0;
		tmp.FULL[0]=0;
		recordPointers.push_back(tmp);

		


		int current=recordPointers.size()-1;
		(recordPointers[current]).type=RECORD_RECORD;
		(recordPointers[current]).groupType=-1;
		(recordPointers[current]).parent=pparent;
		(recordPointers[current]).start=pt;

		(recordPointers[current]).EDID[0]=0;
		(recordPointers[current]).FULL[0]=0;
		(recordPointers[current]).formID=*(int*)(pt+12);

		memset(&(recordPointers[current]).MNAM,0,sizeof((recordPointers[current]).MNAM));

		(recordPointers[current]).end=(recordPointers[current]).start+(*(int*)(pt+4))+24;

		//Check if compressed or not
		int flags=*(int*)((recordPointers[current]).start+8);
		if(flags&0x00040000)
		{
			//Compressed! Let's decompress and parse it....
			static char buffer[1*1000*1000];	//1MB should be fine!
			int sizeDecompressed=*(int*)((recordPointers[current]).start+24);
			int compressedLength=(*(int*)(pt+4))-4;
			if(sizeDecompressed>sizeof(buffer))
				cout<<"Error, buffer too short"<<endl;
			else
			{
				char* ppp=buffer;
				char* pppEnd=buffer+sizeDecompressed;

				int sizeOfBuffer=sizeof(buffer);

				uncompress((Bytef *)buffer,(uLongf *)&sizeOfBuffer,(Bytef *)(recordPointers[current]).start+24+4,(uLongf)compressedLength);

				while(ppp<pppEnd)
				{
					if(strncmp("OFST",ppp,4)==0)
						break;
					ppp=LoopSubRecord(ppp,current);
				}
			}
		}
		else
		{
		
			char* ppp=(recordPointers[current]).start+24;
			{
				while(ppp<(recordPointers[current]).end)
				{
					if(strncmp("OFST",ppp,4)==0)
						break;
					ppp=LoopSubRecord(ppp,current);
				}
			}

		}

		return (recordPointers[current]).end;
	}

	char* LoopGRUP(char* pt,int deep,int pparent)
	{
		standardRecord tmp;
		recordPointers.push_back(tmp);

		int current=recordPointers.size()-1;

		(recordPointers[current]).parent=pparent;

		(recordPointers[current]).groupType=*(int*)(pt+12);

		(recordPointers[current]).start=pt;
		if(pt[0]=='G'&&pt[1]=='R'&&pt[2]=='U'&&pt[3]=='P')
		{
			(recordPointers[current]).end=(recordPointers[current]).start+(*(int*)(pt+4));
			deep=0;
			(recordPointers[current]).type=RECORD_GROUP;
		}
		else
		{
			cout<<"ERROR!!!"<<endl;
		}

		{
			char* ppp=(recordPointers[current]).start+24;

			{
				//Record!
				while(ppp<(recordPointers[current]).end)
				{
					if(ppp[0]=='G'&&ppp[1]=='R'&&ppp[2]=='U'&&ppp[3]=='P')
					{
						//return sr->end-sr->start;
						ppp=LoopGRUP(ppp,1,current);
					}
					else
						ppp=LoopRECORD(ppp,1,current);

				}
			}
		}
		return (recordPointers[current]).end;
	}

	void ParseLoadedData()
	{
		for(int i=0;i<recordPointers.size();i++)
		{
			char* type=recordPointers[i].start;

			if(strncmp(type,"NPC_",4)==0||strncmp(type,"LVLN",4)==0)
			{
				RECORD_NPC tmp;
				strcpy(tmp.editorID,recordPointers[i].EDID);
				strcpy(tmp.fullName,recordPointers[i].FULL);
				tmp.formID=recordPointers[i].formID;

				NPCList.push_back(tmp);
			}
			
			if(strncmp(type,"WEAP",4)==0)
			{
				RECORD_WEAP tmp;
				strcpy(tmp.editorID,recordPointers[i].EDID);
				strcpy(tmp.fullName,recordPointers[i].FULL);
				tmp.formID=recordPointers[i].formID;

				WeaponList.push_back(tmp);
			}

			if(strncmp(type,"AMMO",4)==0)
			{
				RECORD_AMMO tmp;
				strcpy(tmp.editorID,recordPointers[i].EDID);
				strcpy(tmp.fullName,recordPointers[i].FULL);
				tmp.formID=recordPointers[i].formID;

				AmmoList.push_back(tmp);
			}

			if(strncmp(type,"WRLD",4)==0)
			{
				//If it's a wrld record, next element is a group containing all his the cells :)
				int k=i+1;
				char* start=recordPointers[k].start;
				char* end=recordPointers[k].end;

				//Looking for world childs

				for(int j=k+1;j<recordPointers.size()&&recordPointers[j].end<end;j++)
				{
					if(recordPointers[j].type==RECORD_RECORD)
					{
						char *r=recordPointers[j].start;
						if(strncmp(r,"CELL",4)==0)
						{
						
						}
					}
				}

			}
		}
	}

	void Load(char* f)
	{
		vector<standardRecord> emptyVector;

		//Load file in memory
		int size=0;
		char *ptr=0;
		char* cursor=0;
		size=load_file_to_memory(f,&ptr);
		cursor=ptr;

		if(size>0&&ptr!=0)
		{
			cursor=ptr+24+(*(int*)(ptr+4));
			while(cursor<(ptr+size))
				cursor=LoopGRUP(cursor,0,0);
		}

		ParseLoadedData();

		delete ptr;

		//Trick to clear used ram by the vector
		recordPointers.swap(emptyVector);
		recordPointers.clear();
	}

};