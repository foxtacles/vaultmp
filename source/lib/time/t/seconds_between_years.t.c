#include "time64.c"
#include "t/tap.h"

#define SECS_PER_DAY     (60LL * 60LL * 24LL)
#define SECS_PER_YEAR    (SECS_PER_DAY * 365LL)
#define SECS_PER_LEAP    (SECS_PER_DAY * 366LL)

int main(void) {
    is_Int64( seconds_between_years( (Year)2000, (Year)2000 ),
              0LL,
              "2000 - 2000"
    );

    is_Int64( seconds_between_years( (Year)2001, (Year)2000 ),
              SECS_PER_LEAP,
              "2001 - 2000"
    );

    is_Int64( seconds_between_years( (Year)2004, (Year)2000 ),
              SECS_PER_YEAR * 3 + SECS_PER_LEAP,
              "2004 - 2000"
    );

    is_Int64( seconds_between_years( (Year)2005, (Year)2000 ),
              SECS_PER_YEAR * 3 + SECS_PER_LEAP * 2,
              "2005 - 2000"
    );

    is_Int64( seconds_between_years( (Year)2400, (Year)2000 ),
              SECS_PER_YEAR * 303 + SECS_PER_LEAP * 97,
              "2400 - 2000"
    );

    is_Int64( seconds_between_years( (Year)2401, (Year)2000 ),
              SECS_PER_YEAR * 303 + SECS_PER_LEAP * 98,
              "2401 - 2000"
    );

    done_testing();
    return 0;
}
