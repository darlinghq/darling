AC_PREREQ(2.59)
AC_INIT([Xar], [1.8dev], [xar-devel@googlegroups.com], [xar])
AC_CONFIG_SRCDIR([LICENSE])

dnl Revision number for libxar.
LIB_REV=1
AC_SUBST([LIB_REV])

dnl xar version variables.
XAR_MAJOR_VERSION="1"
XAR_MINOR_VERSION="8dev"
XAR_VERSION="${XAR_MAJOR_VERSION}.${XAR_MINOR_VERSION}"
AC_SUBST([XAR_MAJOR_VERSION])
AC_SUBST([XAR_MINOR_VERSION])
AC_SUBST([XAR_VERSION])

dnl 
dnl Various paths.
dnl 
srcroot=$srcdir
if test "x${srcroot}" = "x." ; then
  srcroot=""
else
  srcroot="${srcroot}/"
fi
AC_SUBST([srcroot])
abs_srcroot="`cd "$srcdir"; pwd`/"
AC_SUBST([abs_srcroot])

objroot=""
AC_SUBST([objroot])
abs_objroot="`pwd`/"
AC_SUBST([abs_objroot])

dnl Set install paths.
if test "x$prefix" = "xNONE" ; then
  prefix="/usr/local"
fi
if test "x$exec_prefix" = "xNONE" ; then
  exec_prefix=$prefix
fi
PREFIX=$prefix
AC_SUBST([PREFIX])
BINDIR=`eval echo $bindir`
BINDIR=`eval echo $BINDIR`
AC_SUBST([BINDIR])
DATADIR=`eval echo $datadir`
DATADIR=`eval echo $DATADIR`
AC_SUBST([DATADIR])
LIBDIR=`eval echo $libdir`
LIBDIR=`eval echo $LIBDIR`
AC_SUBST([LIBDIR])
INCLUDEDIR=`eval echo $includedir`
INCLUDEDIR=`eval echo $INCLUDEDIR`
AC_SUBST([INCLUDEDIR])
MANDIR=`eval echo $mandir`
MANDIR=`eval echo $MANDIR`
AC_SUBST([MANDIR])

cfgoutputs="cfgoutputs.stamp"
cfgoutputs="${cfgoutputs} Makefile"
cfgoutputs="${cfgoutputs} include/xar.h"
cfgoutputs="${cfgoutputs} lib/Makefile.inc"
cfgoutputs="${cfgoutputs} lib/libxar.la.in"
cfgoutputs="${cfgoutputs} src/Makefile.inc"
cfgoutputs="${cfgoutputs} xar.spec"

cfghdrs="${objroot}cfghdrs.stamp"
cfghdrs="${cfghdrs} ${objroot}include/config.h"

dnl If CFLAGS isn't defined and using gcc, set CFLAGS to something reasonable.
dnl Otherwise, just prevent autoconf from molesting CFLAGS.
CFLAGS=$CFLAGS
AC_PROG_CC
AC_SYS_LARGEFILE
if test "x$CFLAGS" = "x" ; then
  no_CFLAGS="yes"
fi
if test "x$no_CFLAGS" = "xyes" -a "x$GCC" = "xyes" ; then
  CFLAGS="-Wall -g"
fi
dnl Append EXTRA_CFLAGS to CFLAGS, if defined.
if test "x$EXTRA_CFLAGS" != "x" ; then
  CFLAGS="$CFLAGS $EXTRA_CFLAGS"
fi
AC_PROG_CPP

AC_PROG_INSTALL
AC_PATH_PROG([LD], [ld], , [$PATH])
AC_PATH_PROG([AR], [ar], , [$PATH])
AC_PATH_PROG([RANLIB], [ranlib], , [$PATH])
AC_PATH_PROG([AUTOCONF], [autoconf], , [$PATH])

dnl Some libtool envy
#AC_ENABLE_SHARED
AC_MSG_CHECKING([whether to build shared libraries])
AC_ARG_ENABLE([shared],
    [AC_HELP_STRING([--enable-shared],
	[build shared libraries @<:@default=yes@:>@])],
	[shared=$enableval],[shared=yes])
AC_MSG_RESULT($shared)
AC_SUBST([shared])
#AC_ENABLE_STATIC
AC_MSG_CHECKING([whether to build static libraries])
AC_ARG_ENABLE([static],
    [AC_HELP_STRING([--enable-static],
	[build static libraries @<:@default=yes@:>@])],
	[static=$enableval],[static=yes])
AC_MSG_RESULT($static)
AC_SUBST([static])
# Make sure either enable_shared or enable_static is yes.
test "$shared" = "yes" || test "$static" = "yes"

dnl Platform-specific settings.  abi and RPATH can probably be determined
dnl programmatically, but doing so is error-prone, which makes it generally
dnl not worth the trouble.
dnl 
dnl Define cpp macros in CPPFLAGS, rather than doing AC_DEFINE(macro), since the
dnl definitions need to be seen before and headers are included, which is a pain
dnl to make happen otherwise.
AC_CANONICAL_HOST
case "${host}" in
  *-*-darwin*)
	abi="macho"
	RPATH=""
	;;
  *-*-freebsd*)
	CFLAGS="$CFLAGS"
	abi="elf"
	RPATH="-Wl,-rpath,"
	;;
  *-*-linux*)
	CFLAGS="$CFLAGS"
	abi="elf"
	dnl Linux needs this for things like asprintf() and poll() flags.
	CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
	RPATH="-Wl,-rpath,"
	;;
  *-*-netbsd*)
	AC_MSG_CHECKING([ABI])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#ifdef __ELF__
/* ELF */
#else
#error aout
#endif
]])],
                          [CFLAGS="$CFLAGS"; abi="elf"],
                          [abi="aout"])
	AC_MSG_RESULT([$abi])
	RPATH="-Wl,-rpath,"
	;;
  *-*-solaris2*)
	CFLAGS="$CFLAGS"
	abi="elf"
	RPATH="-Wl,-R,"
	dnl XXX Remove the following if it ends up not being useful.
	dnl Solaris needs this for sigwait().
	dnl CPPFLAGS="$CPPFLAGS -D_POSIX_PTHREAD_SEMANTICS"
	dnl LIBS="$LIBS -lposix4 -lsocket -lnsl"
	;;
  *)
	AC_MSG_RESULT([Unsupported operating system: ${host}])
	abi="elf"
	RPATH="-Wl,-rpath,"
	;;
esac
AC_SUBST([abi])
AC_SUBST([RPATH])

dnl Disable rules that do automatic regeneration of configure output by default.
AC_ARG_ENABLE([autogen],
              [  --enable-autogen        Automatically regenerate configure output],
[if test "x$enable_autogen" = "xno" ; then
  enable_autogen="0"
else
  enable_autogen="1"
fi
],
[enable_autogen="0"]
)
AC_SUBST([enable_autogen])

AC_TRY_COMPILE([#include <sys/types.h> 
#include <sys/acl.h>], [acl_t a], [AC_DEFINE([HAVE_SYS_ACL_H],[1], [define if you have sys/acl.h and it has a working acl_t type])])
AC_CHECK_HEADERS(ext2fs/ext2_fs.h sys/statfs.h sys/xattr.h sys/param.h sys/extattr.h libutil.h)
AC_CHECK_FUNCS(lgetxattr)
AC_CHECK_FUNCS(lsetxattr)
AC_CHECK_FUNCS(getxattr)
AC_CHECK_FUNCS(setxattr)
AC_CHECK_FUNCS(getattrlist)
AC_CHECK_FUNCS(setattrlist)
AC_CHECK_FUNCS(lchmod)
AC_CHECK_FUNCS(lchown)
AC_CHECK_FUNCS(chflags)
AC_CHECK_FUNCS(statvfs)
AC_CHECK_FUNCS(statfs)
AC_CHECK_FUNCS(strmode)

AC_CHECK_MEMBERS([struct statfs.f_fstypename],,,[#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>])
AC_CHECK_MEMBERS([struct statvfs.f_fstypename],,,[#include <sys/statvfs.h>])
AC_CHECK_MEMBERS([struct stat.st_flags])

AC_CHECK_SIZEOF(uid_t)
if test $ac_cv_sizeof_uid_t = "4"; then
AC_DEFINE(UID_STRING, RId32)
AC_DEFINE(UID_CAST, (uint32_t))
elif test $ac_cv_sizeof_uid_t = "8"; then
AC_DEFINE(UID_STRING, PRId64)
AC_DEFINE(UID_CAST, (uint64_t))
else
AC_ERROR(can not detect the size of your system's uid_t type)
fi

AC_CHECK_SIZEOF(gid_t)
if test $ac_cv_sizeof_gid_t = "4"; then
AC_DEFINE(GID_STRING, PRId32)
AC_DEFINE(GID_CAST, (uint32_t))
elif test $ac_cv_sizeof_gid_t = "8"; then
AC_DEFINE(GID_STRING, PRId64)
AC_DEFINE(GID_CAST, (uint64_t))
else
AC_ERROR(can not detect the size of your system's gid_t type)
fi

AC_CHECK_SIZEOF(ino_t)
if test $ac_cv_sizeof_ino_t = "4"; then
AC_DEFINE(INO_STRING, PRId32)
AC_DEFINE(INO_HEXSTRING, PRIx32)
AC_DEFINE(INO_CAST, (uint32_t))
elif test $ac_cv_sizeof_ino_t = "8"; then
AC_DEFINE(INO_STRING, PRId64)
AC_DEFINE(INO_HEXSTRING, PRIx64)
AC_DEFINE(INO_CAST, (uint64_t))
else
AC_ERROR(can not detect the size of your system's ino_t type)
fi

AC_CHECK_SIZEOF(dev_t)
if test $ac_cv_sizeof_dev_t = "4"; then
AC_DEFINE(DEV_STRING, PRId32)
AC_DEFINE(DEV_HEXSTRING, PRIx32)
AC_DEFINE(DEV_CAST, (uint32_t))
elif test $ac_cv_sizeof_dev_t = "8"; then
AC_DEFINE(DEV_STRING, PRId64)
AC_DEFINE(DEV_HEXSTRING, PRIx64)
AC_DEFINE(DEV_CAST, (uint64_t))
else
AC_ERROR(can not detect the size of your system's dev_t type)
fi

AC_CHECK_LIB(acl, acl_get_file)

dnl Check for paths
AC_PREFIX_DEFAULT(/usr/local)

AC_CHECK_FUNC([asprintf], AC_DEFINE([HAVE_ASPRINTF]))

dnl 
dnl Configure libxml2.
dnl 
LIBXML2_VERSION_MIN=2.6.11

have_libxml2="1"

AC_ARG_WITH([xml2-config], [  --with-xml2-config      libxml2 config program],
if test "x${with_xml2_config}" = "xno" ; then
  XML2_CONFIG=
else
  XML2_CONFIG="${with_xml2_config}"
fi
,
  XML2_CONFIG=
)
if test "x${XML2_CONFIG}" != "x" ; then
   if test ! -x "${XML2_CONFIG}" ; then
     AC_MSG_ERROR([Unusable or missing xml2-config: ${XML2_CONFIG}])
   fi
else
  AC_PATH_PROG([XML2_CONFIG], [xml2-config], , [${PATH}])
  if test "x${XML2_CONFIG}" = "x" ; then
    AC_MSG_ERROR([Cannot configure without xml2-config])
  fi
fi

dnl Make sure the version of libxml2 found is sufficient.
AC_MSG_CHECKING([for libxml >= ${LIBXML2_VERSION_MIN}])
LIBXML2_FOUND=`2>&1 ${XML2_CONFIG} --version`
LIBXML2_MAJOR=`echo ${LIBXML2_FOUND} | tr . " " | awk '{print $1}'`
LIBXML2_MINOR=`echo ${LIBXML2_FOUND} | tr . " " | awk '{print $2}' | tr a-z " " |awk '{print $1}'`
LIBXML2_BRANCH=`echo ${LIBXML2_FOUND} | tr . " " | awk '{print $3}' | tr a-z " " |awk '{print $1}'`
if test "x${LIBXML2_BRANCH}" = "x" ; then
  LIBXML2_BRANCH=0
fi
LIBXML2_MAJOR_MIN=`echo ${LIBXML2_VERSION_MIN} | tr . " " | awk '{print $1}'`
LIBXML2_MINOR_MIN=`echo ${LIBXML2_VERSION_MIN} | tr . " " | awk '{print $2}'`
LIBXML2_BRANCH_MIN=`echo ${LIBXML2_VERSION_MIN} | tr . " " | awk '{print $3}'`
if test ${LIBXML2_MAJOR} -gt ${LIBXML2_MAJOR_MIN} \
     -o ${LIBXML2_MAJOR} -eq ${LIBXML2_MAJOR_MIN} \
     -a ${LIBXML2_MINOR} -gt ${LIBXML2_MINOR_MIN} \
     -o ${LIBXML2_MAJOR} -eq ${LIBXML2_MAJOR_MIN} \
     -a ${LIBXML2_MINOR} -eq ${LIBXML2_MINOR_MIN} \
     -a ${LIBXML2_BRANCH} -ge $LIBXML2_BRANCH_MIN ; then
  AC_MSG_RESULT([${LIBXML2_MAJOR}.${LIBXML2_MINOR}.${LIBXML2_BRANCH}])
  have_libxml2="1"
  CPPFLAGS="${CPPFLAGS} `${XML2_CONFIG} --cflags`"
  LIBS="${LIBS} `${XML2_CONFIG} --libs`"
else
  AC_MSG_RESULT([no])
  have_libxml2="0"
fi
if test "x${have_libxml2}" = "x1" ; then
  dnl Final sanity check, to make sure that xmlwriter is present.
  AC_CHECK_HEADER([libxml/xmlwriter.h], , [have_libxml2="0"])
fi
if test "x${have_libxml2}" = "x0" ; then
  AC_MSG_ERROR([Cannot build without libxml2])
fi

dnl 
dnl Configure libz.
dnl 
have_libz="1"
AC_CHECK_HEADERS([zlib.h], , [have_libz="0"])
AC_CHECK_LIB([z], [deflate], , [have_libz="0"])
if test "x${have_libz}" = "x0" ; then
  AC_MSG_ERROR([Cannot build without libz])
fi

dnl 
dnl Configure libbz2.
dnl 
have_libbz2="1"
AC_CHECK_HEADERS([bzlib.h], , [have_libbz2="0"])
AC_CHECK_LIB([bz2], [BZ2_bzCompress], , [have_libbz2="0"])
if test "x${have_libbz2}" = "x1" ; then
  AC_DEFINE([HAVE_LIBBZ2])
fi

dnl 
dnl Configure libpthread.
dnl 
have_libpthread="1"
AC_CHECK_HEADERS([pthread.h], , [have_pthread="0"])
AC_CHECK_LIB([pthread], [pthread_mutex_lock], , [have_pthread="0"])
if test "x${have_pthread}" = "x1" ; then
  AC_DEFINE([HAVE_PTHREAD])
fi

dnl 
dnl Process .in files.
dnl 
AC_SUBST([cfghdrs])
AC_CONFIG_HEADER([$cfghdrs])

AC_SUBST([cfgoutputs])
AC_CONFIG_FILES([$cfgoutputs])

AC_OUTPUT
