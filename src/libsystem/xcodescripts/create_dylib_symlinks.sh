#!/bin/bash -x

if [ $# -ne 3 ]; then
    echo "Usage: $0 <dstroot> <action> <variants>" 1>&2
    exit 1
fi

if [[ "${DRIVERKIT}" == "1" ]]; then exit 0; fi; # No compatibility symlinks in DriverKit SDK

DSTROOT="$1${INSTALL_PATH_PREFIX}"
ACTION="$2"
VARIANTS="$3"

BSD_LIBS="c info m pthread dbm poll dl rpcsvc proc"

if [[ "${PLATFORM_NAME}" =~ macosx ]]; then
   BSD_LIBS="${BSD_LIBS} gcc_s.1"
fi

mkdir -p "${DSTROOT}/usr/lib" || exit 1

if [ "${ACTION}" != "installhdrs" ]; then
    for variant in ${VARIANTS}; do
	suffix=""
	if [ ${variant} != "normal" ]; then
	    suffix="_${variant}"
	fi

        ln -sf "libSystem.B${suffix}.dylib" "${DSTROOT}/usr/lib/libSystem${suffix}.dylib" || exit 1

	for i in ${BSD_LIBS}; do
	    ln -sf "libSystem.dylib" "${DSTROOT}/usr/lib/lib${i}.dylib" || exit 1
	done
    done
fi
