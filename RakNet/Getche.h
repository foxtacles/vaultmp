#if defined(_XBOX) || defined(_X360)

#elif defined(_WIN32) && !defined(_XBOX) && !defined(X360)
#include <conio.h> /* getche() */
#elif defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3)
#else
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
char getche();
#endif 
