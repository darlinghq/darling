#!/bin/bash
set -x

pushd $(dirname "$0") > /dev/null
SCRIPTDIR=$(pwd -P)
popd > /dev/null

SRCROOT="$SCRIPTDIR/.."

pushd "${SRCROOT}/include"
for i in `find . -name \*.h -print0 | xargs -0 grep -l '^//Begin-Libc'`; do
	mkdir -p "../private-include/$(dirname $i)"
	cp "$i" "../private-include/$i"
	ed - "$i" < "${SRCROOT}/xcodescripts/strip-header.ed"
	#sed -i '' -e 's/\/\/Begin-Libc/\#ifdef BUILDING_LIBC/g; s/\/\/End-Libc/\#endif \/\/ BUILDING_LIBC/g' "$i"
done
popd
