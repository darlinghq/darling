#!/bin/bash -ex

if [[ "${PLATFORM_NAME}" =~ "simulator" ]]; then
	rm -rf ${DSTROOT}/usr/share/man
fi
