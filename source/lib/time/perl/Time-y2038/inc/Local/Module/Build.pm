package Local::Module::Build;

use strict;
use base qw(Module::Build);

use ExtUtils::CBuilder;

# A very conservative set of default limits for the system time.h
my %Limit_Defaults = (
    gmtime      => {
        max =>  2147483647,
        min =>  0,
    },

    localtime => {
        max => 2147483647,
        min => 0,
    },

    mktime => {
        max => {
            tm_gmtoff =>  0,
            tm_hour =>  3,
            tm_isdst =>  0,
            tm_mday =>  19,
            tm_min =>  14,
            tm_mon =>  0,
            tm_sec =>  7,
            tm_wday =>  2,
            tm_yday =>  18,
            tm_year =>  138,
            tm_zone =>  "UTC",
        },
        min => {
            tm_gmtoff =>  0,
            tm_hour =>  0,
            tm_isdst =>  0,
            tm_mday =>  1,
            tm_min =>  0,
            tm_mon =>  0,
            tm_sec =>  0,
            tm_wday =>  4,
            tm_yday =>  0,
            tm_year =>  70,
            tm_zone =>  "UTC",
        },
    },
    timegm => {
        max => {
            tm_gmtoff =>  0,
            tm_hour =>  3,
            tm_isdst =>  0,
            tm_mday =>  19,
            tm_min =>  14,
            tm_mon =>  0,
            tm_sec =>  7,
            tm_wday =>  2,
            tm_yday =>  18,
            tm_year =>  138,
            tm_zone =>  "UTC",
        },

        min => {
            tm_gmtoff =>  0,
            tm_hour =>  0,
            tm_isdst =>  0,
            tm_mday =>  1,
            tm_min =>  0,
            tm_mon =>  0,
            tm_sec =>  0,
            tm_wday =>  4,
            tm_yday =>  0,
            tm_year =>  70,
            tm_zone =>  "UTC",
        }
    }
);

sub is_osx_106 {
    return 0 unless $^O eq 'darwin';
    my $version = `sw_vers -productVersion`;
    return $version =~ m{^10\.6\.};
}

sub probe_system_time {
    my $self = shift;
    $self->note_time_capabilities;
    $self->munge("y2038/time64_config.h.in" => "y2038/time64_config.h");
    $self->note_time_limits;
    $self->munge("y2038/time64_limits.h.in" => "y2038/time64_limits.h");
}

sub munge {
    my $self = shift;
    my($src, $dest) = @_;

    return if $self->up_to_date(["munge_config",$src] => [$dest]);
    system $^X, "munge_config", $src, $dest;
    $self->add_to_cleanup($dest);
}

sub note_time_capabilities {
    my $self = shift;

    return if $self->up_to_date(["Build", "y2038/time64_limits.h.in"], ["y2038/time64_limits.h"]);
    my %tests = (
        HAS_TIMEGM      => <<'END',
    struct tm *date;
    time_t zero;

    date = localtime(&zero);
    zero = timegm(date);
END

        HAS_GMTIME_R    => <<'END',
    struct tm date;
    time_t zero = 0;
    (void)gmtime_r(&zero, &date);
END

        HAS_LOCALTIME_R => <<'END',
    struct tm date;
    time_t zero = 0;
    (void)localtime_r(&zero, &date);
END

        HAS_TM_TM_GMTOFF => <<'END',
    struct tm *date;
    time_t zero;
    int offset;

    date = gmtime(&zero);
    offset = date->tm_gmtoff;
END

        HAS_TM_TM_ZONE => <<'END',
    struct tm *date;
    time_t zero;
    char *zone;

    date = gmtime(&zero);
    zone = date->tm_zone;
END
    );

    warn "Probing time.h capabilities.\n";
    warn "You may see some C errors, that's ok.\n";
    my $cb = ExtUtils::CBuilder->new( quiet => 1 );
    for my $test (keys %tests) {
        my $code = $tests{$test};

        $code = <<END;
#include <time.h>

int main() {
$code

    return 0;
}
END

        open my $fh, ">", "try.c" or die "Can't write try.c: $!";
        print $fh $code;
        close $fh;

        my $exe = eval {
            # Compile AND link to force undefined symbols to error
            my $obj = $cb->compile(source => "try.c");
            my $exe = $cb->link_executable(objects => $obj, exe_file => "try");
            unlink $obj;
            $exe;
        };
        $self->notes($test, $exe ? 1 : 0);
        unlink $exe if $exe;
        unlink "try.c";
    }
}


sub note_time_limits {
    my $self = shift;

    my $source = "check_max.c";
    my $exe    = $self->notes("check_max") || "check_max";
    unless( $self->up_to_date($source => $exe) ) {
        warn "Building a program to test the range of your system time functions...\n";
        my $cb = $self->cbuilder;
        my $obj = $cb->compile(source => "check_max.c", include_dirs => ['y2038']);
        $exe = $cb->link_executable(objects => $obj, exe_file => $exe);
        $exe = $self->find_real_exe($exe);
        $self->notes(check_max => $exe);
        $self->add_to_cleanup($obj, $exe);
    }

    return if $self->up_to_date(["y2038/time64_limits.h.in", "munge_config", $exe]
                                => "y2038/time64_limits.h");
    warn "  and running it...\n";

    require JSON;

    my $limits;
    my $json = run_in_alarm( 2, sub {
        return `./$exe`;
    }, sub {
        warn "  time limit check timed out, using defaults";
        return;
    });
    warn "  Done.\n";

    if( $json ) {
        $json =~ s{^\#.*\n}{}gm;
        $limits = JSON::from_json($json);
    }
    else {
        $limits = \%Limit_Defaults;
    }

    my %config;
    for my $key (qw(gmtime localtime)) {
        for my $limit (qw(min max)) {
            $config{$key."_".$limit} = $limits->{$key}{$limit};
        }
    }

    for my $key (qw(mktime timegm)) {
        for my $limit (qw(min max)) {
            my $struct = $limits->{$key}{$limit};
            for my $tm (keys %$struct) {
                $config{$key."_".$limit."_".$tm} = $struct->{$tm};
            }
        }
    }

    # Windows lies about being able to handle just a little bit of
    # negative time.
    for my $key (qw(gmtime_min localtime_min)) {
        if( -10_000 < $config{$key} && $config{$key} < 0 ) {
            $config{$key} = 0;
        }
    }

    # OS X 10.6's gmtime is broken before -70546986201600
    # See Apple bug 7654647
    if( is_osx_106 ) {
        for my $key (qw(gmtime_min localtime_min)) {
            if( $config{$key} < -70546986201600 ) {
                $config{$key} = -70546986201600;
            }
        }
    }

    for my $key (sort { $a cmp $b } keys %config) {
        my $val = $config{$key};
        warn sprintf "%15s:  %s\n", $key, $val;
        $self->notes($key, "$val");
    }

    return;
}


sub run_in_alarm {
    my($timeout, $try, $catch) = @_;

    my(@ret, $ret);
    my $pass = eval {
        local $SIG{ALRM} = sub { die "alarm\n" };
        alarm $timeout;
        wantarray ? @ret = $try->() : $ret = $try->();
        alarm 0;
        1;
    };

    return wantarray ? @ret : $ret if $pass;

    return $catch->() if $@ eq "alarm\n";

    die $@;
}


# This is necessary because Cygwin nerotically puts a .exe on
# every executable.  This appears to be built into gcc.
sub find_real_exe {
    my $self = shift;
    my $exe = shift;

    my $real_exe;
    for ($exe, "$exe.exe") {
        $real_exe = $_ if -e;
    }

    warn "Can't find the executable, thought it was $exe"
        unless defined $real_exe;

    return $real_exe;
}


sub ACTION_code {
    my $self = shift;

    $self->probe_system_time;

    return $self->SUPER::ACTION_code(@_);
}

1;

