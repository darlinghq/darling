#!/usr/bin/perl
#
# Copyright (c) 2006, 2007 Apple Inc. All rights reserved.
#
# @APPLE_LICENSE_HEADER_START@
# 
# This file contains Original Code and/or Modifications of Original Code
# as defined in and that are subject to the Apple Public Source License
# Version 2.0 (the 'License'). You may not use this file except in
# compliance with the License. Please obtain a copy of the License at
# http://www.opensource.apple.com/apsl/ and read it before using this
# file.
# 
# The Original Code and all software distributed under the License are
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
# Please see the License for the specific language governing rights and
# limitations under the License.
# 
# @APPLE_LICENSE_HEADER_END@
#
# patchheaders srcdir destdir
#
# The last path component of srcdir is replicated in destdir, with the
# __DARWIN_ALIAS* and __DARWIN_EXTSN macro wrapped so that Libc can set
# the symbol decoration independently:
#
#   #ifndef LIBC_ALIAS_FOO
#   int foo(int) __DARWIN_ALIAS(foo);
#   #else /* LIBC_ALIAS_FOO */
#   int foo(int) LIBC_ALIAS(foo);
#   #endif /* !LIBC_ALIAS_FOO */

use strict;
use IO::File;
use File::Basename ();
use File::Find ();
use File::Path ();
use File::Spec;

my $MyName = File::Basename::basename($0);
my $dest;

sub process {
    my($path, $file) = @_;
    local $_;
	if (-e $file) {
		my $dest_mtime = (stat($file))[9];
		my $src_mtime = (stat($path))[9];

		if ($dest_mtime > $src_mtime) {
			return;
		}
	}

	my($fname, $dirs, $suffix) = File::Basename::fileparse($file, ".h");
	if ($suffix ne ".h") {
		return;
	}

    my $p = IO::File->new($path, 'r');
    die "$MyName: Can't open $path: $!\n" unless defined($p);
    my $f = IO::File->new($file, 'w');
    die "$MyName: Can't open $file: $!\n" unless defined($f);
    my @save;
    while(<$p>) {
		if(/^\S/ or /^\s*$/) {
			my $n = scalar(@save);
			my $sym;
			if($n > 0) {
				my($sym) = ($save[$n - 1] =~ /__DARWIN_(?:10\d+|ALIAS|EXTSN|INODE64)[^(]*\(([^)]*)\)/);
				if($save[$n - 1] =~ /__DARWIN_ALIAS_STARTING/) {
					undef $sym;
				}
				if(defined($sym)) {
					if(defined($path)) {
						print "  $path\n";
						undef($path);
					}
					$sym =~ s/^\s+//;
					$sym =~ s/\s+$//;
					$sym =~ tr/a-z/A-Z/;
					$f->print("#ifndef LIBC_ALIAS_$sym\n");
				}
				$f->print(@save);
				if(defined($sym)) {
					$save[$n - 1] =~ s/__DARWIN_(10\d+|ALIAS|EXTSN|INODE64)/LIBC_$1/;
					$f->print("#else /* LIBC_ALIAS_$sym */\n");
					$f->print(@save);
					$f->print("#endif /* !LIBC_ALIAS_$sym */\n");
				}
			}
			if(/^#/) {
				$f->print($_);
			@save = ();
	    } else {
			@save = ($_);
	    }
	} else {
	    push(@save, $_);
	}
}
$f->print(@save);
}

sub usage {
    die "Usage: $MyName srcdir dstdir\n";
}

sub wanted {
    if(-d $File::Find::name) {
		#print "DIR: $File::Find::name\n";
		my $dir = File::Spec->join($dest, $File::Find::name);
		File::Path::mkpath($dir, 0, 0755);
    } else {
		#print "FIL: $File::Find::name\n";
		my $file = File::Spec->join($dest, $File::Find::name);
		process($File::Find::name, $file);
    }
}

usage() unless scalar(@ARGV) == 2;
my $start = File::Basename::dirname($ARGV[0]);
chdir($start) || die "$MyName: chdir($start): $!\n";
$dest = $ARGV[1];
File::Path::mkpath($dest, 0, 0755);
File::Find::find({wanted => \&wanted, no_chdir => 1}, File::Basename::basename($ARGV[0]));
