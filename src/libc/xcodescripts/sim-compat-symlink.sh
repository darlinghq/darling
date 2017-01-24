#!/bin/bash -ex

if [[ "${ACTION}" == "installhdrs" ]]; then
    exit 0
fi

if [[ "${PLATFORM_NAME}" =~ "simulator" ]]; then
    ln -s libsystem_c.dylib ${DSTROOT}${INSTALL_PATH}/libsystem_sim_c.dylib
fi
