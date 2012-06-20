#include "ESMLoader.h"

namespace ESMLoader
{

	vector<CRecord*> records;

	void ParseGroup(char** cursor,CGroup* grp);

	int ae_load_file_to_memory(const char *filename, char **result) 
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

	void ParseSubRecord(CField* f,CRecord *r)
	{
		char* tmp=f->GetType();
		if(strncmp(tmp,"EDID",4)==0)
		{
			strcpy(r->editorID,f->GetDataStart());
		}
		else if(strncmp(tmp,"NAME",4)==0)
		{
			memcpy(&r->paramFormID,f->GetDataStart(),4);
		}
		else if(strncmp(tmp,"DATA",4)==0&&(strncmp(r->GetType(),"REFR",4)==0||strncmp(r->GetType(),"ACHR",4)==0))
		{
			//It's a refr, save the coords
			memcpy(r->pos,f->GetDataStart(),12);
		}
		else if(strncmp(tmp,"XCLC",4)==0)
		{
			//It's a refr, save the coords
			memcpy(r->grid,f->GetDataStart(),8);
		}
	}

	void ParseRecord(char** cursor,CRecord* r)
	{
			while((*cursor)<r->GetDataEnd())
			{
				//Check if is a record
				if((*cursor)[0]=='W'&&(*cursor)[1]=='R'&&(*cursor)[2]=='L'&&(*cursor)[3]=='D')
				{
					CRecord p;
					p.Load(*cursor);
					(*cursor)+=sizeof(Record);
	#ifdef DUMP
					p.Dump();
	#endif
					ParseRecord(cursor,&p);
				}
				else
				{
					CField f;
					f.Load(*cursor);
	#ifdef DUMP
					f.Dump();
	#endif
					(*cursor)+=f.Size();

					ParseSubRecord(&f,r);
				}
			}
	}

	void ParseGroup(char** cursor,CGroup* grp)
	{
		while((*cursor)<grp->GetDataEnd())
		{
			//If is a group, don't read the records
			if(((*cursor)[0]=='G'&&(*cursor)[1]=='R'&&(*cursor)[2]=='U'&&(*cursor)[3]=='P'))
			{
				//cout<<"----GRUP Record"<<endl;
				//cin.get();
				CGroup p;
				p.Load(*cursor);
				(*cursor)+=sizeof(Group);
	#ifdef DUMP
				p.Dump();
	#endif
				ParseGroup(cursor,&p);
				//cout<<"----GRUP Record end"<<endl;
				//cin.get();
			}
			else
			{
				CRecord *r=new CRecord();
				r->Load(*cursor);
				(*cursor)+=sizeof(Record);
	#ifdef DUMP
				r->Dump();
	#endif
				if(r->DataCompressed()||strncmp("WRLD",r->GetType(),4)==0)
				{
					(*cursor)-=sizeof(Record);
					(*cursor)+=r->Size();
					//cout<<"Compressed data"<<endl;
					continue;
				}
				ParseRecord(cursor,r);
				records.push_back(r);
			}
		}
	}

	void Load(char* f)
	{
		//Load file in memory
		int size=0;
		char *ptr=0;
		char* cursor=0;
		size=ae_load_file_to_memory(f,&ptr);
		cursor=ptr;

		if(size>0&&ptr!=0)
		{
			//First Part of the file is a Record
			CRecord header;
			header.Load(ptr);
	#ifdef DUMP
			header.Dump();
	#endif
			cursor+=header.Size();
			while(cursor<(ptr+size))
			{
				CGroup grp;
				grp.Load(cursor);
	#ifdef DUMP
				grp.Dump();
	#endif
				cursor=grp.GetDataStart();
				ParseGroup(&cursor,&grp);
			}
		}

		delete ptr;
	}
	
	void Unload()
	{
		for(int i=0;i<records.size();i++)
		{
			delete records[i];
		}
	}

};