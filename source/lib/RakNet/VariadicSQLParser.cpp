#include "VariadicSQLParser.h"
#include "BitStream.h"
#include <stdarg.h>

using namespace VariadicSQLParser;

struct TypeMapping
{
	char inputType;
	const char *type;
};
const int NUM_TYPE_MAPPINGS=7;
TypeMapping typeMappings[NUM_TYPE_MAPPINGS] =
{
	{'i', "int"},
	{'d', "int"},
	{'s', "text"},
	{'b', "bool"},
	{'f', "numeric"},
	{'g', "double precision"},
	{'a', "bytea"},
};
unsigned int GetTypeMappingIndex(char c)
{
	unsigned int i;
	for (i=0; i < (unsigned int) NUM_TYPE_MAPPINGS; i++ )
		if (typeMappings[i].inputType==c)
			return i;
	return (unsigned int)-1;
}
const char* VariadicSQLParser::GetTypeMappingAtIndex(int i)
{
	return typeMappings[i].type;
}
void VariadicSQLParser::GetTypeMappingIndices( const char *format, DataStructures::List<IndexAndType> &indices )
{
	bool previousCharWasPercentSign;
	unsigned int i;
	unsigned int typeMappingIndex;
	indices.Clear(false, _FILE_AND_LINE_);
	unsigned int len = (unsigned int) strlen(format);
	previousCharWasPercentSign=false;
	for (i=0; i < len; i++)
	{
		if (previousCharWasPercentSign==true )
		{
			typeMappingIndex = GetTypeMappingIndex(format[i]);
			if (typeMappingIndex!=(unsigned int) -1)
			{
				IndexAndType iat;
				iat.strIndex=i-1;
				iat.typeMappingIndex=typeMappingIndex;
				indices.Insert(iat, _FILE_AND_LINE_ );
			}
		}

		previousCharWasPercentSign=format[i]=='%';
	}
}
void VariadicSQLParser::ExtractArguments( va_list argptr, const DataStructures::List<IndexAndType> &indices, char ***argumentBinary, int **argumentLengths )
{
	if (indices.Size()==0)
		return;

	unsigned int i;
	*argumentBinary=RakNet::OP_NEW_ARRAY<char *>(indices.Size(), _FILE_AND_LINE_);
	*argumentLengths=RakNet::OP_NEW_ARRAY<int>(indices.Size(), _FILE_AND_LINE_);

	char **paramData=*argumentBinary;
	int *paramLength=*argumentLengths;

	int variadicArgIndex;
	for (variadicArgIndex=0, i=0; i < indices.Size(); i++, variadicArgIndex++)
	{
		switch (typeMappings[indices[i].typeMappingIndex].inputType)
		{
		case 'i':
		case 'd':
			{
				int val = va_arg( argptr, int );
				paramLength[i]=sizeof(val);
				paramData[i]=(char*) rakMalloc_Ex(paramLength[i], _FILE_AND_LINE_);
				memcpy(paramData[i], &val, paramLength[i]);
				if (RakNet::BitStream::IsNetworkOrder()==false) RakNet::BitStream::ReverseBytesInPlace((unsigned char*) paramData[i], paramLength[i]);
			}
			break;
		case 's':
			{
				char* val = va_arg( argptr, char* );
				paramLength[i]=(int) strlen(val);
				paramData[i]=(char*) rakMalloc_Ex(paramLength[i]+1, _FILE_AND_LINE_);
				memcpy(paramData[i], val, paramLength[i]+1);
			}
			break;
		case 'b':
			{
				bool val = (va_arg( argptr, int )!=0);
				paramLength[i]=sizeof(val);
				paramData[i]=(char*) rakMalloc_Ex(paramLength[i], _FILE_AND_LINE_);
				memcpy(paramData[i], &val, paramLength[i]);
				if (RakNet::BitStream::IsNetworkOrder()==false) RakNet::BitStream::ReverseBytesInPlace((unsigned char*) paramData[i], paramLength[i]);
			}
			break;
			/*
		case 'f':
			{
				// On MSVC at least, this only works with double as the 2nd param
				float val = (float) va_arg( argptr, double );
				//float val = va_arg( argptr, float );
				paramLength[i]=sizeof(val);
				paramData[i]=(char*) rakMalloc_Ex(paramLength[i], _FILE_AND_LINE_);
				memcpy(paramData[i], &val, paramLength[i]);
				if (RakNet::BitStream::IsNetworkOrder()==false) RakNet::BitStream::ReverseBytesInPlace((unsigned char*) paramData[i], paramLength[i]);
			}
			break;
			*/
		// On MSVC at least, this only works with double as the 2nd param
		case 'f':
		case 'g':
			{
				double val = va_arg( argptr, double );
				paramLength[i]=sizeof(val);
				paramData[i]=(char*) rakMalloc_Ex(paramLength[i], _FILE_AND_LINE_);
				memcpy(paramData[i], &val, paramLength[i]);
				if (RakNet::BitStream::IsNetworkOrder()==false) RakNet::BitStream::ReverseBytesInPlace((unsigned char*) paramData[i], paramLength[i]);
			}
			break;
		case 'a':
			{
				char* val = va_arg( argptr, char* );
				paramLength[i]=va_arg( argptr, unsigned int );
				paramData[i]=(char*) rakMalloc_Ex(paramLength[i], _FILE_AND_LINE_);
				memcpy(paramData[i], val, paramLength[i]);
			}
			break;
		}
	}

}
void VariadicSQLParser::FreeArguments(const DataStructures::List<IndexAndType> &indices, char **argumentBinary, int *argumentLengths)
{
	if (indices.Size()==0)
		return;

	unsigned int i;
	for (i=0; i < indices.Size(); i++)
		rakFree_Ex(argumentBinary[i],_FILE_AND_LINE_);
	RakNet::OP_DELETE_ARRAY(argumentBinary,_FILE_AND_LINE_);
	RakNet::OP_DELETE_ARRAY(argumentLengths,_FILE_AND_LINE_);
}
