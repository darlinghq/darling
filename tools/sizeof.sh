#!/bin/sh

set -e

cfile=/tmp/sizeof.$$.cpp
include="$1"
struct="$2"

cat > $cfile <<END
#include <stdio.h>
#include <$include>

int main()
{
	printf("sizeof($struct)=%d\n", sizeof($struct));
	return 0;
}

END

g++ -w $cfile -o $cfile.bin
$cfile.bin

rm $cfile*

