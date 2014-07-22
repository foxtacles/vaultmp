#include "time64.h"
#include <stdio.h>
#include "t/tap.h"

int main(void)
{
    struct TM gtime;
#ifdef USE_TM64
    Time64_T time = 0x7FFFFFFFFFFFFFFFLL;
    Year expected_year  = 292277026596LL;
    int  expected_month = 12;
    int  expected_mday  = 4;
#else
    Time64_T time = 0x00EFFFFFFFFFFFFFLL;
    Year expected_year = 2140702833LL;
    int  expected_month = 12;
    int  expected_mday  = 19;
#endif

    printf("# time: %lld\n", time);
    gmtime64_r(&time, &gtime);
    printf("# sizeof time_t: %ld\n", sizeof(time_t));
    printf("# sizeof long long: %ld\n", sizeof(Time64_T));
    printf("# sizeof tm.tm_year: %ld\n", sizeof(gtime.tm_year));
    printf("# %04lld.%02d.%02d %02d:%02d:%02d\n",
        (Year)(gtime.tm_year + 1900),
        gtime.tm_mon  + 1,
        gtime.tm_mday,
        
        gtime.tm_hour,
        gtime.tm_min,
        gtime.tm_sec
    );

    is_Int64( (Year)(gtime.tm_year + 1900), expected_year,  "gtime.tm_year" );
    is_int( gtime.tm_mon + 1,    expected_month, "gtime.tm_mon"  );
    is_int( gtime.tm_mday,       expected_mday,  "gmtime.tm_mday" ); 

    done_testing();
    return 0;
}
