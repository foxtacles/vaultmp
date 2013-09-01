#include "pawn.hpp"

#include <cstring>
#include <climits>

int PAWN::amx_StrPack(cell *dest,cell *source,int len,int offs)
{
  int i;

  if ((ucell)*source>UNPACKEDMAX && offs%sizeof(cell)==0) {
    /* source string is already packed and the destination is cell-aligned */
    unsigned char* pdest=(unsigned char*)dest+offs;
    i=(len+sizeof(cell)-1)/sizeof(cell);
    memmove(pdest,source,i*sizeof(cell));
    /* zero-terminate */
    #if BYTE_ORDER==BIG_ENDIAN
      pdest+=len;
      for (i=len; i==len || i%sizeof(cell)!=0; i++)
        *pdest++='\0';
    #else
      i=(len/sizeof(cell))*sizeof(cell);
      pdest+=i;
      len=(len==i) ? sizeof(cell) : sizeof(cell)-(len-i);
      assert(len>0 && len<=sizeof(cell));
      for (i=0; i<len; i++)
        *pdest++='\0';
    #endif
  } else if ((ucell)*source>UNPACKEDMAX) {
    /* source string is packed, destination is not aligned */
    cell mask,c;
    dest+=offs/sizeof(cell);    /* increment whole number of cells */
    offs%=sizeof(cell);         /* get remainder */
    mask=(~(ucell)0) >> (offs*CHARBITS);
    c=*dest & ~mask;
    for (i=0; i<len+offs+1; i+=sizeof(cell)) {
      *dest=c | ((*source >> (offs*CHARBITS)) & mask);
      c=(*source << ((sizeof(cell)-offs)*CHARBITS)) & ~mask;
      dest++;
      source++;
    } /* for */
    /* set the zero byte in the last cell */
    mask=(~(ucell)0) >> (((offs+len)%sizeof(cell))*CHARBITS);
    *(dest-1) &= ~mask;
  } else {
    /* source string is unpacked: pack string, from top-down */
    cell c=0;
    if (offs!=0) {
      /* get the last cell in "dest" and mask of the characters that must be changed */
      cell mask;
      dest+=offs/sizeof(cell);  /* increment whole number of cells */
      offs%=sizeof(cell);       /* get remainder */
      mask=(~(ucell)0) >> (offs*CHARBITS);
      c=(*dest & ~mask) >> ((sizeof(cell)-offs)*CHARBITS);
    } /* if */
    /* for proper alignement, add the offset to both the starting and the ending
     * criterion (so that the number of iterations stays the same)
     */
    for (i=offs; i<len+offs; i++) {
      c=(c<<CHARBITS) | (*source++ & 0xff);
      if (i%sizeof(cell)==sizeof(cell)-1) {
        *dest++=c;
        c=0;
      } /* if */
    } /* for */
    if (i%sizeof(cell) != 0)    /* store remaining packed characters */
      *dest=c << (sizeof(cell)-i%sizeof(cell))*CHARBITS;
    else
      *dest=0;                  /* store full cell of zeros */
  } /* if */
  return AMX_ERR_NONE;
}

int PAWN::amx_StrUnpack(cell *dest,cell *source,int len)
{
  /* len excludes the terminating '\0' byte */
  if ((ucell)*source>UNPACKEDMAX) {
    /* unpack string, from bottom up (so string can be unpacked in place) */
    cell c;
    int i;
    for (i=len-1; i>=0; i--) {
      c=source[i/sizeof(cell)] >> (sizeof(cell)-i%sizeof(cell)-1)*CHARBITS;
      dest[i]=c & UCHAR_MAX;
    } /* for */
    dest[len]=0;        /* zero-terminate */
  } else {
    /* source string is already unpacked */
    while (len-->0)
      *dest++=*source++;
    *dest=0;
  } /* if */
  return AMX_ERR_NONE;
}
