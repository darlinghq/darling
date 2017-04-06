#!/usr/bin/perl -w

use strict;
use Data::Dumper;
use File::Find;
use Cwd;

$Data::Dumper::Terse = 1;

my $root = undef;
my $entry = '';
my $pass_count = 0;
my $total_count = 0;

# first match "root: "

# a line starting with "cwd:" marks the beginning of a new test case
# call process_entry() on each test case
while(<>)
{
    if(m/^root:\s+(.*?)$/)
    {
	$root = $1;
    }
    elsif(m/^cwd:\s+(.*?)$/)
    {
	if(length($entry))
	{
	    &process_entry($root, $entry);
	    $entry = '';
	}
	$entry .= $_;
    }
    else
    {
	$entry .= $_;
    }
}
# don't forget last test case (no cwd: to mark end)
if(length($entry))
{
    &process_entry($root, $entry);
}

# show totals
my $percentage = $pass_count * 100 / $total_count;
printf " * * * %d of %d unit-tests passed (%.1f percent) * * *\n", $pass_count, $total_count, $percentage;


sub process_entry
{
    my ($root, $lines) = @_;

    # build an associative array of keys to value(s) 
    my $lines_seq = [split /\n/, $lines];
    #print Dumper($lines_seq);
    my $tbl = { 'root' => $root, 'stdout' => [], 'stderr' => [] };
    my $line;
    foreach $line (@$lines_seq)
    {
	if($line =~ m/^(\w+):\s+(.*)$/)
	{
	    my $key = $1;
	    my $val = $2;
	    if(!exists($$tbl{$key}))
	    { $$tbl{$key} = ''; }

	    if($key eq 'stdout' || $key eq 'stderr') # if type is @array
	    {
		push @{$$tbl{$key}}, $val;
	    }
	    else
	    {
		$$tbl{$key} .= $val;
	    }
	}
	else
	{
	    print "ERROR: $line";
	}
    }
    #print Dumper($tbl);
    #return;

    my $test_name = $$tbl{cwd};
    if ($test_name =~ m|.*/([a-zA-Z0-9-+_]+)$|)
    {
	$test_name = $1;
    }
    
    #if make failed (exit was non-zero), mark this as a failure
    if(0 ne $$tbl{exit})
    {
	printf "%-40s FAIL Makefile failure\n", $test_name;
	$total_count++;
	return;
    }
    my $seen_result = 0;

    #if there was any output to stderr, mark this as a failure
    foreach $line (@{$$tbl{stderr}})
    {
	printf "%-40s FAIL spurious stderr failure: %s\n", $test_name, $line;
	$total_count++;
	return;
    }

    # scan all stdout looking for lines that start with PASS or FAIL
    foreach $line (@{$$tbl{stdout}})
    {
	if($line =~ m/^(PASS|XPASS|FAIL|XFAIL).+/)
	{
	    $total_count++;
	    if($line =~ m/^PASS.+/)
	    {
		$pass_count++;
	    }
	    else
	    {
		# only print failure lines
		printf "%-40s %s\n", $test_name, $line;
	    }
	    $seen_result = 1;
	}
    }
    if(!$seen_result)
    {
	printf "%-40s AMBIGIOUS missing [X]PASS/[X]FAIL\n", $test_name;
	$total_count++;
    }
}
