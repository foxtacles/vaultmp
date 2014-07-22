#include "time64.h"
#include "t/tap.h"

int main(void) {
    Time64_T time = 12345678987654321LL;
    char* ctime_ret1;
    char* ctime_ret2;
    char ctime_buf1[35];

    /* The ctime() function shall convert the time pointed to by clock, representing time
       in seconds since the Epoch, to local time in the form of a string. It shall be equivalent
       to: asctime(localtime(clock)) (ISO)
    */
    is_str( ctime64(&time), asctime64(localtime64(&time)), "ctime64()" );

    /* The asctime(), ctime(), gmtime(), and localtime() functions shall return values in 
       one of two static objects: a broken-down time structure and an array of char. Execution
       of any of the functions may overwrite the information returned in either of these
       objects by any of the other functions. (POSIX)
    */
    ctime_ret1 = ctime64(&time);
    time++;
    ctime_ret2 = ctime64(&time);
    is_str( ctime_ret1, ctime_ret2, "  return is static"   );

    /* The ctime_r() function shall convert the calendar time pointed to by clock to local
       time in exactly the same form as ctime() and put the string into the array pointed
       to by buf (which shall be at least 26 bytes in size) and return buf.
       (We need 35 bytes) (POSIX)
    */
    ctime_ret1 = ctime64_r(&time, ctime_buf1);
    is_str( ctime_ret1, ctime_buf1,                     "ctime64_r()" );
    is_str( ctime_ret1, asctime64(localtime64(&time)),  "  right time" );

    /* Unlike ctime(), the thread-safe version ctime_r() is not required to set tzname. (ISO)
       (I don't understand)
     */

    /* Upon successful completion, ctime_r() shall return a pointer to the string pointed to
       by buf. When an error is encountered, a null pointer shall be returned. (POSIX)
       (Can't think of any way to make it error)
    */

    done_testing();

    return 0;
}
