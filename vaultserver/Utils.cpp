#include "Utils.h"

void Utils::timestamp()
{
    time_t ltime;
    ltime = time(NULL);
    char t[32];
    sprintf(t, "[%s", asctime(localtime(&ltime)));
    char* newline = strchr(t, '\n');
    *newline = ']';
    strcat(t, " ");
    printf(t);
}
