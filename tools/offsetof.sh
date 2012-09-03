#!/bin/sh

set -e

cfile=/tmp/offsetof.$$.c
include="$1"
struct="$2"
member="$3"

cat > $cfile <<END
#include <stdio.h>
#include <stddef.h>
#include <$include>

int main()
{
	printf("offsetof($struct, $member)=%d\n", offsetof($struct, $member));
	return 0;
}

END

gcc -w $cfile -o $cfile.bin
$cfile.bin

rm $cfile*

