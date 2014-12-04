#!/bin/bash -e

export MIG=`xcrun -find mig`
export MIGCC=`xcrun -find cc`

BASE=`basename ${SCRIPT_INPUT_FILE_0} .defs`.h

for a in ${ARCHS}; do
	mkdir -p "${DERIVED_FILE_DIR}/${a}"
	"${MIG}" -arch $a -header "${DERIVED_FILE_DIR}/${a}/${BASE}" \
		-sheader /dev/null -user /dev/null \
		-server /dev/null "${SCRIPT_INPUT_FILE_0}"
done
