#include "time64.h"
#include "t/tap.h"

int mktime64_ok(Time64_T time) {
    struct TM date;

    localtime64_r(&time, &date);
    return is_Int64( mktime64(&date), time, "mktime64(%lld)", time );
}

int main(void) {
    struct TM date;
    Time64_T time;

    /* Some basic round trip mktime64 tests */
    mktime64_ok((Time64_T)0);
    mktime64_ok((Time64_T)1);
    mktime64_ok((Time64_T)-1);
    mktime64_ok((Time64_T)60*60*24*15);
    mktime64_ok((Time64_T)60*60*24*15);
    mktime64_ok((Time64_T)60*60*24*365*143);
    mktime64_ok((Time64_T)-60*60*24*365*143);
    mktime64_ok((Time64_T)60*60*24*365*433);
    mktime64_ok((Time64_T)-60*60*24*365*433);
    mktime64_ok((Time64_T)-2147483647);
    mktime64_ok((Time64_T)2147483647);


    /* Test timelocal64 alias to mktime64 */
    time = 12345;
    localtime64_r(&time, &date);
    is_Int64( mktime64(&date), timelocal64(&date), "timelocal64 alias" );


    /* Test that mktime64 accepts and corrects out of bound dates */
    /* The original values of the tm_wday and tm_yday components of the
     * structure are ignored, and the original values of the other components
     * are not restricted to the ranges described in <time.h>.
     * http://www.opengroup.org/onlinepubs/009695399/functions/mktime.html
     */
    date.tm_mday = 35;

    time = 12344678900LL;     /* Thu Mar  9 21:28:20 2361 */
    localtime64_r(&time, &date);

    /* Feb 37 == Mar 9 */
    date.tm_mon  = 1;
    date.tm_mday = 37;
    date.tm_wday = 9;           /* deliberately wrong week day */
    date.tm_yday = 487;         /* and wrong year day */

    /* Upon successful completion, the values of the tm_wday and tm_yday
     * components of the structure shall be set appropriately, and the other
     * components are set to represent the specified time since the Epoch,
     * but with their values forced to the ranges indicated in the <time.h>
     * entry; the final value of tm_mday shall not be set until tm_mon and
     * tm_year are determined.
     * http://www.opengroup.org/onlinepubs/009695399/functions/mktime.html
     */
    is_Int64( mktime64(&date), time, "mktime64(%lld)", time );
    is_int( date.tm_mon,  2, "tm_mon corrected" );
    is_int( date.tm_mday, 9, "tm_mday corrected" );
    is_int( date.tm_yday, 67,"tm_yday corrected" );
    is_int( date.tm_wday, 4, "tm_wday corrected" );

    done_testing();
    return 0;
}
