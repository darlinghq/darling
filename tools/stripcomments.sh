#!/bin/sh

# This cool can be used to strip most of the copyrightable material from header files.
# Remember that all files processed with this script stil need to be checked manually!

gcc -DNS_BLOCKS_AVAILABLE -fpreprocessed -dD -E "$1" | egrep -v '# [[:digit:]]' | sed -r 's/APPKIT_EXTERN/EXTERN_C/;s/ ?(NS_AVAILABLE(_MAC)?|NS_DEPRECATED_MAC|NS_CLASS_AVAILABLE)\([^)]+\)//;s/#pragma mark.*//;s/NS_AUTOMATED_REFCOUNT_WEAK_UNAVAILABLE//;s/NS_RETURNS_INNER_POINTER//;s/NS_FORMAT_FUNCTION\(([0-9]+), ?([0-9]+)\)//'

