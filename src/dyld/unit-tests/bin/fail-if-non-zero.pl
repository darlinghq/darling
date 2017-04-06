#!/usr/bin/perl -w

use strict;

# first arg is fail string, rest of args are command line to invoke
my $fail_string = shift @ARGV;

if(system(@ARGV) != 0)
{
    printf("FAIL \"$fail_string\"\n");
}

exit 0;

