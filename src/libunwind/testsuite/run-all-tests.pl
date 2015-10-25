#!/usr/bin/perl 

use strict;
use warnings;
use File::stat;
use Cwd;


my $testCaseFilter = '*';
if ( $#ARGV == 0 ) {
	$testCaseFilter = $ARGV[0];
}

my $useLocalDylib = 0;
my $headerPath = "-I../include -I../src";
if ( stat('../build/Debug/libunwind.dylib') ) {
	printf "### running with ../build/Debug/libunwind.dylib ###\n";
	$useLocalDylib = 1;
}

my $canRunPPC = 0;
if ( stat('/usr/libexec/oah/translate') ) {
#	$canRunPPC = 1;
}


#
#  Search current directory for all files with TEST-OPTIONS: comments
#  Compile and link each file with specified options
#  Execute result and print PASS or FAIL
#
my $path = shift;
# get list of dylibs linked against
open(FILE, "grep -H 'TEST-OPTIONS:' $testCaseFilter |");
$/ = "\n";
while ( <FILE> ) {
	if ($_ =~ m/^(.+):.+TEST-OPTIONS:(.+)/) {
		my $file = $1;
		my $options = "$2";
		my $ppc = 0;
		if ( $options =~ /arch ppc/ ) {
			$ppc = 1;
		}
		if ( $file !~ "run-all-tests.pl" ) {
			my $exit;
			if ( $ppc && !$canRunPPC ) {
				# don't try to build ppc tests
			}
			elsif ( $file =~ /^(.+)\.cxx/ ) {
				#printf "c++ $file $options $headerPath -o /tmp/run-all-tests.a.out\n";
				$exit = system("c++ $file $options $headerPath -o /tmp/run-all-tests.a.out");
			}
			else {
				#printf "cc $file $options $headerPath -o /tmp/run-all-tests.a.out\n";
				$exit = system("cc $file $options $headerPath -o /tmp/run-all-tests.a.out");
			}
			if ( $ppc && ($useLocalDylib || !$canRunPPC) ) {
				#printf "SKIP $file $options\n";
			}
			elsif ( $exit == 0 ) {
				if ( $useLocalDylib ) {
					$ENV{'DYLD_LIBRARY_PATH'} = '../build/Debug';
				}
				my $exit = system("/tmp/run-all-tests.a.out");
				if ( $useLocalDylib ) {
					$ENV{'DYLD_LIBRARY_PATH'} = '';
				}
				if ( $exit == 0 ) {
					printf "PASS $file  $options\n";
				}
				else {
					printf "FAIL $file $options (runtime failure)\n";
				}
			}
			else {
				printf "FAIL $file $options (could not compile)\n";
			}
		}
	}
}
close FILE;

#
# Search for .mk directories and run make in each one
#
my $cwd = getcwd();
open(FILE, "ls -1 |");
$/ = "\n";
while ( <FILE> ) {
	if ($_ =~ m/^(.+\.mk)$/) {
		my $dir = $1;
		if ( ($testCaseFilter =~ /\*/) || ($dir =~ /$testCaseFilter/) ) {
			if ( $useLocalDylib ) {
				$ENV{'DYLD_LIBRARY_PATH'} = "$cwd/../build/Debug";
			}
			my $exit = system("cd $dir && make");
			if ( $useLocalDylib ) {
				$ENV{'DYLD_LIBRARY_PATH'} = '';
			}
			system("cd $dir && make clean");
		}
	}
}
close FILE;




