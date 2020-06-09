#!/bin/bash -x

if [ "${RC_SUPPORT_ADDRESS_SANITIZATION}" == "1" -a "${ACTION}" == "install" ]; then
	mkdir -p "${DSTROOT}/usr/local/share/"
	echo "please" > "${DSTROOT}/usr/local/share/launchd-disable-memory-limits"
fi
