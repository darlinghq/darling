#!/bin/bash
set -x

pushd $(dirname "$0") > /dev/null
SCRIPTDIR=$(pwd -P)
popd > /dev/null

export SRCROOT="$SCRIPTDIR/.."
export SDKROOT="$SRCROOT/../../Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"

# copied and modified from dyld.xcodeproj
${SRCROOT}/bin/expand.rb < "${SRCROOT}/include/mach-o/dyld_priv.h" > "${SRCROOT}/include/mach-o/dyld_priv.h.tmp"

mv "${SRCROOT}/include/mach-o/dyld_priv.h.tmp" "${SRCROOT}/include/mach-o/dyld_priv.h"
