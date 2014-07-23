#include "time64.h"
#include "t/tap.h"
#include <stdio.h>

/* Test that the non-reentrant functions share the same memory */
void test_non_reentrant(void) {
    Time64_T time;
    struct TM *have_date;

    time = 60 * 60 * 24 * 1;
    have_date = localtime64(&time);

    is_Int64( (Year)have_date->tm_year, 70LL, "localtime64()" );

    time = 60 * 60 * 24 * 50;
    gmtime64(&time);
    tm_ok( have_date, 70, 1, 20, 0, 0, 0 );
}

int main(void) {
    Time64_T time;
    struct TM want_date;
    struct TM *have_date;
    char name[100];

    /* Compare the results of the reentrant and non-reentrant functions */
    for( time = -6000000000LL;  time < 6000000000LL; time += 712359 ) {
        localtime64_r(&time, &want_date);
        have_date = localtime64(&time);
        sprintf(name, "localtime64(%lld)", time);
        tm_is( have_date, &want_date, name );

        gmtime64_r(&time, &want_date);
        have_date = gmtime64(&time);
        sprintf(name, "gmtime64(%lld)", time);
        tm_is( have_date, &want_date, name );
    }

    test_non_reentrant();

    done_testing();

    return 0;
}
