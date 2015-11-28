#!/bin/sh

for i in autoconf; do
    echo "$i"
    $i
    if [ $? -ne 0 ]; then
	echo "Error $? in $i"
	exit 1
    fi
done

if [ "$1" = "--noconfigure" ]; then
	exit 0;
fi

echo "./configure --enable-autogen $@"
./configure --enable-autogen $@
if [ $? -ne 0 ]; then
    echo "Error $? in ./configure"
    exit 1
fi
