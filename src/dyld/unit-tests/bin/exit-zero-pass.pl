#!/usr/bin/perl -w

use strict;

sub PASS
{
    my ($format, $args) = @_;
    if(!defined $args)
    { $args = []; }
    printf("PASS \"$format\"\n", @$args);
}

sub FAIL
{
    my ($format, $args) = @_;
    if(!defined $args)
    { $args = []; }
    printf("FAIL \"$format\"\n", @$args);
}

my $pass_string = shift @ARGV;
my $fail_string = shift @ARGV;

# redirect stderr to stdout
open(STDERR, ">/tmp/exit-zero-pass.tmp") || die("$!");
if(0 == system(@ARGV))
{
    PASS($pass_string);
}
else
{
    FAIL($fail_string);
}
close(STDERR) || die("$!");
open(OUT, "</tmp/exit-zero-pass.tmp") || die("$!");
while(<OUT>)
{
	print $_;
}
close(OUT) || die("$!");
unlink "/tmp/exit-zero-pass.tmp";
exit 0;

