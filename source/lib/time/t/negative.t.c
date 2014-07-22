#include "time64.h"
#include "t/tap.h"
#include <stdio.h>

int main(void) {
    struct TM date;
    Time64_T time;

    time = -10;
    is_not_null( gmtime64_r(&time, &date), "gmtime / Wed Dec 31 23:59:50 1969" );
    tm_ok(&date,
          1969 - 1900, 11, 31,
          23, 59, 50);


    time = -0x00EFFFFFFFFFFFFFLL;
    is_not_null( gmtime64_r(&time, &date), "gmtime / -2140698894-01-13 14:56:01" );
    tm_ok(&date,
          -2140698894 - 1900, 0, 13,
          14, 56, 01);


    /* Try something that results in a negative tm_year */
    time = -4298423296LL;
    is_not_null( gmtime64_r(&time, &date), "Tue Oct 15 17:31:44 1833" );
    tm_ok(&date,
          1833 - 1900, 9, 15,
          17, 31, 44);

    /* It is safe to assume that the year and month
       will be the same, even with a julian/gregorian shift */
    is_not_null( localtime64_r(&time, &date), "localtime" );
    is_Int64((Year)date.tm_year,   (Year)(1833 - 1900),    "localtm.year");
    is_int(date.tm_mon,         9,                      "       .mon");


    /* And now something that results in a negative real year */
    time = -68719476736LL;
    is_not_null( gmtime64_r(&time, &date), "Sun May 13 16:27:44 -208" );
    tm_ok(&date,
          -208 - 1900, 4, 13,
          16, 27, 44);

    is_not_null( localtime64_r(&time, &date), "localtime" );
    is_Int64((Year)date.tm_year,   (Year)(-208 - 1900),    "localtm.year");
    is_int(date.tm_mon,         4,                      "       .mon");

    done_testing();
    return 0;
}
