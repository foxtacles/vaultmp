#!/usr/bin/perl -w

use Config;
unless( defined $Config{quadtype} and length $Config{quadtype} ) {
    warn "Time::y2038 needs \$Config{quadtype} set to work.\n";
    die "OS Unsupported\n";
}


use lib 'inc';
use Local::Module::Build;

my $build = Local::Module::Build->new(
    module_name         => 'Time::y2038',
    license             => 'perl',

    include_dirs        => ['y2038'],
    c_source            => 'y2038',

#    extra_compiler_flags => "-g -Wall -ansi -pedantic -Wno-long-long -Wextra -Wdeclaration-after-statement -Wendif-labels -Wconversion",

    build_requires      => {
        'Module::Build'         => '0.36',
        'Test::Warn'            => '0.11',
        'Test::Exception'       => '0.27',
        'Test::More'            => 0.82,
        'ExtUtils::CBuilder'    => 0.24,
        'JSON'                  => 2.17,
    },

    configure_requires => {
        'Module::Build'         => '0.36'
    },

    requires            => {
        perl                    => '5.6.1'
    },
);

$build->create_build_script;
