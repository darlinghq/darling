#!/bin/bash -e

if [ "$ACTION" = installhdrs ]; then exit 0; fi
if [[ "$PLATFORM_NAME" =~ iphone ]]; then exit 0; fi

UNIFDEF_FLAGS="-D__OPEN_SOURCE__"
MANPAGES_LIST="${SRCROOT}/man/manpages.lst"
FILES=$(find -E ${SRCROOT} -regex '.*/[^.]+\.[0-9]' -type f)

cat ${MANPAGES_LIST} | grep -v -E '(^#|^\s*$)' | while read first solid rest; do
	SOURCE=$(grep -E "/${first}$"<<EOF
${FILES}
EOF
)

	# This is a subshell, the real exit is after the loop.
	if [ -z "${SOURCE}" ]; then
		echo "Error: ${first} not found"
		exit 1
	fi

	SECTION=$(echo ${first} | tail -c 2)

	DESTDIR=${DSTROOT}/usr/share/man/man${SECTION}
	DEST=${DESTDIR}/${solid}

	mkdir -p ${DSTROOT}/usr/share/man/man${SECTION}

	# cat is used to keep bash happy, unifdef returns non-zero in some success cases
	cmd="unifdef -t ${UNIFDEF_FLAGS} < ${SOURCE} | cat > ${DEST}"
	echo ${cmd}
	eval ${cmd}

	for link in ${rest}; do
		cmd="ln -sf ${first} ${DESTDIR}/${link}"
		echo ${cmd}
		eval ${cmd}
	done
done

if [ $? -ne 0 ]; then
	echo "Exiting due to previous error(s)."
	exit 1
fi
