#include <stdio.h>
#include <time.h>
#include "time64.h"

int main(void)
{
    struct TM d;
    Time64_T time = 1202380093LL;

    d.tm_year = 90;

    while(d.tm_year < 800) {
        gmtime64_r(&time, &d);
        printf("%lld %d %d %d %d %d %lld %d %d %d\n",
                time,
                d.tm_sec,
                d.tm_min,
                d.tm_hour,
                d.tm_mday,
                d.tm_mon,
                (Year)d.tm_year,
                d.tm_wday,
                d.tm_yday,
                d.tm_isdst
        );
        
        time += 60 * 60 * 11 + 1;
    }
    return(0);
}
