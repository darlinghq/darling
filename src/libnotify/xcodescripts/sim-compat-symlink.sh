#!/bin/bash -ex

if [[ "${PLATFORM_NAME}" =~ "simulator" ]]; then
	ln -s libsystem_notify.dylib ${DSTROOT}${INSTALL_PATH}/libnotify_sim.dylib
fi
