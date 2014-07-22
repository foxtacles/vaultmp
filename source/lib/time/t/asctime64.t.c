#include "time64.h"
#include "t/tap.h"

int main(void) {
    struct TM date;
    char * asctime_return1;
    char * asctime_return2;
    char asctime_r_buf1[35];
    char asctime_r_buf2[35];
    char * asctime_r_ret1;
    char * asctime_r_ret2;

    date = make_tm( 1, 2, 3, 4, 5, 234567LL );
    date.tm_wday = 1;   /* It's a Monday */

    /* The asctime() function shall convert the broken-down time in the structure pointed
       to by timeptr into a string in the form: (ISO)
       Sun Sep 16 01:03:52 1973\n\0
    */
    asctime_return1 = asctime64(&date);
    is_str( asctime_return1, "Mon May  4 03:02:01 234567\n", "asctime64()" );

    /* The asctime(), ctime(), gmtime(), and localtime() functions shall return values in
       one of two static objects: a broken-down time structure and an array of type char.
       Execution of any of the functions may overwrite the information returned in either
       of these objects by any of the other functions. (POSIX)
    */
    date.tm_sec = 42;
    asctime_return2 = asctime64(&date);
    is_str( asctime_return2, "Mon May  4 03:02:42 234567\n", "asctime64() again" );
    is_str( asctime_return1, asctime_return2, "  return is static" );

    /* The asctime_r() function shall convert the broken-down time in the structure pointed
       to by tm into a string (of the same form as that returned by asctime()) that is placed
       in the user-supplied buffer pointed to by buf (which shall contain at least 26 bytes)
       and then return buf. (POSIX)
       It'll have to be 35 bytes to hold the year -292,000,000,000
    */
    asctime_r_ret1 = asctime64_r( &date, asctime_r_buf1 );
    is_str( asctime_r_buf1,  "Mon May  4 03:02:42 234567\n", "asctime64_r()" );
    is_str( asctime_r_buf1,  asctime_r_ret1,                 "  return value same" );

    date.tm_hour = 5;
    asctime_r_ret2 = asctime64_r( &date, asctime_r_buf2 );
    is_str( asctime_r_buf2,  "Mon May  4 05:02:42 234567\n", "asctime64_r() again" );
    is_str( asctime_r_ret1,  "Mon May  4 03:02:42 234567\n", "  return not static" );

    /* Upon successful completion, asctime() shall return a pointer to the string.
       If the function is unsuccessful, it shall return NULL. (ISO)
    */
    date.tm_wday = -1;
    ok( asctime64(&date) == NULL, "asctime64() fail" );

    /* Upon successful completion, asctime_r() shall return a pointer to a character string
       containing the date and time. This string is pointed to by the argument buf. If the
       function is unsuccessful, it shall return NULL. (POSIX)
    */
    ok( asctime64_r(&date, asctime_r_buf2) == NULL,         "asctime64_r() fail" );
    is_str( asctime_r_buf2,  "Mon May  4 05:02:42 234567\n", "  buffer not overwritten" );

    /* No errors are defined. (ISO) */

    done_testing();

    return 0;
}
