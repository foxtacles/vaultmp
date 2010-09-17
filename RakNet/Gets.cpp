#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

char * Gets ( char * str, int num )
{
	fgets(str, num, stdin);
	if (str[0]=='\n' || str[0]=='\r')
		str[0]=0;
	return str;
}

#ifdef __cplusplus
}
#endif
