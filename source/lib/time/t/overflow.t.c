#include <errno.h>
#include "time64.h"
#include "t/tap.h"

int main(void) {
    Time64_T time = 0x0FFFFFFFFFFFFFFFLL;
    struct TM date;
    struct TM *error;

#ifndef EOVERFLOW
    skip_all( "EOVERFLOW not defined on this system" );
#else
    if( sizeof(date.tm_year) > 4 )
        skip_all( "tm_year is too large to overflow (yay!)" );

    /* Overflow */
    error = gmtime64_r(&time, &date);
    ok( error == NULL,          "gmtime64_r() returned null on overflow" );
    is_int( errno, EOVERFLOW,   "  errno set to EOVERFLOW" );

    error = localtime64_r(&time, &date);
    ok( error == NULL,          "localtime64_r() returned null on overflow" );
    is_int( errno, EOVERFLOW,   "  errno set to EOVERFLOW" );


    /* Underflow */
    time = -0x0FFFFFFFFFFFFFFFLL;

    error = gmtime64_r(&time, &date);
    ok( error == NULL,          "gmtime64_r() returned null on underflow" );
    is_int( errno, EOVERFLOW,   "  errno set to EOVERFLOW" );

    error = localtime64_r(&time, &date);
    ok( error == NULL,          "localtime64_r() returned null on underflow" );
    is_int( errno, EOVERFLOW,   "  errno set to EOVERFLOW" );


    done_testing();
#endif

    return 0;
}
 
