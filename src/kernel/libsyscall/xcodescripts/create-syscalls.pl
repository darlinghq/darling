#!/usr/bin/perl
#
# Copyright (c) 2006-2014 Apple Inc. All rights reserved.
#
# @APPLE_OSREFERENCE_LICENSE_HEADER_START@
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
# @APPLE_OSREFERENCE_LICENSE_HEADER_END@
#
##########################################################################
#
# % create-syscalls.pl syscalls.master custom-directory platforms-directory platform-name out-directory
#
# This script fills the the out-directory with a Makefile.inc and *.s
# files to create the double-underbar syscall stubs.  It reads the
# syscall.master file to get the symbol names and number of arguments,
# and whether Libsystem should automatically create the (non-double-underbar)
# stubs if Libc doesn't provide a wrapper.  Which system calls will get
# the automatic treatment is writen to the libsyscall.list file, also
# written to the out-directory.
#
# The custom-directory contains:
# 1. SYS.h - used by the automatically created *.s and custom files
# 2. custom.s - contains architecture specific additional system calls and
#    auxilliary routines (like cerror)
# 3. special case double-underbar stub files - which are copied into
#    the out-directory
#
##########################################################################

use strict;
use File::Basename ();
use File::Copy ();
use File::Spec;
use IO::File;

my $MyName = File::Basename::basename($0);

my @CustomSrc = qw(custom.s);

my @Architectures = split /\s/, $ENV{"ARCHS"};
my @Copy = (qw(SYS.h), @CustomSrc);
my $CustomDir;
my $PlatformsDir;
my $PlatformName;
my $OutDir;
# size in bytes of known types (only used for i386)
my %TypeBytes = (
    'au_asid_t'		=> 4,
    'sae_associd_t'	=> 4,
    'caddr_t'		=> 4,
    'sae_connid_t'	=> 4,
    'gid_t'		=> 4,
    'id_t'		=> 4,
    'idtype_t'		=> 4,
    'int'		=> 4,
    'int32_t'		=> 4,
    'int64_t'		=> 8,
    'key_t'		=> 4,
    'long'		=> 4,
    'mach_port_name_t'	=> 4,
    'mode_t'		=> 4,
    'off_t'		=> 8,
    'pid_t'		=> 4,
    'semun_t'		=> 4,
    'sigset_t'		=> 4,
    'size_t'		=> 4,
    'socklen_t'		=> 4,
    'ssize_t'		=> 4,
    'u_int'		=> 4,
    'u_long'		=> 4,
    'uid_t'		=> 4,
    'uint32_t'		=> 4,
    'uint64_t'		=> 8,
    'user_addr_t'	=> 4,
    'user_long_t'	=> 4,
    'user_size_t'	=> 4,
    'user_ssize_t'	=> 4,
    'user_ulong_t'	=> 4,
    'uuid_t'		=> 4,
);

# Types that potentially have different sizes in user-space compared to
# kernel-space as well as whether the value should be sign/zero-extended when
# passing the user/kernel boundary.
my %UserKernelMismatchTypes = (
    'long'          => 'SIGN_EXTEND',
    'size_t'        => 'ZERO_EXTEND',
    'u_long'        => 'ZERO_EXTEND',
    'user_size_t'   => 'ZERO_EXTEND',
    'user_ssize_t'  => 'SIGN_EXTEND'
);

# Moving towards storing all data in this hash, then we always know
# if data is aliased or not, or promoted or not.
my %Symbols = (
    "syscall" => {
        c_sym => "syscall",
        syscall => "syscall",
        asm_sym => "_syscall",
        is_private => undef,
        is_custom => undef,
        nargs => 0,
        bytes => 0,
        aliases => {},
        mismatch_args => {}, # Arguments that might need to be zero/sign-extended
    },
);

# An explicit list of cancelable syscalls. For creating stubs that call the
# cancellable version of cerror.
my @Cancelable = qw/
	accept access aio_suspend
	close connect connectx
	disconnectx
	faccessat fcntl fdatasync fpathconf fstat fstatat fsync
	getlogin
	ioctl
	link linkat lseek lstat
	msgrcv msgsnd msync
	open openat
	pathconf peeloff poll posix_spawn pread pselect pwrite
	read readv recvfrom recvmsg rename renameat
	rename_ext
	__semwait_signal __sigwait
	select sem_wait semop sendmsg sendto sigsuspend stat symlink symlinkat sync
	unlink unlinkat
	wait4 waitid write writev
/;

sub usage {
    die "Usage: $MyName syscalls.master custom-directory platforms-directory platform-name out-directory\n";
}

##########################################################################
# Read the syscall.master file and collect the system call names and number
# of arguments.  It looks for the NO_SYSCALL_STUB quailifier following the
# prototype to determine if no automatic stub should be created by Libsystem.
#
# The `sys_` prefix is stripped from syscall names, and is only kept for
# the kernel symbol in order to avoid namespace clashes and identify
# syscalls more easily.
#
# For the #if lines in syscall.master, all macros are assumed to be defined,
# except COMPAT_GETFSSTAT (assumed undefined).
##########################################################################
sub readMaster {
    my $file = shift;
    local $_;
    my $f = IO::File->new($file, 'r');
    die "$MyName: $file: $!\n" unless defined($f);
    my $line = 0;
    my $skip = 0;
    while(<$f>) {
        $line++;
        if(/^#\s*endif/) {
            $skip = 0;
            next;
        }
        if(/^#\s*else/) {
            $skip = -$skip;
            next;
        }
        chomp;
        if(/^#\s*if\s+(\S+)$/) {
            $skip = ($1 eq 'COMPAT_GETFSSTAT') ? -1 : 1;
            next;
        }
        next if $skip < 0;
        next unless /^\d/;
        s/^[^{]*{\s*//;
        s/\s*}.*$//; # }
        die "$MyName: no function prototype on line $line\n" unless length($_) > 0 && /;$/;
        my $no_syscall_stub = /\)\s*NO_SYSCALL_STUB\s*;/;
        my($name, $args) = /\s(\S+)\s*\(([^)]*)\)/;
        next if $name =~ /e?nosys/;
        $name =~ s/^sys_//;
        $args =~ s/^\s+//;
        $args =~ s/\s+$//;
        my $argbytes = 0;
        my $nargs = 0;
        my %mismatch_args;
        if($args ne '' && $args ne 'void') {
            my @a = split(',', $args);
            $nargs = scalar(@a);
            my $index = 0;
            for my $type (@a) {
                $type =~ s/\s*\w+$//; # remove the argument name

                # Calculate the size of all the arguments (only used for i386)
                if($type =~ /\*$/) {
                    $argbytes += 4; # a pointer type
                } else {
                    $type =~ s/^.*\s//; # remove any type qualifier, like unsigned
                    my $b = $TypeBytes{$type};
                    die "$MyName: $name: unknown type '$type'\n" unless defined($b);
                    $argbytes += $b;
                }
                # Determine which arguments might need to be zero/sign-extended
                if(exists $UserKernelMismatchTypes{$type}) {
                    $mismatch_args{$index} = $UserKernelMismatchTypes{$type};
                }

                $index++;
            }
        }
        $Symbols{$name} = {
            c_sym => $name,
            syscall => $name,
            asm_sym => $no_syscall_stub ? "___$name" : "_$name",
            is_private => $no_syscall_stub,
            is_custom => undef,
            nargs => $nargs,
            bytes => $argbytes,
            aliases => {},
            mismatch_args => \%mismatch_args, # Arguments that might need to be zero/sign-extended
            except => [],
        };
    }
}

sub checkForCustomStubs {
    my ($dir) = @_;
    
    my ($c_sym_name, $sym);
    while (($c_sym_name, $sym) = each %Symbols) {
        my $source = "__".$$sym{c_sym}.".s";
        my $custom = File::Spec->join($dir, $source);
        next unless -f $custom;

        $$sym{is_custom} = $source;
        if (!$$sym{is_private}) {
            foreach my $subarch (@Architectures) {
                (my $arch = $subarch) =~ s/arm(v.*)/arm/;
                $arch =~ s/x86_64(.*)/x86_64/;
                $arch =~ s/arm64(.*)/arm64/;
                $$sym{aliases}{$arch} = [] unless $$sym{aliases}{$arch};
                push(@{$$sym{aliases}{$arch}}, $$sym{asm_sym});
            }
            $$sym{asm_sym} = "__".$$sym{asm_sym};
            $$sym{is_private} = 1;
        }
    }    
}

sub readAliases {
    my ($platformDir, $platformName) = @_;
    my $genericMap = File::Spec->join($platformDir, "syscall.map");
    
    my %sym_to_c;
    foreach my $k (keys %Symbols) {
        $sym_to_c{$Symbols{$k}{asm_sym}} = $k;
    }
    
    my @a = ();
    for my $arch (@Architectures) {
        (my $new_arch = $arch) =~ s/arm(v.*)/arm/g;
        $new_arch =~ s/x86_64(.*)/x86_64/g;
        $new_arch =~ s/arm64(.*)/arm64/g;
        push(@a, $new_arch) unless grep { $_ eq $new_arch } @a;
    }
    
    foreach my $arch (@a) {
        my $syscallFile = File::Spec->join($platformDir, $platformName, $arch, "syscall.map");
        
        my @files = ();
        push(@files, IO::File->new($syscallFile, 'r'));
        die "$MyName: $syscallFile: $!\n" unless defined($files[$#files]);
        push(@files, IO::File->new($genericMap, 'r'));
        die "$MyName: $genericMap: $!\n" unless defined($files[$#files]);
        
        foreach my $f (@files) {
            while (<$f>) {
                next if /^#/;
                chomp;
                
                my ($alias, $target_symbol) = split;
                if (defined($target_symbol)) {
                    foreach my $sym (values %Symbols) {
                        # I've eliminated most of the ugly from this script except
                        # the need to try stripping underbars here.
                        if ($$sym{is_private}) {
                            next unless $$sym{asm_sym} eq $target_symbol;
                        } else {
                            (my $target = $target_symbol) =~ s/^__//;
                            next unless ($$sym{asm_sym} eq $target || $$sym{asm_sym} eq $target_symbol);
                        }
                        $$sym{aliases}{$arch} = [] unless $$sym{aliases}{$arch};
                        
                        die "$MyName: $arch $$sym{asm_sym} -> $alias: Duplicate alias.\n" if grep { $_ eq $alias } @{$$sym{aliases}{$arch}};
                        push(@{$$sym{aliases}{$arch}}, $alias);
                        
                        # last thing to do, if we aliased over a first class symbol, we need
                        # to mark it
                        my $c = $sym_to_c{$alias};
                        if ($Symbols{$c}) {
                            push(@{$Symbols{$c}{except}}, $arch);
                        }
                    }
                }
            }
        }
    }
}

##########################################################################
# Make a __xxx.s file: if it exists in the $CustomDir, just copy it, otherwise
# create one.  We define the macro __SYSCALL_32BIT_ARG_BYTES so that SYS.h could
# use that to define __SYSCALL dependent on the arguments' total size.
##########################################################################
sub writeStubForSymbol {
    my ($f, $symbol) = @_;
    
    my @conditions;
    my $has_arm64 = 0;
    for my $subarch (@Architectures) {
        (my $arch = $subarch) =~ s/arm(v.*)/arm/;
        $arch =~ s/x86_64(.*)/x86_64/;
        $arch =~ s/arm64(.*)/arm64/;
        push(@conditions, "defined(__${arch}__)") unless grep { $_ eq $arch } @{$$symbol{except}};

        if($arch eq "arm64") {
            $has_arm64 = 1 unless grep { $_ eq $arch } @{$$symbol{except}};
        }
    }

    my %is_cancel;
    for (@Cancelable) { $is_cancel{$_} = 1 };

    print $f "#define __SYSCALL_32BIT_ARG_BYTES $$symbol{bytes}\n";
    print $f "#include \"SYS.h\"\n\n";

    if (scalar(@conditions)) {
        printf $f "#ifndef SYS_%s\n", $$symbol{syscall};
        printf $f "#error \"SYS_%s not defined. The header files libsyscall is building against do not match syscalls.master.\"\n", $$symbol{syscall};
        printf $f "#endif\n\n";
    }

    my $nc = ($is_cancel{$$symbol{syscall}} ? "cerror" : "cerror_nocancel");

    if($has_arm64) {
        printf $f "#if defined(__arm64__)\n";
        printf $f "MI_ENTRY_POINT(%s)\n", $$symbol{asm_sym};
        if(keys %{$$symbol{mismatch_args}}) {
            while(my($argnum, $extend) = each %{$$symbol{mismatch_args}}) {
                printf $f "%s(%d)\n", $extend, $argnum;
            }
        }

        printf $f "SYSCALL_NONAME(%s, %d, %s)\n", $$symbol{syscall}, $$symbol{nargs}, $nc;
        printf $f "ret\n";
        printf $f "#else\n";
    }

    if (scalar(@conditions)) {
        printf $f "#if " . join(" || ", @conditions) . "\n";
        printf $f "__SYSCALL2(%s, %s, %d, %s)\n", $$symbol{asm_sym}, $$symbol{syscall}, $$symbol{nargs}, $nc;
        if (!$$symbol{is_private} && (scalar(@conditions) < scalar(@Architectures))) {
            printf $f "#else\n";
            printf $f "__SYSCALL2(%s, %s, %d, %s)\n", "__".$$symbol{asm_sym}, $$symbol{syscall}, $$symbol{nargs}, $nc;
        }
        printf $f "#endif\n\n";
    } else {
        # actually this isnt an inconsistency. kernel can expose what it wants but if all our arches
        # override it we need to honour that.
    }

    if($has_arm64) {
        printf $f "#endif\n\n";
    }
}

sub writeAliasesForSymbol {
    my ($f, $symbol) = @_;
    
    foreach my $subarch (@Architectures) {
        (my $arch = $subarch) =~ s/arm(v.*)/arm/;
        $arch =~ s/x86_64(.*)/x86_64/;
        $arch =~ s/arm64(.*)/arm64/;
        
        next unless scalar($$symbol{aliases}{$arch});
        
				printf $f "#if defined(__${arch}__)\n";
        foreach my $alias_sym (@{$$symbol{aliases}{$arch}}) {
            my $sym = (grep { $_ eq $arch } @{$$symbol{except}}) ? "__".$$symbol{asm_sym} : $$symbol{asm_sym};
					
						printf $f "\t.globl\t$alias_sym\n";
						printf $f "\t.set\t$alias_sym, $sym\n";
        }
				printf $f "#endif\n\n";
    }
}

usage() unless scalar(@ARGV) == 5;
$CustomDir = $ARGV[1];
die "$MyName: $CustomDir: No such directory\n" unless -d $CustomDir;
$PlatformsDir = $ARGV[2];
die "$MyName: $PlatformsDir: No such directory\n" unless -d $PlatformsDir;
$PlatformName = $ARGV[3];
die "$MyName: $PlatformsDir/$PlatformName: No such directory\n" unless -d "$PlatformsDir/$PlatformName";
$OutDir = $ARGV[4];
die "$MyName: $OutDir: No such directory\n" unless -d $OutDir;

readMaster($ARGV[0]);
checkForCustomStubs($CustomDir);
readAliases($PlatformsDir, $PlatformName);

##########################################################################
# copy the files specified in @Copy from the $CustomDir to $OutDir
##########################################################################
for(@Copy) {
    my $custom = File::Spec->join($CustomDir, $_);
    my $path = File::Spec->join($OutDir, $_);
    print "Copy $custom -> $path\n";
    File::Copy::copy($custom, $path) || die "$MyName: copy($custom, $path): $!\n";
}

##########################################################################
# make all the *.s files
##########################################################################
my @src;
my($k, $sym);
while (($k, $sym) = each %Symbols)
{
	my $srcname = $$sym{asm_sym} . ".s";
	my $outpath = File::Spec->join($OutDir, $srcname);

	if ($$sym{is_custom}) {
		my $custom = File::Spec->join($CustomDir, $$sym{is_custom});
		File::Copy::copy($custom, $outpath);
		print "Copied $outpath\n";
		
		print "Writing aliases for $srcname\n";
		my $f = IO::File->new($outpath, 'a');
		die "$MyName: $outpath: $!\n" unless defined($f);
		writeAliasesForSymbol($f, $sym);
		undef $f;
	} else {
		my $f = IO::File->new($outpath, 'w');
		die "$MyName: $outpath: $!\n" unless defined($f);
		
		printf "Creating $outpath\n";
		writeStubForSymbol($f, $sym);
		writeAliasesForSymbol($f, $sym);
		undef $f;
	}
	push(@src, $srcname);
}

##########################################################################
# create the Makefile.inc file from the list for files in @src and @CustomSrc
##########################################################################
my $path = File::Spec->join($OutDir, 'stubs.list');
my $f = IO::File->new($path, 'w');
my @sources = sort(@src, @CustomSrc);
for my $s (@sources) {
	printf $f File::Spec->join($OutDir, $s) . "\n";
}
undef $f;
undef $path;

