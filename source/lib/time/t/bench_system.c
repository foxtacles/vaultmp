#include <time.h>

int main(void) {
    struct tm date;
    time_t time = 1221176626;  /* About 23:44 Sep 11, 2008 GMT */
    int i;

    for( i = 0; i < 1000000; i++ ) {
        gmtime_r(&time, &date);
    }

    return(0);
}
