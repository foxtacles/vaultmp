#include "time64.h"

int main(void) {
    struct TM date;
    Time64_T time = 1221176626;  /* About 23:44 Sep 11, 2008 GMT */
    int i;

    for( i = 0; i < 1000000; i++ ) {
        gmtime64_r(&time, &date);
    }

    return(0);
}
