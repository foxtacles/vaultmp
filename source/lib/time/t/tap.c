#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "time64.h"

int Test_Count = 0;


int diag(const char *message, ...) {
    va_list args;
    va_start(args, message);

    fprintf(stderr, "# ");
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n");

    va_end(args);

    return(0);
}


void skip_all(const char *reason) {
    printf("1..0 # Skip %s\n", reason);
    exit(0);
}


int do_test(const int test, const char *message, va_list args) {
    Test_Count++;

    printf("%s %d ", (test ? "ok" : "not ok"), Test_Count);
    vprintf(message, args);
    printf("\n");

    if( !test ) {
        diag("Failed test");
        diag(message, args);
    }

    return test;
}    


int ok(const int test, const char *message, ...) {
    va_list args;
    int pass;

    va_start(args, message);

    pass = do_test(test, message, args);

    va_end(args);

    return pass;
}


int is_int(const int have, const int want, const char *message, ...) {
    int test = (have == want);
    va_list args;
    va_start(args, message);

    do_test( test, message, args );

    if( !test ) {
        diag("have: %d", have);
        diag("want: %d", want);
    }

    va_end(args);

    return test;
}


int is_str(const char* have, const char* want, const char *message, ...) {
    int test = strcmp(have, want) == 0;
    va_list args;
    va_start(args, message);

    do_test( test, message, args );

    if( !test ) {
        diag("have: %s", have);
        diag("want: %s", want);
    }

    va_end(args);

    return test;
}


int is_Int64(const Int64 have, const Int64 want, const char *name, ...) {
    int test = (have == want);

    va_list args;
    va_start(args, name);

    do_test( test, name, args );

    if( !test ) {
        diag("have: %lld", have);
        diag("want: %lld", want);
    }

    va_end(args);

    return test;
}


int is_not_null(void *arg, const char *name) {
    return( ok( arg != NULL, name ) );
}


int tm_ok(const struct TM *have,
          const int year, const int mon, const int mday,
          const int hour, const int min, const int sec)
{
    int ok = 1;

    ok *= is_Int64((Year)have->tm_year, (Year)year,  "tm.year");
    ok *= is_int(have->tm_mon,  mon,   "   month");
    ok *= is_int(have->tm_mday, mday,  "   day");
    ok *= is_int(have->tm_hour, hour,  "   hour");
    ok *= is_int(have->tm_min,  min,   "   min");
    ok *= is_int(have->tm_sec,  sec,   "   sec");

    return ok;
}


int tm_is(const struct TM *have, const struct TM *want, const char *name)
{
    int pass = 1;

    pass *= ok( have != NULL, name );
    pass *= ok( want != NULL, name );
    pass *= is_Int64((Year)have->tm_year, (Year)want->tm_year,  "tm.year");
    pass *= is_int(have->tm_mon,  want->tm_mon,   "   month");
    pass *= is_int(have->tm_mday, want->tm_mday,  "   day");
    pass *= is_int(have->tm_hour, want->tm_hour,  "   hour");
    pass *= is_int(have->tm_min,  want->tm_min,   "   min");
    pass *= is_int(have->tm_sec,  want->tm_sec,   "   sec");

    return pass;
}


struct TM make_tm( int sec, int min, int hour, int day, int mon, Year year ) {
    struct TM date;

    date.tm_year = year - 1900;
    date.tm_mon  = mon - 1;
    date.tm_mday  = day;
    date.tm_hour = hour;
    date.tm_min  = min;
    date.tm_sec  = sec;

    return date;
}
    

void done_testing(void) {
    printf("1..%d\n", Test_Count);
}
