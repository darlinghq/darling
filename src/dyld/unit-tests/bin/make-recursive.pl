#!/usr/bin/perl 

use strict;
use Data::Dumper;
use File::Find;
use Cwd qw(realpath);

my @args = @ARGV;

my $makefiles =
{
    'makefile' => undef,
    'Makefile' => undef,
};

my $find_opts =
{
    'wanted' => \&find_callback,
};

my $keywords =
{
    'root'   => '',
    'cwd'    => '',
    'cmd'    => '',
    'exit'   => '',
    'stdout' => [],
    'stderr' => [],
};

my $keyword;
my $max_keyword_len = 0;
foreach $keyword (keys %$keywords)
{ if($max_keyword_len < length($keyword)) { $max_keyword_len = length($keyword); } }
my $delim = ':';
$max_keyword_len += length($delim) + length(' ');

my $last_keyword = '';

sub print_line
{
    my ($keyword, $val) = @_;

    if(!exists($$keywords{$keyword}))
    {
	print STDERR "error: keyword $keyword not in \$keywords set\n";
	exit(1);
    }

    my $keyword_len = 0;

    if($keyword ne $last_keyword)
    {
	print("$keyword"); print($delim);
	$keyword_len = length($keyword) + length($delim);
    }
    if($max_keyword_len > $keyword_len)
    {
	my $num_spaces = $max_keyword_len - $keyword_len;
	print(' ' x $num_spaces);
    }
    print("$val");
    if(0)
    {
	$last_keyword = $keyword;
    }
}

my $root = '.';
$root = &realpath($root);
print_line("root", "$root\n");

find($find_opts, $root);

sub find_callback
{
    if(exists($$makefiles{$_}))
    {
	my $makefile = $_;
	my $reldir = $File::Find::dir;
	$reldir =~ s|^$root/||;
	
	&print_line("cwd", "\$root/$reldir\n");
	my $cmd = [ "make" ];
	
	my $arg; foreach $arg (@ARGV) { push @$cmd, $arg; } # better way to do this?
	&print_line("cmd", "@$cmd\n");

        open(SAVEOUT, ">&STDOUT") || die("$!");
        open(SAVEERR, ">&STDERR") || die("$!");
	open(STDOUT, ">/tmp/unit-tests-stdout") || die("$!");
	open(STDERR, ">/tmp/unit-tests-stderr") || die("$!");

	$ENV{UNIT_TEST_NAME} = $reldir;
	my $exit = system(@$cmd);

	close(STDOUT) || die("$!");
	close(STDERR) || die("$!");
        open(STDOUT, ">&SAVEOUT") || die("$!");
        open(STDERR, ">&SAVEERR") || die("$!");

	&print_line("exit", "$exit\n");

	open(OUT, "</tmp/unit-tests-stdout") || die("$!");
	while(<OUT>)
	{
	    &print_line("stdout", "$_");
	}
	close(OUT) || die("$!");
	unlink("/tmp/unit-tests-stdout");
	
	open(ERR, "</tmp/unit-tests-stderr") || die("$!");
	while(<ERR>)
	{
	    &print_line("stderr", "$_");
	}
	close(ERR) || die("$!");
    }
	unlink("/tmp/unit-tests-stderr");
}
