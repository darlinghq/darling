#!/bin/bash -x

if [ $# -ne 5 ]; then
    echo "Usage: $0 <dstroot> <srcroot> <action> <archs> <variants>" 1>&2
    exit 1
fi

DSTROOT="$1${INSTALL_PATH_FRAMEWORK_PREFIX}"
SRCROOT="$2"
ACTION="$3"
ARCHS="$4"
VARIANTS="$5"

FPATH="/System/Library/Frameworks/System.framework"

# <rdar://problem/21815807> System.framework on iOS needs to be a special snowflake.
# Other projects install headers into Versions/B/PrivateHeaders the sub-directories
# and symlinks still need to exist, even though they don't fit in flat bundles.
mkdir -p "${DSTROOT}/${FPATH}" || exit 1
mkdir -p "${DSTROOT}/${FPATH}/Versions" || exit 1
mkdir -p "${DSTROOT}/${FPATH}/Versions/B" || exit 1
ln -sf "B" "${DSTROOT}/${FPATH}/Versions/Current" || exit 1
ln -sf "Versions/Current/PrivateHeaders" "${DSTROOT}/${FPATH}/PrivateHeaders" || exit 1

if [[ "${DRIVERKIT}" == "1" ]]; then exit 0; fi; # No compatibility symlinks in DriverKit SDK

if [[ "${PLATFORM_NAME}" =~ macosx ]]; then
	ln -sf "Versions/Current/Resources" "${DSTROOT}/${FPATH}/Resources" || exit 1

	for variant in ${VARIANTS}; do
		suffix=""
		if [ ${variant} != "normal" ]; then
			suffix="_${variant}"
		fi
		ln -sf "Versions/Current/System${suffix}" "${DSTROOT}/${FPATH}/System${suffix}" || exit 1
		ln -sf "../../../../../../usr/lib/libSystem.B${suffix}.dylib" "${DSTROOT}/${FPATH}/Versions/B/System${suffix}" || exit 1
	done
else
	for variant in ${VARIANTS}; do
		suffix=""
		if [ ${variant} != "normal" ]; then
			suffix="_${variant}"
		fi

		ln -sf "../../../../usr/lib/libSystem.B${suffix}.dylib" "${DSTROOT}/${FPATH}/System${suffix}" || exit 1
	done
fi
