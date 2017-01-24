#!/bin/bash
set -x

# Skip installing headers during Xcode build (buildit uses installhdrs+install)
if [ "$ACTION" == build ]; then exit 0; fi

# Installs Libc header files

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

INCDIR=${DSTROOT}/${PUBLIC_HEADERS_FOLDER_PATH}
LOCINCDIR=${DSTROOT}/${PRIVATE_HEADERS_FOLDER_PATH}
SYSTEMFRAMEWORK=${DSTROOT}/System/Library/Frameworks/System.framework
KERNELFRAMEWORK=${DSTROOT}/System/Library/Frameworks/Kernel.framework

PRIVHDRS=${SYSTEMFRAMEWORK}/Versions/B/PrivateHeaders
PRIVKERNELHDRS=${KERNELFRAMEWORK}/Versions/A/PrivateHeaders
INSTALLMODE=$([[ `id -u` -eq 0 ]] && echo 444 || echo 644)

INSTHDRS=(
	${SRCROOT}/gen/get_compat.h
	${SRCROOT}/gen/execinfo.h
)

INC_INSTHDRS=(
	_locale.h
	_types.h
	_wctype.h
	_xlocale.h
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

INC_SECURE_INSTHDRS=( _common.h _string.h _stdio.h )
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

${MKDIR} ${INCDIR}/arpa
${MKDIR} ${INCDIR}/libkern
${MKDIR} ${INCDIR}/malloc
${MKDIR} ${INCDIR}/protocols
${MKDIR} ${INCDIR}/secure
${MKDIR} ${INCDIR}/sys
${MKDIR} ${INCDIR}/xlocale
${MKDIR} ${INCDIR}/_types
${INSTALL} -m ${INSTALLMODE} ${INSTHDRS[@]} ${INCDIR}
${INSTALL} -m ${INSTALLMODE} ${ARPA_INSTHDRS[@]} ${INCDIR}/arpa
if [ "x${FEATURE_MEM_NOTIFICATION_APIS}" == "x1" ]; then
${INSTALL} -m ${INSTALLMODE} ${MEM_INSTHDRS[@]} ${INCDIR}/libkern
fi
if [ "x${FEATURE_THERM_NOTIFICATION_APIS}" == "x1" ]; then
${INSTALL} -m ${INSTALLMODE} ${THERM_INSTHDRS[@]} ${INCDIR}/libkern
fi
${INSTALL} -m ${INSTALLMODE} ${PROTO_INSTHDRS[@]} ${INCDIR}/protocols
${INSTALL} -m ${INSTALLMODE} ${SECURE_INSTHDRS[@]} ${INCDIR}/secure
${INSTALL} -m ${INSTALLMODE} ${SYS_INSTHDRS[@]} ${INCDIR}/sys
${INSTALL} -m ${INSTALLMODE} ${XLOCALE_INSTHDRS[@]} ${INCDIR}/xlocale
${INSTALL} -m ${INSTALLMODE} ${TYPES_INSTHDRS[@]} ${INCDIR}/_types
${MKDIR} ${LOCINCDIR}
${MKDIR} ${LOCINCDIR}/os
${INSTALL} -m ${INSTALLMODE} ${LOCALHDRS[@]} ${LOCINCDIR}
${INSTALL} -m ${INSTALLMODE} ${OS_LOCALHDRS[@]} ${LOCINCDIR}/os
${MKDIR} ${PRIVHDRS}/btree
${MKDIR} ${PRIVHDRS}/machine
${MKDIR} ${PRIVHDRS}/uuid
${MKDIR} ${PRIVHDRS}/sys
${MKDIR} ${PRIVKERNELHDRS}/uuid
${INSTALL} -m ${INSTALLMODE} ${PRIV_INSTHDRS[@]} ${PRIVHDRS}
${INSTALL} -m ${INSTALLMODE} ${PRIV_BTREEHDRS[@]} ${PRIVHDRS}/btree
${MV} ${INCDIR}/asm.h ${PRIVHDRS}/machine
${INSTALL} -m ${INSTALLMODE} ${SYS_INSTHDRS[@]} ${PRIVHDRS}/sys
${INSTALL} -m ${INSTALLMODE} ${PRIVUUID_INSTHDRS[@]} ${PRIVHDRS}/uuid
${INSTALL} -m ${INSTALLMODE} ${PRIVUUID_INSTHDRS[@]} ${PRIVKERNELHDRS}/uuid

for i in `${FIND} "${DSTROOT}" -name \*.h -print0 | ${XARGS} -0 ${GREP} -l '^//Begin-Libc'`; do
	${CHMOD} u+w $i &&
	${ECHO} ${ED} - $i \< ${SRCROOT}/xcodescripts/strip-header.ed &&
	${ED} - $i < ${SRCROOT}/xcodescripts/strip-header.ed &&
	${CHMOD} u-w $i || exit 1;
done
for i in `${FIND} "${DSTROOT}" -name \*.h -print0 | ${XARGS} -0 ${FGREP} -l -e UNIFDEF -e OPEN_SOURCE`; do
	${CHMOD} u+w $i &&
	${CP} $i $i.orig &&
	${ECHO} ${UNIFDEF} ${UNIFDEFARGS} $i.orig \> $i &&
	{ ${UNIFDEF} ${UNIFDEFARGS} $i.orig > $i || [ $? -ne 2 ]; } &&
	${RM} $i.orig &&
	${CHMOD} u-w $i || exit 1;
done

exit 0
