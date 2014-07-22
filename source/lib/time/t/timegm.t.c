#include <stdlib.h>
#include "time64.h"
#include "t/tap.h"

int main(void) {
    struct TM date;
    Time64_T time = 0;

    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(0)" );

    time = 60*60*16;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(60*60*16)" );

    time = 60*60*24*364 + 60*60*23;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(60*60*24*364 + 60*60*23)" );

    time = -1;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(-1)" );

    time = -60*60*24*364 -60*60*23;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(-60*60*24*364 - 60*60*23)" );

    time = 1230774010;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(1230774010)" );

    time = 1262296406;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(1262296406)" );

    time = 1325380799;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(1325380799)" );

    time = 1356982397;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(1356982397)" );

    time = 4294967296LL; /* 2**32 */
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(2**32)" );

    time = -31536000;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(-31536000)" );

    /* Negative leap year */
    time = -302216279;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(-302216279)" );

    time = 0x000FFFFFFFFFFFFFLL;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(2**52)" );

    time = -0x000FFFFFFFFFFFFFLL;
    gmtime64_r(&time, &date);
    is_Int64( timegm64(&date), time, "timegm64(-2**52)" );

    done_testing();
    return(0);
}
