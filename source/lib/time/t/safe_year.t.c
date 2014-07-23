/* Test the internal safe_year() function which does the critical mapping
   to make the localtime trick work
*/

#include "time64.c"
#include "t/tap.h"

static void year_to_TM(const Year year, struct TM *date) {
    Time64_T time;

    date->tm_year = year - 1900;
    date->tm_mon  = 0;
    date->tm_mday = 1;
    date->tm_hour = 0;
    date->tm_min  = 0;
    date->tm_sec  = 0;

    time = timegm64(date);
    gmtime64_r(&time, date);
}


static void year_to_tm(const Year year, struct tm *ret) {
    Time64_T time;
    struct TM date;

    date.tm_year = year - 1900;
    date.tm_mon  = 0;
    date.tm_mday = 1;
    date.tm_hour = 0;
    date.tm_min  = 0;
    date.tm_sec  = 0;

    time = timegm64(&date);
    gmtime64_r(&time, &date);
    copy_TM64_to_tm(&date, ret);
}


static void test_safe_year(Year orig_year) {
    int year = safe_year( orig_year );
    struct tm safe_tm;
    struct TM orig_tm;

    if( orig_year > 1970 && orig_year < 2038 ) {
        is_Int64( orig_year, (Year)year, "safe_year() returns same year if already safe" );
    }
    else if( orig_year <= 1970 ) {
        ok( year < 2000, "safe_year() < 2000 for year <= 1970" );
    }
    else if( orig_year >= 2038 ) {
        ok( year > 2000, "safe_year() > 2000 for year >= 2038" );
    }
    else {
        assert(0);
    }

    year_to_tm((Year)year, &safe_tm);
    year_to_TM(orig_year, &orig_tm);
    is_Int64( (Year)orig_tm.tm_year, (Year)(orig_year - 1900), "year_to_tm(orig)" );
    is_int( safe_tm.tm_year, year - 1900, "year_to_TM(safe)" );

    ok(1, "orig_year: %lld, safe_year: %d", orig_year, year);
    is_int( safe_tm.tm_wday, orig_tm.tm_wday,                      "  tm_wday" );
    is_int( IS_LEAP( year - 1900 ), IS_LEAP( orig_year - 1900 ),   "  ISLEAP()" );

    year--;
    orig_year--;
    year_to_tm((Year)year, &safe_tm);
    year_to_TM(orig_year, &orig_tm);
    is_int( safe_tm.tm_wday, orig_tm.tm_wday,                      "  previous tm_wday" );
    is_int( IS_LEAP( year - 1900 ), IS_LEAP( orig_year - 1900 ),   "  previous ISLEAP()" );

    year += 2;
    orig_year += 2;
    year_to_tm((Year)year, &safe_tm);
    year_to_TM(orig_year, &orig_tm);
    is_int( safe_tm.tm_wday, orig_tm.tm_wday,                           "  next tm_wday" );
    /* Don't care about the next leap status */
}

int main(void) {
    Year year;

    /* Boundry tests */
    test_safe_year((Year)1);
    test_safe_year((Year)0);
    test_safe_year((Year)-1);
    

    for( year = 1599;  year <= 2401;  year++ ) {
        test_safe_year(year);
    }

    for( year = -401;  year <= 401;  year++ ) {
        test_safe_year(year);
    }

    done_testing();

    return(0);
}
