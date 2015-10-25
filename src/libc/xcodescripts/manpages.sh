#!/bin/bash -e

if [ "$ACTION" = installhdrs ]; then exit 0; fi
if [ "$PLATFORM_NAME" = iphoneos ]; then exit 0; fi
if [ "$PLATFORM_NAME" = iphonesimulator ]; then exit 0; fi

UNIFDEF_FLAGS=`${SRCROOT}/xcodescripts/generate_features.pl --unifdef`
MANPAGES_LIST="${SRCROOT}/man/manpages.lst"
FILES=$(find -E ${SRCROOT} -regex '.*/[^.]+\.[0-9]' -type f)

cat ${MANPAGES_LIST} | grep -v -E '(^#|^\s*$)' | while read first solid rest; do
	SOURCE=$(grep -E "/${first}$"<<EOF
${FILES}
EOF
)
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

# grrr, uuid special case
for page in libuuid.3 uuid_clear.3 uuid_compare.3 uuid_copy.3 uuid_generate.3 uuid_is_null.3 uuid_parse.3 uuid_unparse.3; do
	SECTION=$(echo ${page} | tail -c 2)
	DESTDIR=${DSTROOT}/usr/share/man/man${SECTION}
	DEST=${DESTDIR}/${page}

	# libuuid.3 -> uuid.3
	[[ "${page}" == "libuuid.3" ]] && DEST=${DESTDIR}/uuid.3

	sed -f ${SRCROOT}/uuid/uuidman.sed ${SRCROOT}/uuid/uuidsrc/${page}.in > ${DEST}
done

# and because uuid pages are special cased, so are the links
for link in uuid_generate_random.3 uuid_generate_time.3; do
	SECTION=$(echo ${link} | tail -c 2)
	ln -sf uuid_generate.3 ${DSTROOT}/usr/share/man/man${SECTION}/${link}
done

for link in uuid_unparse_lower.3 uuid_unparse_upper.3; do
	SECTION=$(echo ${link} | tail -c 2)
	ln -sf uuid_unparse.3 ${DSTROOT}/usr/share/man/man${SECTION}/${link}
done
