#!/bin/bash

for platform in ${SUPPORTED_PLATFORMS} ; do
    [[ "${platform}" == "${PLATFORM_NAME}" ]] && exit 0
done

echo "Unsupported platform encountered:" >&2
echo "    PLATFORM_NAME: ${PLATFORM_NAME}" >&2
echo "    SUPPORTED_PLATFORMS: ${SUPPORTED_PLATFORMS}" >&2
echo "Please contact the Darwin Runtime Team for help with your new platform bringup needs" >&2
exit 1
