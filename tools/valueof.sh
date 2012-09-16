#!/bin/sh

set -e

cfile=/tmp/valueof.$$.c
include="$1"
var="$2"

cat > $cfile <<END
#include <stdio.h>
#include <$include>

int main()
{
	printf("$var=%d\n", $var);
	return 0;
}

END

gcc -w $3 $cfile -o $cfile.bin
$cfile.bin

rm $cfile*

