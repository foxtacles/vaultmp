#if defined(_XBOX) || defined(_X360)

#elif defined(_WIN32) && !defined(_XBOX) && !defined(X360)
#include <conio.h> /* getche() */
#elif defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3)

#else

#include "Getche.h"

char getche()
{


  struct termios oldt,
                 newt;
  char            ch;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;

} 
#endif
