#!/bin/bash -e

if [ "${DRIVERKIT}" = 1 ]; then exit 0; fi

# install kdebug trace files based on the input file
INPUT=${SCRIPT_INPUT_FILE_0}
OUTPUT=${SCRIPT_OUTPUT_FILE_0}

# pre-process the source and pass through perl it
xcrun cc -E -I${SDKROOT}/System/Library/Frameworks/System.framework/PrivateHeaders -D_MALLOC_BUILDING_CODES_ "${INPUT}" | perl > "${OUTPUT}"
