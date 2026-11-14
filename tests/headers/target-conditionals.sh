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

# Checks that basic-headers/TargetConditionals.h keeps the TARGET_OS_* values
# clang predefines from -target (macOS and iOS), and that Availability.h has
# the public version macros. Usage: target-conditionals.sh SOURCE_DIR
# CC selects the compiler (default: clang).
set -u
src=${1:?usage: target-conditionals.sh SOURCE_DIR}
cc=${CC:-clang}
fail=0
err=$(mktemp)
trap 'rm -f "$err"' EXIT

run() {
	name=$1; shift
	if $cc -fsyntax-only -nostdinc -I "$src/basic-headers" "$@" "$src/tests/headers/target-conditionals.c" 2>"$err"; then
		echo "PASS $name"
	else
		echo "FAIL $name"; cat "$err"; fail=1
	fi
	
}

run macos-x86_64 -target x86_64-apple-macos10.15
run macos-i386 -target i386-apple-macos10.13
run macos-arm64 -target arm64-apple-macos11
run ios-arm64 -target arm64-apple-ios13.0 -DEXPECT_IOS
exit $fail
