#!/bin/bash

# build_linklists.sh
# Libc

# Skip script during installhdrs
if [ "$ACTION" == installhdrs ]; then exit 0; fi

ARCHIVES=(Platform Base FreeBSD FreeBSD_gcc NetBSD TRE vCancelable vDarwinExtsn vDarwinExtsnCancelable vPre1050 vLegacy vInode32)

NORMAL_LIST="$BUILT_PRODUCTS_DIR/normal.linklist"
DEBUG_LIST="$BUILT_PRODUCTS_DIR/debug.linklist"
INTERPOSABLE_LIST="$DERIVED_FILES_DIR/interposable.list"

rm -f $NORMAL_LIST
rm -f $DEBUG_LIST
rm -f $INTERPOSABLE_LIST

for x in ${ARCHIVES[@]}; do
	nm -AUamgf "$BUILT_PRODUCTS_DIR/lib${x}.a" 2>/dev/null | \
		grep '__TEXT,__text' | \
		grep -v '\[symbol resolver\]' | \
		grep -vE '\$VARIANT\$(m|u)p' | \
		awk '{ print $NF }' >> $INTERPOSABLE_LIST

	echo "-l$x" >> $NORMAL_LIST
	echo "-l${x}_debug" >> $DEBUG_LIST
done

touch "$BUILT_PRODUCTS_DIR/deps.c"

exit 0
