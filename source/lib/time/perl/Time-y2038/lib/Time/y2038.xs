#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <math.h>

#include "time64.h"

#define myPUSHi(int)   PUSHs(sv_2mortal(newSViv(int)));
#define myPUSHn(num)   PUSHs(sv_2mortal(newSVnv(num)));
#define myPUSHs(str)   PUSHs(sv_2mortal(str));


static const char * const dayname[] =
    {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char * const monname[] =
    {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


int about_eq(double left, double right, double epsilon) {
    double diff;

    if( left == right ) {
        return 1;
    }

    diff = fabs(left - right);

    if( diff < epsilon ) {
        return 1;
    }
    else {
        return 0;
    }
}


MODULE = Time::y2038                PACKAGE = Time::y2038
PROTOTYPES: ENABLE


void
gmtime(...)
    PROTOTYPE: ;$
    INIT:
        Time64_T when;
        struct TM *err;
        struct TM date;
    PPCODE:
        if( GIMME_V == G_VOID ) {
            warn("Useless use of gmtime() in void context");
            XSRETURN_EMPTY;
        }

        if( items == 0 ) {
            time_t small_when;
            time(&small_when);
            when = (Time64_T)small_when;
        }
        else {
            double when_float = SvNV(ST(0));
            when = (Time64_T)when_float;

            /* Check for Time64_T overflow */
            if( !about_eq((double)when, when_float, 1024.0) ) {
                warn("gmtime(%.0f) can not be represented", when_float);
                XSRETURN_EMPTY;
            }
        }

        err = gmtime64_r(&when, &date);

        if( err == NULL )
        {
            warn("gmtime(%.0f) can not be represented", (double)when);
            XSRETURN_EMPTY;
        }

        if( GIMME_V == G_ARRAY ) {
            EXTEND(SP, 9);
            myPUSHi(date.tm_sec);
            myPUSHi(date.tm_min);
            myPUSHi(date.tm_hour);
            myPUSHi(date.tm_mday);
            myPUSHi(date.tm_mon);
            myPUSHn((double)date.tm_year);
            myPUSHi(date.tm_wday);
            myPUSHi(date.tm_yday);
            myPUSHi(date.tm_isdst);
        }
        else {
            SV *tsv;
            /* XXX newSVpvf()'s %lld type is broken, so cheat with a double */
            double year = (double)date.tm_year + 1900;

            EXTEND(SP, 1);
            EXTEND_MORTAL(1);

            tsv = newSVpvf("%s %s %2d %02d:%02d:%02d %.0f",
                           dayname[date.tm_wday],
                           monname[date.tm_mon],
                           date.tm_mday,
                           date.tm_hour,
                           date.tm_min,
                           date.tm_sec,
                           year);
            myPUSHs(tsv);
        }


void
localtime(...)
    PROTOTYPE: ;$
    INIT:
        Time64_T when;
        struct TM *err;
        struct TM date;
    PPCODE:
        if( GIMME_V == G_VOID ) {
            warn("Useless use of localtime() in void context");
            XSRETURN_EMPTY;
        }

        if( items == 0 ) {
            time_t small_when;
            time(&small_when);
            when = (Time64_T)small_when;
        }
        else {
            double when_float = SvNV(ST(0));
            when = (Time64_T)when_float;

            /* Check for Time64_T overflow */
            if( !about_eq((double)when, when_float, 1024.0) ) {
                warn("localtime(%.0f) can not be represented", when_float);
                XSRETURN_EMPTY;
            }
        }

	tzset();
        err = localtime64_r(&when, &date);

        if( err == NULL )
        {
            warn("localtime(%.0f) can not be represented", (double)when);
            XSRETURN_EMPTY;
        }

        if( GIMME_V == G_ARRAY ) {
            EXTEND(SP, 9);
            myPUSHi(date.tm_sec);
            myPUSHi(date.tm_min);
            myPUSHi(date.tm_hour);
            myPUSHi(date.tm_mday);
            myPUSHi(date.tm_mon);
            myPUSHn((double)date.tm_year);
            myPUSHi(date.tm_wday);
            myPUSHi(date.tm_yday);
            myPUSHi(date.tm_isdst);
        }
        else {
            SV *tsv;
            /* XXX newSVpvf()'s %lld type is broken, so cheat with a double */
            double year = (double)date.tm_year + 1900;

            EXTEND(SP, 1);
            EXTEND_MORTAL(1);

            tsv = newSVpvf("%s %s %2d %02d:%02d:%02d %.0f",
                           dayname[date.tm_wday],
                           monname[date.tm_mon],
                           date.tm_mday,
                           date.tm_hour,
                           date.tm_min,
                           date.tm_sec,
                           year);
            myPUSHs(tsv);
        }


double
timegm(...)
     INIT:
         struct TM date;
         Time64_T when;
     CODE:
         if( items < 6 )
             croak("Usage: timegm($sec, $min, $hour, $mday, $month, $year)");

         date.tm_sec  = SvIV(ST(0));
         date.tm_min  = SvIV(ST(1));
         date.tm_hour = SvIV(ST(2));
         date.tm_mday = SvIV(ST(3));
         date.tm_mon  = SvIV(ST(4));
         date.tm_year = (Year)SvNV(ST(5));

         when = timegm64(&date);

         RETVAL = (double)when;

     OUTPUT:
         RETVAL


double
timelocal(...)
     INIT:
         struct TM date;
         Time64_T when;
     CODE:
         if( items < 6 )
             croak("Usage: timelocal($sec, $min, $hour, $mday, $month, $year)");

         date.tm_sec  = SvIV(ST(0));
         date.tm_min  = SvIV(ST(1));
         date.tm_hour = SvIV(ST(2));
         date.tm_mday = SvIV(ST(3));
         date.tm_mon  = SvIV(ST(4));
         date.tm_year = (Year)SvNV(ST(5));

         date.tm_isdst = items >= 9 ? SvIV(ST(8)) : -1;

         when = mktime64(&date);

         RETVAL = (double)when;

     OUTPUT:
         RETVAL
