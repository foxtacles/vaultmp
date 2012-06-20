/*

Sample demonstrating how to load the esm/esp and how to parse the records

*/

#include "ESMLoader.h"

using namespace ESMLoader;

int main()
{
	Load("C:\\Program Files (x86)\\Bethesda Softworks\\Fallout New Vegas\\Data\\FalloutNV.esm");

	for(int i=0;i<records.size();i++)
	{
		char* type=records[i]->GetType();
		if(strncmp(type,"CELL",4)==0)
		{
			char *editorID;
			int formID;
			int x;
			int y;

			ParseCELL(records[i],&formID,&editorID,&x,&y);

			if(editorID[0]!=0)
				cout<<editorID<<" , ";
			else
				cout<<"Wilderness , ";
			cout<<"0x"<<hex<<formID<<dec<<" , "<<x<<" , "<<y<<endl;
		}
	}

	Unload();

	//cin.get();
	return 0;
}