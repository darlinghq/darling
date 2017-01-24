#!/bin/bash

# build_linklists.sh
# Libc

# Skip script during installhdrs
if [ "$ACTION" == installhdrs ]; then exit 0; fi
# BUILD_ARCHIVES needs to be set in the project settings
if [ -z "$BUILD_ARCHIVES" ]; then exit 0; fi

[ ! -d "$BUILT_PRODUCTS_DIR" ] && mkdir -p "$BUILT_PRODUCTS_DIR"
[ ! -d "$DERIVED_FILES_DIR" ] && mkdir -p "$DERIVED_FILES_DIR"

NORMAL_LIST="$BUILT_PRODUCTS_DIR/normal.linklist"
DEBUG_LIST="$BUILT_PRODUCTS_DIR/debug.linklist"
INTERPOSABLE_LIST="$DERIVED_FILES_DIR/interposable.list"
UNEXPORT_LIST="$DERIVED_FILES_DIR/unexport.list"

rm -f "$NORMAL_LIST"
rm -f "$DEBUG_LIST"
rm -f "$INTERPOSABLE_LIST"
rm -f "$UNEXPORT_LIST"

for x in ${BUILD_ARCHIVES[@]}; do
	nm -AUamgf "$BUILT_PRODUCTS_DIR/lib${x}.a" 2>/dev/null | \
		grep '__TEXT,__text' | \
		grep -vE '\$VARIANT' | \
		awk '{ print $NF }' >> "$INTERPOSABLE_LIST"

	nm -AUamgf "$BUILT_PRODUCTS_DIR/lib${x}.a" 2>/dev/null | \
		awk '/\$VARIANT/ { print $NF }' >> "$UNEXPORT_LIST"

	echo "$BUILT_PRODUCTS_DIR/lib${x}.a" >> "$NORMAL_LIST.libtool"
	filename=$(printf %s "$BUILT_PRODUCTS_DIR/lib${x}.a" | sed "s/\ /\\\ /g")
	echo "$filename" >> "$NORMAL_LIST"

	echo "$BUILT_PRODUCTS_DIR/lib${x}_debug.a" >> "$DEBUG_LIST.libtool"
	filename=$(printf %s "$BUILT_PRODUCTS_DIR/lib${x}_debug.a" | sed "s/\ /\\\ /g")
	echo "$filename" >> "$DEBUG_LIST"
done

touch "$BUILT_PRODUCTS_DIR/deps.c"

exit 0
