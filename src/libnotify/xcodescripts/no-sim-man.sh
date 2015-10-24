#!/bin/bash -ex

if [[ "${PLATFORM_NAME}" =~ "simulator" ]]; then
	rm -rf ${DSTROOT}${INSTALL_PATH_PREFIX}/usr/share/man
fi
