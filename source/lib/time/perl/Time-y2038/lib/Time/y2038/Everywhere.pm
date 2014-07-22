package Time::y2038::Everywhere;

use strict;
use warnings;

our $VERSION = '20100403';

use Time::y2038;

*CORE::GLOBAL::localtime = \&localtime;
*CORE::GLOBAL::gmtime    = \&gmtime;

1;

__END__

=head1 NAME

Time::y2038::Everywhere - Use Time::y2038's gmtime and localtime everywhere.

=head1 SYNOPSIS

    use Time::y2038::Everywhere;

    # All uses of localtime() and gmtime() in the whole program
    # are now using Time::y2038's

=head1 DESCRIPTION

Time::y2038::Everywhere replaces localtime() and gmtime() with its own
functions everywhere.  This ensures not just that your code is 2038
safe, but that any modules you use are, too.

=head1 NOTES

May also override Time::Local::timelocal and Time::Local::timegm in
the future.

=head1 SEE ALSO

L<Time::y2038>

=cut
