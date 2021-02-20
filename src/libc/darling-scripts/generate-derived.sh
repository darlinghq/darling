#!/bin/bash
set -x

pushd $(dirname "$0") > /dev/null
SCRIPTDIR=$(pwd -P)
popd > /dev/null

export SRCROOT="$SCRIPTDIR/.."
export ARCHS="x86_64 i386 arm64 arm ppc"
export DERIVED_FILES_DIR="$SRCROOT/derived"
export VARIANT_PLATFORM_NAME="macosx"

$SRCROOT/xcodescripts/generate_features.pl
