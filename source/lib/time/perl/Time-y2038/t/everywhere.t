#!/usr/bin/perl -w

use strict;
use warnings;

use Test::More 'no_plan';

{
    package Foo;
    use Time::y2038::Everywhere;
}

is   gmtime(2**52),    "Sat Dec  6 03:48:16 142715360";
like localtime(2**52), qr/Dec .* 142715360/;
