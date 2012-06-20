#include "Common.h"

namespace ESMLoader
{

	struct Field
	{
		char type[4];
		unsigned short dataSize;
	};

	class CField
	{
		Field field;

		char* dataStart;
		char* dataEnd;
	public:
		CField();
		~CField();

		void Load(char*);
		int Size();

		void Dump();

		char* GetType();

		char* GetDataStart();
		char* GetDataEnd();
	};

}