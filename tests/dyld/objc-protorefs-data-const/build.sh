#!/bin/sh
# Builds objc-protorefs-data-const with its __objc_protorefs moved into a
# read-only __DATA_CONST (ld64 itself keeps that section in __DATA).
# Usage: build.sh <SDK> <directory with the ld64 "ld" symlink> <libobjc.A.dylib>
set -eu
sdk=$1; ldDir=$2; objc=$3
cc="clang -target x86_64-apple-macos10.15 -isysroot $sdk -B $ldDir -D__DARWIN_ONLY_UNIX_CONFORMANCE=1"
dataConst="-Wl,-data_const -Wl,-rename_section,__DATA,__objc_protorefs,__DATA_CONST,__objc_protorefs"
$cc -dynamiclib -install_name @rpath/libprotorefproto.dylib -o libprotorefproto.dylib objc-protorefs-data-const-lib.m "$objc" $dataConst
$cc -o objc-protorefs-data-const objc-protorefs-data-const.m "$objc" -L. -lprotorefproto -Wl,-rpath,@executable_path $dataConst
