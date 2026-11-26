#!/bin/bash
# Copyright (C) 2026 Lubos Dolezel
#
# HIToolbox sits above CoreGraphics and calls its window server SPI through the dynamic linker, not
# through dlsym: the keyboard layout lookup of the Text Input Sources and the connection kind query of
# the menu tracking must be imports of the HIToolbox image, bound to CoreGraphics.
#
#   hitoolbox_direct_links.sh <path of the HIToolbox dylib on the host> [llvm-nm]
# Prints PASS or FAIL lines; exits 1 on failure.
lib=$1
nm=${2:-llvm-nm}
fail=0
for arch in x86_64 i386; do
	undef=$("$nm" -u --arch=$arch "$lib" 2>&1)
	for sym in __CGSCopyCurrentKeyboardLayoutInfo __CGSNativeConnectionKind; do
		if grep -qx -- "$sym" <<<"$undef"; then
			echo "PASS $arch imports $sym"
		else
			echo "FAIL $arch does not import $sym"; fail=1
		fi
	done
done
exit $fail
