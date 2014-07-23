#!/usr/bin/perl

# Ensure localtime() honors the current time zone

use strict;
use warnings;

use Time::y2038;

use Test::More 'no_plan';

my $Time = time();

SKIP: {
    local $ENV{TZ};

    # Two time zones, different and likely to exist
    my $tz1 = "America/Los_Angeles";
    my $tz2 = "America/Chicago";

    # If the core localtime doesn't respond to TZ, we don't have to.
    skip "localtime does not respect TZ env", 1
      unless do {
          # check that localtime respects changes to $ENV{TZ}
          $ENV{TZ}  = $tz1;
          my $hour  = (CORE::localtime($Time))[2];
          $ENV{TZ}  = $tz2;
          my $hour2 = (CORE::localtime($Time))[2];
          $hour != $hour2;
      };

    # check that localtime respects changes to $ENV{TZ}
    $ENV{TZ}  = $tz1;
    my $hour  = (localtime($Time))[2];
    $ENV{TZ}  = $tz2;
    my $hour2 = (localtime($Time))[2];
    isnt $hour, $hour2, "localtime() honors TZ";
}
