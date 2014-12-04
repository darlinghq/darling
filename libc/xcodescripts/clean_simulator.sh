#!/bin/sh
# clean_simulator.sh
if [ "$ACTION" = installhdrs ]; then exit 0; fi

if [ "$PLATFORM_NAME" = iphonesimulator ]; then
	rm -rf "${DSTROOT}"/usr/local/lib/dyld
fi
