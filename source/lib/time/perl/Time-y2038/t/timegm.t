#!/usr/bin/perl -w

use strict;
use warnings;

use Test::More 'no_plan';
use Test::Exception;

BEGIN {
    use_ok 'Time::y2038';
}

# Try to set the time zone so we can reliably test localtime().
local $ENV{TZ} = 'US/Pacific';
my $Test_Localtime = localtime(0) eq 'Wed Dec 31 16:00:00 1969';


for my $time (-2**52, -1, 0, 1, 2**52) {
    cmp_ok timegm(gmtime($time)),       '==', $time, sprintf "timegm(gmtime(%.0f))", $time;
    cmp_ok timelocal(localtime($time)), '==', $time, sprintf "timelocal(localtime(%.0f))", $time;
}


# Test DST handling.
SKIP: {
    skip "Tests specific to US/Pacific time zone", 4 unless $Test_Localtime;

    # 1 second before old American DST spring ahead
    my $time = 1143971999;
    cmp_ok timelocal(59, 59, 1, 2, 3, 106), '==', $time,
      'DST just before spring ahead';
    is localtime($time), 'Sun Apr  2 01:59:59 2006';

    $time++;
    cmp_ok timelocal(0, 0, 3, 2, 3, 106), '==', $time,
      'DST just after spring ahead';
    is localtime($time), 'Sun Apr  2 03:00:00 2006';


    # 1 second before fall back.
    $time = 1162108799;
    cmp_ok timelocal(59, 59, 0, 29, 9, 106), '==', $time,
      'DST just before fall back';
    is localtime($time), 'Sun Oct 29 00:59:59 2006';

    # Sun Oct 29 01:00:00 2006 DST, one second later
    $time++;
    cmp_ok timelocal(0, 0, 1, 29, 9, 106, undef, undef, 1), '==', $time,
      'DST at 1am';
    is localtime($time), 'Sun Oct 29 01:00:00 2006';


    # After one hour, it's still 1am but no longer DST
    $time += 3600;
    cmp_ok timelocal(0, 0, 1, 29, 9, 106, undef, undef, 0), '==', $time,
      'not DST at 1am';
    is localtime($time), 'Sun Oct 29 01:00:00 2006';


    # An hour later it's 2am.
    $time += 3600;
    cmp_ok timelocal(0, 0, 2, 29, 9, 106), '==', $time,
      '3am after DST fall back is 2 hours after 2am';
    is localtime($time), 'Sun Oct 29 02:00:00 2006';
}


#line 16
throws_ok {
    timegm();
} qr[^Usage: timegm\(\$sec, \$min, \$hour, \$mday, \$month, \$year\) at \Q$0\E line 17\.$];

#line 24
throws_ok {
    timelocal();
} qr[^Usage: timelocal\(\$sec, \$min, \$hour, \$mday, \$month, \$year\) at \Q$0\E line 25\.$];
