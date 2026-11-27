#!/bin/sh
#
# This file is part of Darling.
#
# Copyright (C) 2026 Lubos Dolezel
#
# Darling is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Darling is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Darling.  If not, see <http://www.gnu.org/licenses/>.

# Compile probes for the header layout of FindFolder: as in the macOS SDK, FindFolder, FSFindFolder, the folder
# type constants and the volume/domain constants are declared by <CarbonCore/Folders.h> (which
# <CarbonCore/CarbonCore.h>, <CoreServices/CoreServices.h> and <Carbon/Carbon.h> include), and
# <CarbonCore/Files.h> alone does not declare them. FindFolder exists on 32-bit only, so the probes
# compile for i386 and x86_64. Host test: needs only the host clang and the SDK of the build tree.
# Usage: carboncore-folders-header.sh SDK_DIR     (CC selects the compiler, default: clang)
set -u
sdk=${1:?usage: carboncore-folders-header.sh SDK_DIR}
cc=${CC:-clang}
fail=0
dir=$(mktemp -d)
trap 'rm -rf "$dir"' EXIT
fw="$sdk/System/Library/Frameworks"

probe() { # name expected(pass|fail) arch source
	name=$1; want=$2; arch=$3; src=$4
	printf '%s\n' "$src" > "$dir/probe.c"
	if $cc -fsyntax-only -std=gnu11 -Wno-everything -Werror=implicit-function-declaration \
		-target $arch-apple-macos10.13 -isysroot "$sdk" -iframework "$fw/CoreServices.framework/Frameworks" \
		-iframework "$fw/ApplicationServices.framework/Frameworks" -iframework "$fw/Carbon.framework/Frameworks" \
		"$dir/probe.c" 2>"$dir/err"; then got=pass; else got=fail; fi
	if [ "$got" = "$want" ]; then echo "PASS $name ($arch)"; else echo "FAIL $name ($arch): expected $want"; head -3 "$dir/err"; fail=1; fi
}

use32='short v; long d; OSErr e = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &v, &d);'
use='FSRef r; OSStatus s = FSFindFolder(kUserDomain, kDesktopFolderType, kDontCreateFolder, &r); int t = kTemporaryFolderType;'

for arch in i386 x86_64; do
	if [ $arch = i386 ]; then u="$use32 $use"; else u="$use"; fi
	probe "CoreServices.h declares them" pass $arch "#include <CoreServices/CoreServices.h>
void f(void) { $u }"
	probe "Carbon.h declares them" pass $arch "#include <Carbon/Carbon.h>
void f(void) { $u }"
	probe "CarbonCore.h declares them" pass $arch "#include <CarbonCore/CarbonCore.h>
void f(void) { $u }"
	probe "Folders.h declares them" pass $arch "#include <CarbonCore/Folders.h>
void f(void) { $u }"
	probe "Files.h does not declare the constants" fail $arch "#include <CarbonCore/Files.h>
int t = kPreferencesFolderType;"
	probe "Files.h does not declare FSFindFolder" fail $arch "#include <CarbonCore/Files.h>
void f(FSRef *r) { FSFindFolder(kUserDomain, 0, 0, r); }"
done
probe "Files.h does not declare FindFolder" fail i386 "#include <CarbonCore/Files.h>
void f(short *v, long *d) { FindFolder(0, 0, 0, v, d); }"
probe "Files.h alone still declares the file manager" pass i386 "#include <CarbonCore/Files.h>
void f(FSRef *r) { FSRef x; (void) x; }"
exit $fail
