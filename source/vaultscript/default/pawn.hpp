#ifndef PAWN_H
#define PAWN_H

#define CHARBITS (8*sizeof(char))
#define UNPACKEDMAX (((cell)1 << (sizeof(cell)-1)*8) - 1)
#define AMX_ERR_NONE 0

namespace PAWN
{
	typedef signed long long cell;
	typedef unsigned long long ucell;

	int amx_StrPack(cell *dest,cell *source,int len,int offs);
	int amx_StrUnpack(cell *dest,cell *source,int len);
}

#endif
