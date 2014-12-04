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

if [ "$PLATFORM_NAME" = iphonesimulator ]; then
	SYSTEMFRAMEWORK=${DSTROOT}/${SDKROOT}/System/Library/Frameworks/System.framework
	KERNELFRAMEWORK=${DSTROOT}/${SDKROOT}/System/Library/Frameworks/Kernel.framework
fi

PRIVHDRS=${SYSTEMFRAMEWORK}/Versions/B/PrivateHeaders
PRIVKERNELHDRS=${KERNELFRAMEWORK}/Versions/A/PrivateHeaders
INSTALLMODE=$([[ `id -u` -eq 0 ]] && echo 444 || echo 644)

INSTHDRS=(
	${SRCROOT}/darwin/libproc.h
	${SRCROOT}/gen/get_compat.h
	${SRCROOT}/gen/execinfo.h
)

INC_INSTHDRS=(
	NSSystemDirectories.h _locale.h _structs.h _types.h _wctype.h _xlocale.h aio.h alloca.h \
	ar.h asl.h assert.h asm.h bitstring.h cpio.h crt_externs.h ctype.h db.h dirent.h disktab.h err.h \
	errno.h fcntl.h fmtmsg.h fnmatch.h fsproperties.h fstab.h fts.h ftw.h getopt.h glob.h inttypes.h \
	iso646.h langinfo.h libc.h libgen.h limits.h locale.h memory.h monetary.h monitor.h mpool.h ndbm.h \
	nlist.h paths.h poll.h ranlib.h readpassphrase.h regex.h runetype.h search.h \
	semaphore.h setjmp.h sgtty.h signal.h spawn.h stab.h standards.h stdbool.h stddef.h stdio.h stdint.h \
	stdlib.h strhash.h string.h stringlist.h strings.h struct.h sysexits.h syslog.h tar.h termios.h time.h \
	timeconv.h ttyent.h ucontext.h ulimit.h unistd.h util.h utime.h vis.h wchar.h wctype.h \
	wordexp.h xlocale.h
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
INC_PTHREADHDRS=(
	${SRCROOT}/pthreads/pthread.h 
	${SRCROOT}/pthreads/pthread_spis.h 
	${SRCROOT}/pthreads/pthread_impl.h 
	${SRCROOT}/pthreads/sched.h
)
INSTHDRS=( "${INSTHDRS[@]}" "${INC_INSTHDRS[@]}" "${INC_PTHREADHDRS[@]}" )

INC_ARPA_INSTHDRS=( ftp.h nameser_compat.h telnet.h tftp.h )
ARPA_INSTHDRS=( "${INC_ARPA_INSTHDRS[@]/#/${SRCROOT}/include/arpa/}" )

INC_LIBKERN_INSTHDRS=( OSAtomic.h OSCacheControl.h )
if [ "x${FEATURE_MEM_THERM_NOTIFICATION_APIS}" == "x1" ]; then
	INC_LIBKERN_INSTHDRS+=( OSMemoryNotification.h OSThermalNotification.h )
fi
LIBKERN_INSTHDRS=( "${INC_LIBKERN_INSTHDRS[@]/#/${SRCROOT}/include/libkern/}" )

MALLOC_INSTHDRS=( ${SRCROOT}/include/malloc/malloc.h )

INC_PROTO_INSTHDRS=(routed.h rwhod.h talkd.h timed.h )
PROTO_INSTHDRS=( "${INC_PROTO_INSTHDRS[@]/#/${SRCROOT}/include/protocols/}" )

INC_SECURE_INSTHDRS=( _common.h _string.h _stdio.h )
SECURE_INSTHDRS=( "${INC_SECURE_INSTHDRS[@]/#/${SRCROOT}/include/secure/}" )

SYS_INSTHDRS=( ${SRCROOT}/include/sys/acl.h ${SRCROOT}/include/sys/statvfs.h )

INC_XLOCALE_INSTHDRS=(
	__wctype.h _ctype.h _inttypes.h _langinfo.h _monetary.h _regex.h
	_stdio.h _stdlib.h _string.h _time.h _wchar.h _wctype.h
)
XLOCALE_INSTHDRS=( "${INC_XLOCALE_INSTHDRS[@]/#/${SRCROOT}/include/xlocale/}" )

LOCALHDRS=(
	${SRCROOT}/darwin/dirhelper.defs
	${SRCROOT}/darwin/dirhelper_priv.h
	${SRCROOT}/darwin/libproc.h
	${SRCROOT}/darwin/libproc_internal.h
	${SRCROOT}/gen/asl_core.h 
	${SRCROOT}/gen/asl_file.h 
	${SRCROOT}/gen/asl_ipc.defs 
	${SRCROOT}/gen/asl_legacy1.h 
	${SRCROOT}/gen/asl_msg.h 
	${SRCROOT}/gen/asl_private.h 
	${SRCROOT}/gen/asl_store.h 
	${SRCROOT}/gen/assumes.h 
	${SRCROOT}/gen/_simple.h 
	${SRCROOT}/gen/stack_logging.h 
	${SRCROOT}/gen/utmpx_thread.h
	${SRCROOT}/include/spawn_private.h
	${SRCROOT}/nls/FreeBSD/msgcat.h
	${SRCROOT}/pthreads/pthread_workqueue.h
)

PRIV_INSTHDRS=(
	${SRCROOT}/gen/stack_logging.h
	${SRCROOT}/pthreads/pthread_machdep.h
	${SRCROOT}/stdlib/FreeBSD/atexit.h
)

PRIV_BTREEHDRS=(
	${SRCROOT}/db/btree/FreeBSD/btree.h
	${SRCROOT}/db/btree/FreeBSD/bt_extern.h
)

SYS_INSTHDRS=( ${SRCROOT}/include/sys/acl.h ${SRCROOT}/include/sys/statvfs.h )
PRIVUUID_INSTHDRS=( ${SRCROOT}/uuid/namespace.h )

${MKDIR} ${INCDIR}/arpa
${MKDIR} ${INCDIR}/libkern
${MKDIR} ${INCDIR}/malloc
${MKDIR} ${INCDIR}/protocols
${MKDIR} ${INCDIR}/secure
${MKDIR} ${INCDIR}/sys
${MKDIR} ${INCDIR}/xlocale
${INSTALL} -m ${INSTALLMODE} ${INSTHDRS[@]} ${INCDIR}
${INSTALL} -m ${INSTALLMODE} ${ARPA_INSTHDRS[@]} ${INCDIR}/arpa
${INSTALL} -m ${INSTALLMODE} ${LIBKERN_INSTHDRS[@]} ${INCDIR}/libkern
${INSTALL} -m ${INSTALLMODE} ${MALLOC_INSTHDRS[@]} ${INCDIR}/malloc
${INSTALL} -m ${INSTALLMODE} ${PROTO_INSTHDRS[@]} ${INCDIR}/protocols
${INSTALL} -m ${INSTALLMODE} ${SECURE_INSTHDRS[@]} ${INCDIR}/secure
${INSTALL} -m ${INSTALLMODE} ${SYS_INSTHDRS[@]} ${INCDIR}/sys
${INSTALL} -m ${INSTALLMODE} ${XLOCALE_INSTHDRS[@]} ${INCDIR}/xlocale
${MKDIR} ${LOCINCDIR}
${INSTALL} -m ${INSTALLMODE} ${LOCALHDRS[@]} ${LOCINCDIR}
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
for i in `${FIND} "${DSTROOT}" -name \*.h -print0 | ${XARGS} -0 ${FGREP} -l UNIFDEF`; do
	${CHMOD} u+w $i &&
	${CP} $i $i.orig &&
	${ECHO} ${UNIFDEF} ${UNIFDEFARGS} $i.orig \> $i &&
	{ ${UNIFDEF} ${UNIFDEFARGS} $i.orig > $i || [ $? -ne 2 ]; } &&
	${RM} $i.orig &&
	${CHMOD} u-w $i || exit 1;
done

exit 0
