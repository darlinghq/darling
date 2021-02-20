#!/bin/bash
set -x

# Installs Libc header files

if [ -n "${DRIVERKIT}" -a -z "${DRIVERKITSDK}" ]; then
	# Run script in the mode that installs public DriverKit SDK headers first:
	# required to get the correct header unifdef ordering, that mode strips out more
	# and rewrites all headers under the parent directory (/System/DriverKit)
	DRIVERKITSDK=1 SDK_INSTALL_HEADERS_ROOT="${SDK_INSTALL_ROOT}" "${BASH}" -e "$0"
fi

MKDIR="mkdir -p"
INSTALL=install
MV=mv
ECHO=echo
CHMOD=chmod
CP=cp
UNIFDEF=unifdef
FIND=find
RM=rm
ED=ed
XARGS=xargs
GREP=grep
FGREP=fgrep

eval $(${SRCROOT}/xcodescripts/generate_features.pl --bash)
UNIFDEFARGS=$(${SRCROOT}/xcodescripts/generate_features.pl --unifdef)

if [[ "${DEPLOYMENT_LOCATION}" == "NO" ]] ; then
    HDRROOT=${BUILT_PRODUCTS_DIR}
else
    HDRROOT=${DSTROOT}
fi

INCDIR=${HDRROOT}/${SDK_INSTALL_HEADERS_ROOT}/usr/include
LOCINCDIR=${HDRROOT}/${SDK_INSTALL_HEADERS_ROOT}/usr/local/include
SYSTEMFRAMEWORK=${HDRROOT}/${SDK_INSTALL_HEADERS_ROOT}/System/Library/Frameworks/System.framework
KERNELFRAMEWORK=${HDRROOT}/${SDK_INSTALL_HEADERS_ROOT}/System/Library/Frameworks/Kernel.framework

PRIVHDRS=${SYSTEMFRAMEWORK}/Versions/B/PrivateHeaders
PRIVKERNELHDRS=${KERNELFRAMEWORK}/Versions/A/PrivateHeaders
INSTALLMODE=$([[ `id -u` -eq 0 ]] && echo 444 || echo 644)

if [ -z "${DRIVERKITSDK}" ]; then

INSTHDRS=(
	${SRCROOT}/gen/get_compat.h
	${SRCROOT}/gen/execinfo.h
)

INC_INSTHDRS=(
	__wctype.h
	_ctype.h
	_locale.h
	_regex.h
	_stdio.h
	_types.h
	_wctype.h
	_xlocale.h
	_ctermid.h
	aio.h
	alloca.h
	ar.h
	assert.h
	asm.h
	bitstring.h
	cpio.h
	crt_externs.h
	ctype.h
	db.h
	dirent.h
	disktab.h
	err.h
	errno.h
	fcntl.h
	fmtmsg.h
	fnmatch.h
	fsproperties.h
	fstab.h
	fts.h
	ftw.h
	getopt.h
	glob.h
	inttypes.h
	iso646.h
	langinfo.h
	libc.h
	libgen.h
	limits.h
	locale.h
	memory.h
	monetary.h
	monitor.h
	mpool.h
	ndbm.h
	nlist.h
	paths.h
	printf.h
	poll.h
	ranlib.h
	readpassphrase.h
	regex.h
	runetype.h
	search.h
	semaphore.h
	sgtty.h
	signal.h
	stab.h
	standards.h
	stddef.h
	stdio.h
	stdint.h
	stdlib.h
	strhash.h
	string.h
	stringlist.h
	strings.h
	struct.h
	sysexits.h
	syslog.h
	tar.h
	termios.h
	time.h
	timeconv.h
	ttyent.h
	ulimit.h
	unistd.h
	util.h
	utime.h
	vis.h
	wchar.h
	wctype.h
	wordexp.h
	xlocale.h
)
if [ "x${FEATURE_LEGACY_RUNE_APIS}" == "x1" ]; then
	INC_INSTHDRS=( "${INC_INSTHDRS[@]}" rune.h )
fi
if [ "x${FEATURE_LEGACY_UTMP_APIS}" == "x1" ]; then
	INC_INSTHDRS=( "${INC_INSTHDRS[@]}" utmp.h )
fi

INC_INSTHDRS=( 
	"${INC_INSTHDRS[@]/#/${SRCROOT}/include/}" 
	${SRCROOT}/include/FreeBSD/nl_types.h
	${SRCROOT}/include/NetBSD/utmpx.h
	${SRCROOT}/stdtime/FreeBSD/tzfile.h
)
INSTHDRS=( "${INSTHDRS[@]}" "${INC_INSTHDRS[@]}" )

INC_ARPA_INSTHDRS=( ftp.h inet.h nameser_compat.h telnet.h tftp.h )
ARPA_INSTHDRS=( "${INC_ARPA_INSTHDRS[@]/#/${SRCROOT}/include/arpa/}" )

if [ "x${FEATURE_THERM_NOTIFICATION_APIS}" == "x1" ]; then
	INC_THERM_INSTHDRS=( OSThermalNotification.h )
	THERM_INSTHDRS=( "${INC_THERM_INSTHDRS[@]/#/${SRCROOT}/include/libkern/}" )
fi

INC_PROTO_INSTHDRS=( routed.h rwhod.h talkd.h timed.h )
PROTO_INSTHDRS=( "${INC_PROTO_INSTHDRS[@]/#/${SRCROOT}/include/protocols/}" )

INC_SECURE_INSTHDRS=( _common.h _string.h _strings.h _stdio.h )
SECURE_INSTHDRS=( "${INC_SECURE_INSTHDRS[@]/#/${SRCROOT}/include/secure/}" )

SYS_INSTHDRS=( ${SRCROOT}/include/sys/acl.h ${SRCROOT}/include/sys/statvfs.h )

INC_XLOCALE_INSTHDRS=(
	__wctype.h
	_ctype.h
	_inttypes.h
	_langinfo.h
	_monetary.h
	_regex.h
	_stdio.h
	_stdlib.h
	_string.h
	_time.h
	_wchar.h
	_wctype.h
)
XLOCALE_INSTHDRS=( "${INC_XLOCALE_INSTHDRS[@]/#/${SRCROOT}/include/xlocale/}" )

TYPES_INSTHDRS=(
	${SRCROOT}/include/_types/_intmax_t.h
	${SRCROOT}/include/_types/_nl_item.h
	${SRCROOT}/include/_types/_uint16_t.h
	${SRCROOT}/include/_types/_uint32_t.h
	${SRCROOT}/include/_types/_uint64_t.h
	${SRCROOT}/include/_types/_uint8_t.h
	${SRCROOT}/include/_types/_uintmax_t.h
	${SRCROOT}/include/_types/_wctrans_t.h
	${SRCROOT}/include/_types/_wctype_t.h
)

LOCALHDRS=(
	${SRCROOT}/darwin/libc_private.h
	${SRCROOT}/gen/utmpx_thread.h
	${SRCROOT}/nls/FreeBSD/msgcat.h
	${SRCROOT}/gen/thread_stack_pcs.h
	${SRCROOT}/libdarwin/h/dirstat.h
)

OS_LOCALHDRS=( ${SRCROOT}/os/assumes.h ${SRCROOT}/os/debug_private.h )

PRIV_INSTHDRS=(
	${SRCROOT}/stdlib/FreeBSD/atexit.h
)

PRIV_BTREEHDRS=(
	${SRCROOT}/db/btree/FreeBSD/btree.h
	${SRCROOT}/db/btree/FreeBSD/bt_extern.h
)

SYS_INSTHDRS=(
	${SRCROOT}/include/sys/acl.h
	${SRCROOT}/include/sys/rbtree.h
	${SRCROOT}/include/sys/statvfs.h
)
PRIVUUID_INSTHDRS=( ${SRCROOT}/uuid/namespace.h )

else # DRIVERKITSDK

# Public DriverKit SDK headers

UNIFDEFARGS="${UNIFDEFARGS} -U_USE_EXTENDED_LOCALES_"

INC_INSTHDRS=(
	__wctype.h
	_ctype.h
	_locale.h
	_stdio.h
	_types.h
	_wctype.h
	alloca.h
	assert.h
	ctype.h
	inttypes.h
	limits.h
	locale.h
	runetype.h
	stddef.h
	stdio.h
	stdint.h
	stdlib.h
	string.h
	strings.h
	time.h
	wchar.h
	wctype.h
)

TYPES_INSTHDRS=(
	${SRCROOT}/include/_types/_intmax_t.h
	${SRCROOT}/include/_types/_uint16_t.h
	${SRCROOT}/include/_types/_uint32_t.h
	${SRCROOT}/include/_types/_uint64_t.h
	${SRCROOT}/include/_types/_uint8_t.h
	${SRCROOT}/include/_types/_uintmax_t.h
	${SRCROOT}/include/_types/_wctrans_t.h
	${SRCROOT}/include/_types/_wctype_t.h
)

INC_INSTHDRS=(
	"${INC_INSTHDRS[@]/#/${SRCROOT}/include/}"
)
INSTHDRS=( "${INSTHDRS[@]}" "${INC_INSTHDRS[@]}" )

INC_SECURE_INSTHDRS=( _common.h _string.h _strings.h _stdio.h )
SECURE_INSTHDRS=( "${INC_SECURE_INSTHDRS[@]/#/${SRCROOT}/include/secure/}" )

fi # DRIVERKITSDK

if [ -n "${INSTHDRS}" ]; then
${MKDIR} ${INCDIR}
${INSTALL} -m ${INSTALLMODE} ${INSTHDRS[@]} ${INCDIR}
fi
if [ -n "${ARPA_INSTHDRS}" ]; then
${MKDIR} ${INCDIR}/arpa
${INSTALL} -m ${INSTALLMODE} ${ARPA_INSTHDRS[@]} ${INCDIR}/arpa
fi
if [ -n "${THERM_INSTHDRS}" ]; then
${MKDIR} ${INCDIR}/libkern
${INSTALL} -m ${INSTALLMODE} ${THERM_INSTHDRS[@]} ${INCDIR}/libkern
fi
if [ -n "${PROTO_INSTHDRS}" ]; then
${MKDIR} ${INCDIR}/protocols
${INSTALL} -m ${INSTALLMODE} ${PROTO_INSTHDRS[@]} ${INCDIR}/protocols
fi
if [ -n "${SECURE_INSTHDRS}" ]; then
${MKDIR} ${INCDIR}/secure
${INSTALL} -m ${INSTALLMODE} ${SECURE_INSTHDRS[@]} ${INCDIR}/secure
fi
if [ -n "${SYS_INSTHDRS}" ]; then
${MKDIR} ${INCDIR}/sys
${INSTALL} -m ${INSTALLMODE} ${SYS_INSTHDRS[@]} ${INCDIR}/sys
fi
if [ -n "${XLOCALE_INSTHDRS}" ]; then
${MKDIR} ${INCDIR}/xlocale
${INSTALL} -m ${INSTALLMODE} ${XLOCALE_INSTHDRS[@]} ${INCDIR}/xlocale
fi
if [ -n "${TYPES_INSTHDRS}" ]; then
${MKDIR} ${INCDIR}/_types
${INSTALL} -m ${INSTALLMODE} ${TYPES_INSTHDRS[@]} ${INCDIR}/_types
fi
if [ -n "${LOCALHDRS}" ]; then
${MKDIR} ${LOCINCDIR}
${INSTALL} -m ${INSTALLMODE} ${LOCALHDRS[@]} ${LOCINCDIR}
fi
if [ -n "${OS_LOCALHDRS}" ]; then
${MKDIR} ${LOCINCDIR}/os
${INSTALL} -m ${INSTALLMODE} ${OS_LOCALHDRS[@]} ${LOCINCDIR}/os
fi
if [ -n "${PRIV_INSTHDRS}" ]; then
${MKDIR} ${PRIVHDRS}
${INSTALL} -m ${INSTALLMODE} ${PRIV_INSTHDRS[@]} ${PRIVHDRS}
fi
if [ -n "${PRIV_BTREEHDRS}" ]; then
${MKDIR} ${PRIVHDRS}/btree
${INSTALL} -m ${INSTALLMODE} ${PRIV_BTREEHDRS[@]} ${PRIVHDRS}/btree
fi
if [ -n "${SYS_INSTHDRS}" ]; then
${MKDIR} ${PRIVHDRS}/sys
${INSTALL} -m ${INSTALLMODE} ${SYS_INSTHDRS[@]} ${PRIVHDRS}/sys
fi
if [ -n "${PRIVUUID_INSTHDRS}" ]; then
${MKDIR} ${PRIVHDRS}/uuid
${INSTALL} -m ${INSTALLMODE} ${PRIVUUID_INSTHDRS[@]} ${PRIVHDRS}/uuid
${MKDIR} ${PRIVKERNELHDRS}/uuid
${INSTALL} -m ${INSTALLMODE} ${PRIVUUID_INSTHDRS[@]} ${PRIVKERNELHDRS}/uuid
fi
if [ -f "${INCDIR}/asm.h" ]; then
${MKDIR} ${PRIVHDRS}/machine
${MV} ${INCDIR}/asm.h ${PRIVHDRS}/machine
fi

for i in `${FIND} "${HDRROOT}/${SDK_INSTALL_HEADERS_ROOT}" -name \*.h -print0 | ${XARGS} -0 ${GREP} -l '^//Begin-Libc'`; do
	${CHMOD} u+w $i &&
	${ECHO} ${ED} - $i \< ${SRCROOT}/xcodescripts/strip-header.ed &&
	${ED} - $i < ${SRCROOT}/xcodescripts/strip-header.ed &&
	${CHMOD} u-w $i || exit 1;
done
for i in `${FIND} "${HDRROOT}/${SDK_INSTALL_HEADERS_ROOT}" -name \*.h -print0 | ${XARGS} -0 ${FGREP} -l -e UNIFDEF -e OPEN_SOURCE -e _USE_EXTENDED_LOCALES_`; do
	${CHMOD} u+w $i &&
	${CP} $i $i.orig &&
	${ECHO} ${UNIFDEF} ${UNIFDEFARGS} $i.orig \> $i &&
	{ ${UNIFDEF} ${UNIFDEFARGS} $i.orig > $i || [ $? -ne 2 ]; } &&
	${RM} $i.orig &&
	${CHMOD} u-w $i || exit 1;
done

exit 0
