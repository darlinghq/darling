dnl***************************************************************************
dnl Copyright (c) 1998-2007,2008 Free Software Foundation, Inc.              *
dnl                                                                          *
dnl Permission is hereby granted, free of charge, to any person obtaining a  *
dnl copy of this software and associated documentation files (the            *
dnl "Software"), to deal in the Software without restriction, including      *
dnl without limitation the rights to use, copy, modify, merge, publish,      *
dnl distribute, distribute with modifications, sublicense, and/or sell       *
dnl copies of the Software, and to permit persons to whom the Software is    *
dnl furnished to do so, subject to the following conditions:                 *
dnl                                                                          *
dnl The above copyright notice and this permission notice shall be included  *
dnl in all copies or substantial portions of the Software.                   *
dnl                                                                          *
dnl THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
dnl OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
dnl MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
dnl IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
dnl DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
dnl OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
dnl THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
dnl                                                                          *
dnl Except as contained in this notice, the name(s) of the above copyright   *
dnl holders shall not be used in advertising or otherwise to promote the     *
dnl sale, use or other dealings in this Software without prior written       *
dnl authorization.                                                           *
dnl***************************************************************************
dnl
dnl Author: Thomas E. Dickey 1995-on
dnl
dnl $Id: aclocal.m4,v 1.470 2008/10/25 22:15:32 tom Exp $
dnl Macros used in NCURSES auto-configuration script.
dnl
dnl These macros are maintained separately from NCURSES.  The copyright on
dnl this file applies to the aggregation of macros and does not affect use of
dnl these macros in other applications.
dnl
dnl See http://invisible-island.net/autoconf/ for additional information.
dnl
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl AM_LANGINFO_CODESET version: 3 updated: 2002/10/27 23:21:42
dnl -------------------
dnl Inserted as requested by gettext 0.10.40
dnl File from /usr/share/aclocal
dnl codeset.m4
dnl ====================
dnl serial AM1
dnl
dnl From Bruno Haible.
AC_DEFUN([AM_LANGINFO_CODESET],
[
  AC_CACHE_CHECK([for nl_langinfo and CODESET], am_cv_langinfo_codeset,
    [AC_TRY_LINK([#include <langinfo.h>],
      [char* cs = nl_langinfo(CODESET);],
      am_cv_langinfo_codeset=yes,
      am_cv_langinfo_codeset=no)
    ])
  if test $am_cv_langinfo_codeset = yes; then
    AC_DEFINE(HAVE_LANGINFO_CODESET, 1,
      [Define if you have <langinfo.h> and nl_langinfo(CODESET).])
  fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADA_INCLUDE_DIRS version: 5 updated: 2006/10/14 15:23:15
dnl -------------------
dnl Construct the list of include-options for the C programs in the Ada95
dnl binding.
AC_DEFUN([CF_ADA_INCLUDE_DIRS],
[
ACPPFLAGS="-I. -I../../include $ACPPFLAGS"
if test "$srcdir" != "."; then
	ACPPFLAGS="-I\${srcdir}/../../include $ACPPFLAGS"
fi
if test "$GCC" != yes; then
	ACPPFLAGS="$ACPPFLAGS -I\${includedir}"
elif test "$includedir" != "/usr/include"; then
	if test "$includedir" = '${prefix}/include' ; then
		if test $prefix != /usr ; then
			ACPPFLAGS="$ACPPFLAGS -I\${includedir}"
		fi
	else
		ACPPFLAGS="$ACPPFLAGS -I\${includedir}"
	fi
fi
AC_SUBST(ACPPFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_CFLAGS version: 7 updated: 2004/04/25 17:48:30
dnl -------------
dnl Copy non-preprocessor flags to $CFLAGS, preprocessor flags to $CPPFLAGS
dnl The second parameter if given makes this macro verbose.
dnl
dnl Put any preprocessor definitions that use quoted strings in $EXTRA_CPPFLAGS,
dnl to simplify use of $CPPFLAGS in compiler checks, etc., that are easily
dnl confused by the quotes (which require backslashes to keep them usable).
AC_DEFUN([CF_ADD_CFLAGS],
[
cf_fix_cppflags=no
cf_new_cflags=
cf_new_cppflags=
cf_new_extra_cppflags=

for cf_add_cflags in $1
do
case $cf_fix_cppflags in
no)
	case $cf_add_cflags in #(vi
	-undef|-nostdinc*|-I*|-D*|-U*|-E|-P|-C) #(vi
		case $cf_add_cflags in
		-D*)
			cf_tst_cflags=`echo ${cf_add_cflags} |sed -e 's/^-D[[^=]]*='\''\"[[^"]]*//'`

			test "${cf_add_cflags}" != "${cf_tst_cflags}" \
			&& test -z "${cf_tst_cflags}" \
			&& cf_fix_cppflags=yes

			if test $cf_fix_cppflags = yes ; then
				cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"
				continue
			elif test "${cf_tst_cflags}" = "\"'" ; then
				cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"
				continue
			fi
			;;
		esac
		case "$CPPFLAGS" in
		*$cf_add_cflags) #(vi
			;;
		*) #(vi
			cf_new_cppflags="$cf_new_cppflags $cf_add_cflags"
			;;
		esac
		;;
	*)
		cf_new_cflags="$cf_new_cflags $cf_add_cflags"
		;;
	esac
	;;
yes)
	cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"

	cf_tst_cflags=`echo ${cf_add_cflags} |sed -e 's/^[[^"]]*"'\''//'`

	test "${cf_add_cflags}" != "${cf_tst_cflags}" \
	&& test -z "${cf_tst_cflags}" \
	&& cf_fix_cppflags=no
	;;
esac
done

if test -n "$cf_new_cflags" ; then
	ifelse($2,,,[CF_VERBOSE(add to \$CFLAGS $cf_new_cflags)])
	CFLAGS="$CFLAGS $cf_new_cflags"
fi

if test -n "$cf_new_cppflags" ; then
	ifelse($2,,,[CF_VERBOSE(add to \$CPPFLAGS $cf_new_cppflags)])
	CPPFLAGS="$cf_new_cppflags $CPPFLAGS"
fi

if test -n "$cf_new_extra_cppflags" ; then
	ifelse($2,,,[CF_VERBOSE(add to \$EXTRA_CPPFLAGS $cf_new_extra_cppflags)])
	EXTRA_CPPFLAGS="$cf_new_extra_cppflags $EXTRA_CPPFLAGS"
fi

AC_SUBST(EXTRA_CPPFLAGS)

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_INCDIR version: 9 updated: 2008/02/09 13:15:34
dnl -------------
dnl Add an include-directory to $CPPFLAGS.  Don't add /usr/include, since it's
dnl redundant.  We don't normally need to add -I/usr/local/include for gcc,
dnl but old versions (and some misinstalled ones) need that.  To make things
dnl worse, gcc 3.x may give error messages if -I/usr/local/include is added to
dnl the include-path).
AC_DEFUN([CF_ADD_INCDIR],
[
if test -n "$1" ; then
  for cf_add_incdir in $1
  do
	while test $cf_add_incdir != /usr/include
	do
	  if test -d $cf_add_incdir
	  then
		cf_have_incdir=no
		if test -n "$CFLAGS$CPPFLAGS" ; then
		  # a loop is needed to ensure we can add subdirs of existing dirs
		  for cf_test_incdir in $CFLAGS $CPPFLAGS ; do
			if test ".$cf_test_incdir" = ".-I$cf_add_incdir" ; then
			  cf_have_incdir=yes; break
			fi
		  done
		fi

		if test "$cf_have_incdir" = no ; then
          if test "$cf_add_incdir" = /usr/local/include ; then
			if test "$GCC" = yes
			then
			  cf_save_CPPFLAGS=$CPPFLAGS
			  CPPFLAGS="$CPPFLAGS -I$cf_add_incdir"
			  AC_TRY_COMPILE([#include <stdio.h>],
				  [printf("Hello")],
				  [],
				  [cf_have_incdir=yes])
			  CPPFLAGS=$cf_save_CPPFLAGS
			fi
		  fi
		fi

		if test "$cf_have_incdir" = no ; then
		  CF_VERBOSE(adding $cf_add_incdir to include-path)
		  ifelse($2,,CPPFLAGS,$2)="-I$cf_add_incdir $ifelse($2,,CPPFLAGS,[$]$2)"

          cf_top_incdir=`echo $cf_add_incdir | sed -e 's%/include/.*$%/include%'`
          test "$cf_top_incdir" = "$cf_add_incdir" && break
          cf_add_incdir="$cf_top_incdir"
		else
		  break
		fi
	  fi
	done
  done
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_LIBDIR version: 6 updated: 2008/02/09 13:15:34
dnl -------------
dnl	Adds to the library-path
dnl
dnl	Some machines have trouble with multiple -L options.
dnl
dnl $1 is the (list of) directory(s) to add
dnl $2 is the optional name of the variable to update (default LDFLAGS)
dnl
AC_DEFUN([CF_ADD_LIBDIR],
[
if test -n "$1" ; then
  for cf_add_libdir in $1
  do
    if test $cf_add_libdir = /usr/lib ; then
      :
    elif test -d $cf_add_libdir
    then
      cf_have_libdir=no
      if test -n "$LDFLAGS$LIBS" ; then
        # a loop is needed to ensure we can add subdirs of existing dirs
        for cf_test_libdir in $LDFLAGS $LIBS ; do
          if test ".$cf_test_libdir" = ".-L$cf_add_libdir" ; then
            cf_have_libdir=yes; break
          fi
        done
      fi
      if test "$cf_have_libdir" = no ; then
        CF_VERBOSE(adding $cf_add_libdir to library-path)
        ifelse($2,,LDFLAGS,$2)="-L$cf_add_libdir $ifelse($2,,LDFLAGS,[$]$2)"
      fi
    fi
  done
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ADD_SUBDIR_PATH version: 2 updated: 2007/07/29 10:12:59
dnl ------------------
dnl Append to a search-list for a nonstandard header/lib-file
dnl	$1 = the variable to return as result
dnl	$2 = the package name
dnl	$3 = the subdirectory, e.g., bin, include or lib
dnl $4 = the directory under which we will test for subdirectories
dnl $5 = a directory that we do not want $4 to match
AC_DEFUN([CF_ADD_SUBDIR_PATH],
[
test "$4" != "$5" && \
test -d "$4" && \
ifelse([$5],NONE,,[(test $5 = NONE || test -d $5) &&]) {
	test -n "$verbose" && echo "	... testing for $3-directories under $4"
	test -d $4/$3 &&          $1="[$]$1 $4/$3"
	test -d $4/$3/$2 &&       $1="[$]$1 $4/$3/$2"
	test -d $4/$3/$2/$3 &&    $1="[$]$1 $4/$3/$2/$3"
	test -d $4/$2/$3 &&       $1="[$]$1 $4/$2/$3"
	test -d $4/$2/$3/$2 &&    $1="[$]$1 $4/$2/$3/$2"
}
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ANSI_CC_CHECK version: 9 updated: 2001/12/30 17:53:34
dnl ----------------
dnl This is adapted from the macros 'fp_PROG_CC_STDC' and 'fp_C_PROTOTYPES'
dnl in the sharutils 4.2 distribution.
AC_DEFUN([CF_ANSI_CC_CHECK],
[
AC_CACHE_CHECK(for ${CC-cc} option to accept ANSI C, cf_cv_ansi_cc,[
cf_cv_ansi_cc=no
cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc
# UnixWare 1.2		(cannot use -Xc, since ANSI/POSIX clashes)
for cf_arg in "-DCC_HAS_PROTOS" \
	"" \
	-qlanglvl=ansi \
	-std1 \
	-Ae \
	"-Aa -D_HPUX_SOURCE" \
	-Xc
do
	CF_ADD_CFLAGS($cf_arg)
	AC_TRY_COMPILE(
[
#ifndef CC_HAS_PROTOS
#if !defined(__STDC__) || (__STDC__ != 1)
choke me
#endif
#endif
],[
	int test (int i, double x);
	struct s1 {int (*f) (int a);};
	struct s2 {int (*f) (double a);};],
	[cf_cv_ansi_cc="$cf_arg"; break])
done
CFLAGS="$cf_save_CFLAGS"
CPPFLAGS="$cf_save_CPPFLAGS"
])

if test "$cf_cv_ansi_cc" != "no"; then
if test ".$cf_cv_ansi_cc" != ".-DCC_HAS_PROTOS"; then
	CF_ADD_CFLAGS($cf_cv_ansi_cc)
else
	AC_DEFINE(CC_HAS_PROTOS)
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ANSI_CC_REQD version: 4 updated: 2008/03/23 14:48:54
dnl ---------------
dnl For programs that must use an ANSI compiler, obtain compiler options that
dnl will make it recognize prototypes.  We'll do preprocessor checks in other
dnl macros, since tools such as unproto can fake prototypes, but only part of
dnl the preprocessor.
AC_DEFUN([CF_ANSI_CC_REQD],
[AC_REQUIRE([CF_ANSI_CC_CHECK])
if test "$cf_cv_ansi_cc" = "no"; then
	AC_MSG_ERROR(
[Your compiler does not appear to recognize prototypes.
You have the following choices:
	a. adjust your compiler options
	b. get an up-to-date compiler
	c. use a wrapper such as unproto])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_AWK_BIG_PRINTF version: 2 updated: 2008/10/04 17:16:18
dnl -----------------
dnl Check if awk can handle big strings using printf.  Some older versions of
dnl awk choke on large strings passed via "%s".
dnl
dnl $1 = desired string size
dnl $2 = variable to set with result
AC_DEFUN([CF_AWK_BIG_PRINTF],
[
    case x$AWK in #(vi
    x)
        eval $2=no
        ;;
    *) #(vi
        if ( ${AWK} 'BEGIN { xx = "x"; while (length(xx) < $1) { xx = xx "x"; }; printf("%s\n", xx); }' \
            | $AWK '{ printf "%d\n", length([$]0); }' | $AWK 'BEGIN { eqls=0; recs=0; } { recs++; if ([$]0 == 12000) eqls++; } END { if (recs != 1 || eqls != 1) exit 1; }' 2>/dev/null >/dev/null ) ; then
            eval $2=yes
        else
            eval $2=no
        fi
        ;;
    esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_BOOL_DECL version: 8 updated: 2004/01/30 15:51:18
dnl ------------
dnl Test if 'bool' is a builtin type in the configured C++ compiler.  Some
dnl older compilers (e.g., gcc 2.5.8) don't support 'bool' directly; gcc
dnl 2.6.3 does, in anticipation of the ANSI C++ standard.
dnl
dnl Treat the configuration-variable specially here, since we're directly
dnl substituting its value (i.e., 1/0).
dnl
dnl $1 is the shell variable to store the result in, if not $cv_cv_builtin_bool
AC_DEFUN([CF_BOOL_DECL],
[
AC_MSG_CHECKING(if we should include stdbool.h)

AC_CACHE_VAL(cf_cv_header_stdbool_h,[
	AC_TRY_COMPILE([],[bool foo = false],
		[cf_cv_header_stdbool_h=0],
		[AC_TRY_COMPILE([
#ifndef __BEOS__
#include <stdbool.h>
#endif
],[bool foo = false],
			[cf_cv_header_stdbool_h=1],
			[cf_cv_header_stdbool_h=0])])])

if test "$cf_cv_header_stdbool_h" = 1
then	AC_MSG_RESULT(yes)
else	AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING([for builtin bool type])

AC_CACHE_VAL(ifelse($1,,cf_cv_builtin_bool,[$1]),[
	AC_TRY_COMPILE([
#include <stdio.h>
#include <sys/types.h>
],[bool x = false],
		[ifelse($1,,cf_cv_builtin_bool,[$1])=1],
		[ifelse($1,,cf_cv_builtin_bool,[$1])=0])
	])

if test "$ifelse($1,,cf_cv_builtin_bool,[$1])" = 1
then	AC_MSG_RESULT(yes)
else	AC_MSG_RESULT(no)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_BOOL_SIZE version: 12 updated: 2006/12/16 12:33:30
dnl ------------
dnl Test for the size of 'bool' in the configured C++ compiler (e.g., a type).
dnl Don't bother looking for bool.h, since it's been deprecated.
dnl
dnl If the current compiler is C rather than C++, we get the bool definition
dnl from <stdbool.h>.
AC_DEFUN([CF_BOOL_SIZE],
[
AC_MSG_CHECKING([for size of bool])
AC_CACHE_VAL(cf_cv_type_of_bool,[
	rm -f cf_test.out
	AC_TRY_RUN([
#include <stdlib.h>
#include <stdio.h>

#if defined(__cplusplus)

#ifdef HAVE_GXX_BUILTIN_H
#include <g++/builtin.h>
#elif HAVE_GPP_BUILTIN_H
#include <gpp/builtin.h>
#elif HAVE_BUILTIN_H
#include <builtin.h>
#endif

#else

#if $cf_cv_header_stdbool_h
#include <stdbool.h>
#endif

#endif

main()
{
	FILE *fp = fopen("cf_test.out", "w");
	if (fp != 0) {
		bool x = true;
		if ((bool)(-x) >= 0)
			fputs("unsigned ", fp);
		if (sizeof(x) == sizeof(int))       fputs("int",  fp);
		else if (sizeof(x) == sizeof(char)) fputs("char", fp);
		else if (sizeof(x) == sizeof(short))fputs("short",fp);
		else if (sizeof(x) == sizeof(long)) fputs("long", fp);
		fclose(fp);
	}
	${cf_cv_main_return:-return}(0);
}
		],
		[cf_cv_type_of_bool=`cat cf_test.out`
		 if test -z "$cf_cv_type_of_bool"; then
		   cf_cv_type_of_bool=unknown
		 fi],
		[cf_cv_type_of_bool=unknown],
		[cf_cv_type_of_bool=unknown])
	])
	rm -f cf_test.out
AC_MSG_RESULT($cf_cv_type_of_bool)
if test "$cf_cv_type_of_bool" = unknown ; then
	case .$NCURSES_BOOL in #(vi
	.auto|.) NCURSES_BOOL=unsigned;;
	esac
	AC_MSG_WARN(Assuming $NCURSES_BOOL for type of bool)
	cf_cv_type_of_bool=$NCURSES_BOOL
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_BUILD_CC version: 6 updated: 2006/10/14 15:23:15
dnl -----------
dnl If we're cross-compiling, allow the user to override the tools and their
dnl options.  The configure script is oriented toward identifying the host
dnl compiler, etc., but we need a build compiler to generate parts of the
dnl source.
dnl
dnl $1 = default for $CPPFLAGS
dnl $2 = default for $LIBS
AC_DEFUN([CF_BUILD_CC],[
AC_REQUIRE([CF_PROG_EXT])
if test "$cross_compiling" = yes ; then

	# defaults that we might want to override
	: ${BUILD_CFLAGS:=''}
	: ${BUILD_CPPFLAGS:='ifelse([$1],,,[$1])'}
	: ${BUILD_LDFLAGS:=''}
	: ${BUILD_LIBS:='ifelse([$2],,,[$2])'}
	: ${BUILD_EXEEXT:='$x'}
	: ${BUILD_OBJEXT:='o'}

	AC_ARG_WITH(build-cc,
		[  --with-build-cc=XXX     the build C compiler ($BUILD_CC)],
		[BUILD_CC="$withval"],
		[AC_CHECK_PROGS(BUILD_CC, gcc cc cl)])
	AC_MSG_CHECKING(for native build C compiler)
	AC_MSG_RESULT($BUILD_CC)

	AC_MSG_CHECKING(for native build C preprocessor)
	AC_ARG_WITH(build-cpp,
		[  --with-build-cpp=XXX    the build C preprocessor ($BUILD_CPP)],
		[BUILD_CPP="$withval"],
		[BUILD_CPP='${BUILD_CC} -E'])
	AC_MSG_RESULT($BUILD_CPP)

	AC_MSG_CHECKING(for native build C flags)
	AC_ARG_WITH(build-cflags,
		[  --with-build-cflags=XXX the build C compiler-flags ($BUILD_CFLAGS)],
		[BUILD_CFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_CFLAGS)

	AC_MSG_CHECKING(for native build C preprocessor-flags)
	AC_ARG_WITH(build-cppflags,
		[  --with-build-cppflags=XXX the build C preprocessor-flags ($BUILD_CPPFLAGS)],
		[BUILD_CPPFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_CPPFLAGS)

	AC_MSG_CHECKING(for native build linker-flags)
	AC_ARG_WITH(build-ldflags,
		[  --with-build-ldflags=XXX the build linker-flags ($BUILD_LDFLAGS)],
		[BUILD_LDFLAGS="$withval"])
	AC_MSG_RESULT($BUILD_LDFLAGS)

	AC_MSG_CHECKING(for native build linker-libraries)
	AC_ARG_WITH(build-libs,
		[  --with-build-libs=XXX   the build libraries (${BUILD_LIBS})],
		[BUILD_LIBS="$withval"])
	AC_MSG_RESULT($BUILD_LIBS)

	# this assumes we're on Unix.
	BUILD_EXEEXT=
	BUILD_OBJEXT=o

	: ${BUILD_CC:='${CC}'}

	if ( test "$BUILD_CC" = "$CC" || test "$BUILD_CC" = '${CC}' ) ; then
		AC_MSG_ERROR([Cross-build requires two compilers.
Use --with-build-cc to specify the native compiler.])
	fi

else
	: ${BUILD_CC:='${CC}'}
	: ${BUILD_CPP:='${CPP}'}
	: ${BUILD_CFLAGS:='${CFLAGS}'}
	: ${BUILD_CPPFLAGS:='${CPPFLAGS}'}
	: ${BUILD_LDFLAGS:='${LDFLAGS}'}
	: ${BUILD_LIBS:='${LIBS}'}
	: ${BUILD_EXEEXT:='$x'}
	: ${BUILD_OBJEXT:='o'}
fi

AC_SUBST(BUILD_CC)
AC_SUBST(BUILD_CPP)
AC_SUBST(BUILD_CFLAGS)
AC_SUBST(BUILD_CPPFLAGS)
AC_SUBST(BUILD_LDFLAGS)
AC_SUBST(BUILD_LIBS)
AC_SUBST(BUILD_EXEEXT)
AC_SUBST(BUILD_OBJEXT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CFG_DEFAULTS version: 7 updated: 2005/09/24 16:15:00
dnl ---------------
dnl Determine the default configuration into which we'll install ncurses.  This
dnl can be overridden by the user's command-line options.  There's two items to
dnl look for:
dnl	1. the prefix (e.g., /usr)
dnl	2. the header files (e.g., /usr/include/ncurses)
dnl We'll look for a previous installation of ncurses and use the same defaults.
dnl
dnl We don't use AC_PREFIX_DEFAULT, because it gets evaluated too soon, and
dnl we don't use AC_PREFIX_PROGRAM, because we cannot distinguish ncurses's
dnl programs from a vendor's.
AC_DEFUN([CF_CFG_DEFAULTS],
[
AC_MSG_CHECKING(for prefix)
if test "x$prefix" = "xNONE" ; then
	case "$cf_cv_system_name" in
		# non-vendor systems don't have a conflict
	openbsd*|freebsd*|linux*|cygwin*|k*bsd*-gnu)
		prefix=/usr
		;;
	*)	prefix=$ac_default_prefix
		;;
	esac
fi
AC_MSG_RESULT($prefix)

if test "x$prefix" = "xNONE" ; then
AC_MSG_CHECKING(for default include-directory)
test -n "$verbose" && echo 1>&AC_FD_MSG
for cf_symbol in \
	$includedir \
	$includedir/ncurses \
	$prefix/include \
	$prefix/include/ncurses \
	/usr/local/include \
	/usr/local/include/ncurses \
	/usr/include \
	/usr/include/ncurses
do
	cf_dir=`eval echo $cf_symbol`
	if test -f $cf_dir/curses.h ; then
	if ( fgrep NCURSES_VERSION $cf_dir/curses.h 2>&1 >/dev/null ) ; then
		includedir="$cf_symbol"
		test -n "$verbose"  && echo $ac_n "	found " 1>&AC_FD_MSG
		break
	fi
	fi
	test -n "$verbose"  && echo "	tested $cf_dir" 1>&AC_FD_MSG
done
AC_MSG_RESULT($includedir)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CGETENT version: 3 updated: 2000/08/12 23:18:52
dnl ----------
dnl Check if the terminal-capability database functions are available.  If not,
dnl ncurses has a much-reduced version.
AC_DEFUN([CF_CGETENT],[
AC_MSG_CHECKING(for terminal-capability database functions)
AC_CACHE_VAL(cf_cv_cgetent,[
AC_TRY_LINK([
#include <stdlib.h>],[
	char temp[128];
	char *buf = temp;
	char *db_array = temp;
	cgetent(&buf, /* int *, */ &db_array, "vt100");
	cgetcap(buf, "tc", '=');
	cgetmatch(buf, "tc");
	],
	[cf_cv_cgetent=yes],
	[cf_cv_cgetent=no])
])
AC_MSG_RESULT($cf_cv_cgetent)
test "$cf_cv_cgetent" = yes && AC_DEFINE(HAVE_BSD_CGETENT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_CACHE version: 11 updated: 2008/03/23 14:45:59
dnl --------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname).  Normally we'll use AC_CANONICAL_HOST, but allow
dnl an extra parameter that we may override, e.g., for AC_CANONICAL_SYSTEM
dnl which is useful in cross-compiles.
dnl
dnl Note: we would use $ac_config_sub, but that is one of the places where
dnl autoconf 2.5x broke compatibility with autoconf 2.13
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f $srcdir/config.guess || test -f $ac_aux_dir/config.guess ; then
	ifelse([$1],,[AC_CANONICAL_HOST],[$1])
	system_name="$host_os"
else
	system_name="`(uname -s -r) 2>/dev/null`"
	if test -z "$system_name" ; then
		system_name="`(hostname) 2>/dev/null`"
	fi
fi
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name")
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT(Configuring for $cf_cv_system_name)

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_MSG_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_ERRNO version: 10 updated: 2008/08/22 16:33:22
dnl --------------
dnl Check for data that is usually declared in <stdio.h> or <errno.h>, e.g.,
dnl the 'errno' variable.  Define a DECL_xxx symbol if we must declare it
dnl ourselves.
dnl
dnl $1 = the name to check
dnl $2 = the assumed type
AC_DEFUN([CF_CHECK_ERRNO],
[
AC_CACHE_CHECK(if external $1 is declared, cf_cv_dcl_$1,[
    AC_TRY_COMPILE([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <errno.h> ],
    ifelse($2,,int,$2) x = (ifelse($2,,int,$2)) $1,
    [cf_cv_dcl_$1=yes],
    [cf_cv_dcl_$1=no])
])

if test "$cf_cv_dcl_$1" = no ; then
    CF_UPPER(cf_result,decl_$1)
    AC_DEFINE_UNQUOTED($cf_result)
fi

# It's possible (for near-UNIX clones) that the data doesn't exist
CF_CHECK_EXTERN_DATA($1,ifelse($2,,int,$2))
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_EXTERN_DATA version: 3 updated: 2001/12/30 18:03:23
dnl --------------------
dnl Check for existence of external data in the current set of libraries.  If
dnl we can modify it, it's real enough.
dnl $1 = the name to check
dnl $2 = its type
AC_DEFUN([CF_CHECK_EXTERN_DATA],
[
AC_CACHE_CHECK(if external $1 exists, cf_cv_have_$1,[
    AC_TRY_LINK([
#undef $1
extern $2 $1;
],
    [$1 = 2],
    [cf_cv_have_$1=yes],
    [cf_cv_have_$1=no])
])

if test "$cf_cv_have_$1" = yes ; then
    CF_UPPER(cf_result,have_$1)
    AC_DEFINE_UNQUOTED($cf_result)
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CHECK_GPM_WGETCH version: 1 updated: 2007/04/28 14:38:06
dnl -------------------
dnl Check if GPM is already linked with curses.  If so - and if the linkage
dnl is not "weak" - warn about this because it can create problems linking
dnl applications with ncurses.
AC_DEFUN([CF_CHECK_GPM_WGETCH],[
AC_CHECK_LIB(gpm,Gpm_Wgetch,[

AC_CACHE_CHECK(if GPM is weakly bound to curses library, cf_cv_check_gpm_wgetch,[
cf_cv_check_gpm_wgetch=unknown
if test "$cross_compiling" != yes ; then

cat >conftest.$ac_ext <<CF_EOF
#include <gpm.h>
int main()
{
	Gpm_Wgetch();
	${cf_cv_main_return:-return}(0);
}
CF_EOF

	cf_save_LIBS="$LIBS"
	# This only works if we can look at the symbol table.  If a shared
	# library is stripped for install, we cannot use that.  So we're forced
	# to rely on the static library, noting that some packagers may not
	# include it.
	LIBS="-static -lgpm -dynamic $LIBS"
	if AC_TRY_EVAL(ac_compile) ; then
		if AC_TRY_EVAL(ac_link) ; then
			cf_cv_check_gpm_wgetch=`nm conftest$ac_exeext | egrep '\<wgetch\>' | egrep '\<[[vVwW]]\>'`
			test -n "$cf_cv_check_gpm_wgetch" && cf_cv_check_gpm_wgetch=yes
			test -z "$cf_cv_check_gpm_wgetch" && cf_cv_check_gpm_wgetch=no
		fi
	fi
	rm -f conftest*
	LIBS="$cf_save_LIBS"
fi
])

if test "$cf_cv_check_gpm_wgetch" != yes ; then
	AC_MSG_WARN(GPM library is already linked with curses - read the FAQ)
fi
])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CPP_PARAM_INIT version: 4 updated: 2001/04/07 22:31:18
dnl -----------------
dnl Check if the C++ compiler accepts duplicate parameter initialization.  This
dnl is a late feature for the standard and is not in some recent compilers
dnl (1999/9/11).
AC_DEFUN([CF_CPP_PARAM_INIT],
[
if test -n "$CXX"; then
AC_CACHE_CHECK(if $CXX accepts parameter initialization,cf_cv_cpp_param_init,[
	AC_LANG_SAVE
	AC_LANG_CPLUSPLUS
	AC_TRY_RUN([
class TEST {
private:
	int value;
public:
	TEST(int x = 1);
	~TEST();
};

TEST::TEST(int x = 1)	// some compilers do not like second initializer
{
	value = x;
}
void main() { }
],
	[cf_cv_cpp_param_init=yes],
	[cf_cv_cpp_param_init=no],
	[cf_cv_cpp_param_init=unknown])
	AC_LANG_RESTORE
])
fi
test "$cf_cv_cpp_param_init" = yes && AC_DEFINE(CPP_HAS_PARAM_INIT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_CPP_STATIC_CAST version: 1 updated: 2005/07/23 16:52:43
dnl ------------------
dnl Check if the C++ compiler accepts static_cast in generics.  This appears to
dnl not be supported in g++ before 3.0
AC_DEFUN([CF_CPP_STATIC_CAST],
[
if test -n "$CXX"; then

AC_CACHE_CHECK(if $CXX accepts static_cast,cf_cv_cpp_static_cast,[
	AC_LANG_SAVE
	AC_LANG_CPLUSPLUS

	AC_TRY_COMPILE([
class NCursesPanel
{
public:
  NCursesPanel(int nlines,
	       int ncols,
	       int begin_y = 0,
	       int begin_x = 0)
  {
  }

  ~NCursesPanel();
};

template<class T> class NCursesUserPanel : public NCursesPanel
{
public:
  NCursesUserPanel (int nlines,
		    int ncols,
		    int begin_y = 0,
		    int begin_x = 0,
		    const T* p_UserData = static_cast<T*>(0))
    : NCursesPanel (nlines, ncols, begin_y, begin_x)
  {
  };
  NCursesUserPanel(const T* p_UserData = static_cast<T*>(0)) : NCursesPanel()
  {
  };

  virtual ~NCursesUserPanel() {};
};
],[
	const char* p_UserData = static_cast<char*>(0)],
	[cf_cv_cpp_static_cast=yes],
	[cf_cv_cpp_static_cast=no])

	AC_LANG_RESTORE
])

fi

test "$cf_cv_cpp_static_cast" = yes && AC_DEFINE(CPP_HAS_STATIC_CAST)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_C_INLINE version: 2 updated: 2007/08/11 14:09:50
dnl -----------
dnl Check if the C compiler supports "inline".
dnl $1 is the name of a shell variable to set if inline is supported
dnl $2 is the threshold for gcc 4.x's option controlling maximum inline size
AC_DEFUN([CF_C_INLINE],[
AC_C_INLINE
$1=
if test "$ac_cv_c_inline" != no ; then
  $1=inline
  if test "$INTEL_COMPILER" = yes
  then
    :
  elif test "$GCC" = yes
  then
    AC_CACHE_CHECK(if gcc supports options to tune inlining,cf_cv_gcc_inline,[
      cf_save_CFLAGS=$CFLAGS
      CFLAGS="$CFLAGS --param max-inline-insns-single=$2"
      AC_TRY_COMPILE([inline int foo(void) { return 1; }],
      [${cf_cv_main_return:-return} foo()],
      [cf_cv_gcc_inline=yes],
      [cf_cv_gcc_inline=no])
      CFLAGS=$cf_save_CFLAGS
    ])
    if test "$cf_cv_gcc_inline" = yes ; then
        CF_ADD_CFLAGS([--param max-inline-insns-single=$2])
    fi
  fi
fi
AC_SUBST($1)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DIRNAME version: 4 updated: 2002/12/21 19:25:52
dnl ----------
dnl "dirname" is not portable, so we fake it with a shell script.
AC_DEFUN([CF_DIRNAME],[$1=`echo $2 | sed -e 's%/[[^/]]*$%%'`])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DIRS_TO_MAKE version: 3 updated: 2002/02/23 20:38:31
dnl ---------------
AC_DEFUN([CF_DIRS_TO_MAKE],
[
DIRS_TO_MAKE="lib"
for cf_item in $cf_list_models
do
	CF_OBJ_SUBDIR($cf_item,cf_subdir)
	for cf_item2 in $DIRS_TO_MAKE
	do
		test $cf_item2 = $cf_subdir && break
	done
	test ".$cf_item2" != ".$cf_subdir" && DIRS_TO_MAKE="$DIRS_TO_MAKE $cf_subdir"
done
for cf_dir in $DIRS_TO_MAKE
do
	test ! -d $cf_dir && mkdir $cf_dir
done
AC_SUBST(DIRS_TO_MAKE)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_LEAKS version: 4 updated: 2006/12/16 15:10:42
dnl ----------------
dnl Combine no-leak checks with the libraries or tools that are used for the
dnl checks.
AC_DEFUN([CF_DISABLE_LEAKS],[

AC_REQUIRE([CF_WITH_DMALLOC])
AC_REQUIRE([CF_WITH_DBMALLOC])
AC_REQUIRE([CF_WITH_VALGRIND])

AC_MSG_CHECKING(if you want to perform memory-leak testing)
AC_ARG_ENABLE(leaks,
	[  --disable-leaks         test: free permanent memory, analyze leaks],
	[with_no_leaks=yes],
	: ${with_no_leaks:=no})
AC_MSG_RESULT($with_no_leaks)

if test "$with_no_leaks" = yes ; then
	AC_DEFINE(NO_LEAKS)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ENABLE_RPATH version: 1 updated: 2008/09/13 10:22:30
dnl ---------------
dnl Check if the rpath option should be used, setting cache variable
dnl cf_cv_ld_rpath if so.
AC_DEFUN([CF_ENABLE_RPATH],
[
AC_MSG_CHECKING(if rpath option should be used)
AC_ARG_ENABLE(rpath,
[  --enable-rpath          use rpath option when generating shared libraries],
[cf_cv_ld_rpath=$enableval],
[cf_cv_ld_rpath=no])
AC_MSG_RESULT($cf_cv_ld_rpath)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ERRNO version: 5 updated: 1997/11/30 12:44:39
dnl --------
dnl Check if 'errno' is declared in <errno.h>
AC_DEFUN([CF_ERRNO],
[
CF_CHECK_ERRNO(errno)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ETIP_DEFINES version: 3 updated: 2003/03/22 19:13:43
dnl ---------------
dnl Test for conflicting definitions of exception in gcc 2.8.0, etc., between
dnl math.h and builtin.h, only for ncurses
AC_DEFUN([CF_ETIP_DEFINES],
[
AC_MSG_CHECKING(for special defines needed for etip.h)
cf_save_CXXFLAGS="$CXXFLAGS"
cf_result="none"
for cf_math in "" MATH_H
do
for cf_excp in "" MATH_EXCEPTION
do
	CXXFLAGS="$cf_save_CXXFLAGS -I${srcdir}/c++ -I${srcdir}/menu -I${srcdir}/include"
	test -n "$cf_math" && CXXFLAGS="$CXXFLAGS -DETIP_NEEDS_${cf_math}"
	test -n "$cf_excp" && CXXFLAGS="$CXXFLAGS -DETIP_NEEDS_${cf_excp}"
AC_TRY_COMPILE([
#include <etip.h.in>
],[],[
	test -n "$cf_math" && AC_DEFINE_UNQUOTED(ETIP_NEEDS_${cf_math})
	test -n "$cf_excp" && AC_DEFINE_UNQUOTED(ETIP_NEEDS_${cf_excp})
	cf_result="$cf_math $cf_excp"
	break
],[])
done
done
AC_MSG_RESULT($cf_result)
CXXFLAGS="$cf_save_CXXFLAGS"
])
dnl ---------------------------------------------------------------------------
dnl CF_FIND_LINKAGE version: 12 updated: 2007/07/29 20:13:53
dnl ---------------
dnl Find a library (specifically the linkage used in the code fragment),
dnl searching for it if it is not already in the library path.
dnl See also CF_ADD_SEARCHPATH.
dnl
dnl Parameters (4-on are optional):
dnl     $1 = headers for library entrypoint
dnl     $2 = code fragment for library entrypoint
dnl     $3 = the library name without the "-l" option or ".so" suffix.
dnl     $4 = action to perform if successful (default: update CPPFLAGS, etc)
dnl     $5 = action to perform if not successful
dnl     $6 = module name, if not the same as the library name
dnl     $7 = extra libraries
dnl
dnl Sets these variables:
dnl     $cf_cv_find_linkage_$3 - yes/no according to whether linkage is found
dnl     $cf_cv_header_path_$3 - include-directory if needed
dnl     $cf_cv_library_path_$3 - library-directory if needed
dnl     $cf_cv_library_file_$3 - library-file if needed, e.g., -l$3
AC_DEFUN([CF_FIND_LINKAGE],[

# If the linkage is not already in the $CPPFLAGS/$LDFLAGS configuration, these
# will be set on completion of the AC_TRY_LINK below.
cf_cv_header_path_$3=
cf_cv_library_path_$3=

CF_MSG_LOG([Starting [FIND_LINKAGE]($3,$6)])

AC_TRY_LINK([$1],[$2],
    cf_cv_find_linkage_$3=yes,[
    cf_cv_find_linkage_$3=no

    CF_MSG_LOG([Searching for headers in [FIND_LINKAGE]($3,$6)])

    cf_save_CPPFLAGS="$CPPFLAGS"
    cf_test_CPPFLAGS="$CPPFLAGS"

    CF_HEADER_PATH(cf_search,ifelse([$6],,[$3],[$6]))
    for cf_cv_header_path_$3 in $cf_search
    do
      if test -d $cf_cv_header_path_$3 ; then
        CF_VERBOSE(... testing $cf_cv_header_path_$3)
        CPPFLAGS="$cf_save_CPPFLAGS -I$cf_cv_header_path_$3"
        AC_TRY_COMPILE([$1],[$2],[
            CF_VERBOSE(... found $3 headers in $cf_cv_header_path_$3)
            cf_cv_find_linkage_$3=maybe
            cf_test_CPPFLAGS="$CPPFLAGS"
            break],[
            CPPFLAGS="$cf_save_CPPFLAGS"
            ])
      fi
    done

    if test "$cf_cv_find_linkage_$3" = maybe ; then

      CF_MSG_LOG([Searching for $3 library in [FIND_LINKAGE]($3,$6)])

      cf_save_LIBS="$LIBS"
      cf_save_LDFLAGS="$LDFLAGS"

      ifelse([$6],,,[
        CPPFLAGS="$cf_test_CPPFLAGS"
        LIBS="-l$3 $7 $cf_save_LIBS"
        AC_TRY_LINK([$1],[$2],[
            CF_VERBOSE(... found $3 library in system)
            cf_cv_find_linkage_$3=yes])
            CPPFLAGS="$cf_save_CPPFLAGS"
            LIBS="$cf_save_LIBS"
            ])

      if test "$cf_cv_find_linkage_$3" != yes ; then
        CF_LIBRARY_PATH(cf_search,$3)
        for cf_cv_library_path_$3 in $cf_search
        do
          if test -d $cf_cv_library_path_$3 ; then
            CF_VERBOSE(... testing $cf_cv_library_path_$3)
            CPPFLAGS="$cf_test_CPPFLAGS"
            LIBS="-l$3 $7 $cf_save_LIBS"
            LDFLAGS="$cf_save_LDFLAGS -L$cf_cv_library_path_$3"
            AC_TRY_LINK([$1],[$2],[
                CF_VERBOSE(... found $3 library in $cf_cv_library_path_$3)
                cf_cv_find_linkage_$3=yes
                cf_cv_library_file_$3="-l$3"
                break],[
                CPPFLAGS="$cf_save_CPPFLAGS"
                LIBS="$cf_save_LIBS"
                LDFLAGS="$cf_save_LDFLAGS"
                ])
          fi
        done
        LIBS="$cf_save_LIBS"
        CPPFLAGS="$cf_save_CPPFLAGS"
        LDFLAGS="$cf_save_LDFLAGS"
      fi

    else
      cf_cv_find_linkage_$3=no
    fi
    ],$7)

if test "$cf_cv_find_linkage_$3" = yes ; then
ifelse([$4],,[
  CF_ADD_INCDIR($cf_cv_header_path_$3)
  CF_ADD_LIBDIR($cf_cv_library_path_$3)
  LIBS="-l$3 $LIBS"
],[$4])
else
ifelse([$5],,AC_MSG_WARN(Cannot find $3 library),[$5])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_DLSYM version: 1 updated: 2004/06/16 20:52:45
dnl -------------
dnl Test for dlsym() and related functions, as well as libdl.
dnl
dnl Sets
dnl	$cf_have_dlsym
dnl	$cf_have_libdl
AC_DEFUN([CF_FUNC_DLSYM],[
cf_have_dlsym=no
AC_CHECK_FUNC(dlsym,cf_have_dlsym=yes,[

cf_have_libdl=no
AC_CHECK_LIB(dl,dlsym,[
	cf_have_dlsym=yes
	cf_have_libdl=yes])])

if test "$cf_have_dlsym" = yes ; then
	test "$cf_have_libdl" = yes && LIBS="-ldl $LIBS"

	AC_MSG_CHECKING(whether able to link to dl*() functions)
	AC_TRY_LINK([#include <dlfcn.h>],[
		void *obj;
		if ((obj = dlopen("filename", 0)) != 0) {
			if (dlsym(obj, "symbolname") == 0) {
			dlclose(obj);
			}
		}],[
		AC_DEFINE(HAVE_LIBDL)],[
		AC_MSG_ERROR(Cannot link test program for libdl)])
	AC_MSG_RESULT(ok)
else
	AC_MSG_ERROR(Cannot find dlsym function)
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_MEMMOVE version: 7 updated: 2006/12/16 12:33:30
dnl ---------------
dnl Check for memmove, or a bcopy that can handle overlapping copy.  If neither
dnl is found, add our own version of memmove to the list of objects.
AC_DEFUN([CF_FUNC_MEMMOVE],
[
AC_CHECK_FUNC(memmove,,[
AC_CHECK_FUNC(bcopy,[
	AC_CACHE_CHECK(if bcopy does overlapping moves,cf_cv_good_bcopy,[
		AC_TRY_RUN([
int main() {
	static char data[] = "abcdefghijklmnopqrstuwwxyz";
	char temp[40];
	bcopy(data, temp, sizeof(data));
	bcopy(temp+10, temp, 15);
	bcopy(temp+5, temp+15, 10);
	${cf_cv_main_return:-return} (strcmp(temp, "klmnopqrstuwwxypqrstuwwxyz"));
}
		],
		[cf_cv_good_bcopy=yes],
		[cf_cv_good_bcopy=no],
		[cf_cv_good_bcopy=unknown])
		])
	],[cf_cv_good_bcopy=no])
	if test "$cf_cv_good_bcopy" = yes ; then
		AC_DEFINE(USE_OK_BCOPY)
	else
		AC_DEFINE(USE_MY_MEMMOVE)
	fi
])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_NANOSLEEP version: 3 updated: 2006/12/16 12:33:30
dnl -----------------
dnl Check for existence of workable nanosleep() function.  Some systems, e.g.,
dnl AIX 4.x, provide a non-working version.
AC_DEFUN([CF_FUNC_NANOSLEEP],[
AC_CACHE_CHECK(if nanosleep really works,cf_cv_func_nanosleep,[
AC_TRY_RUN([
#include <stdio.h>
#include <errno.h>
#include <time.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

int main() {
	struct timespec ts1, ts2;
	int code;
	ts1.tv_sec  = 0;
	ts1.tv_nsec = 750000000;
	ts2.tv_sec  = 0;
	ts2.tv_nsec = 0;
	errno = 0;
	code = nanosleep(&ts1, &ts2); /* on failure errno is ENOSYS. */
	${cf_cv_main_return:-return}(code != 0);
}
],
	[cf_cv_func_nanosleep=yes],
	[cf_cv_func_nanosleep=no],
	[cf_cv_func_nanosleep=unknown])])

test "$cf_cv_func_nanosleep" = "yes" && AC_DEFINE(HAVE_NANOSLEEP)
])
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_OPENPTY version: 2 updated: 2008/04/12 19:49:01
dnl ---------------
dnl Check for openpty() function, along with <pty.h> header.  It may need the
dnl "util" library as well.
AC_DEFUN([CF_FUNC_OPENPTY],
[
AC_CHECK_LIB(util,openpty,cf_cv_lib_util=yes,cf_cv_lib_util=no)
AC_CACHE_CHECK(for openpty header,cf_cv_func_openpty,[
    cf_save_LIBS="$LIBS"
    test $cf_cv_lib_util = yes && LIBS="-lutil $LIBS"
    for cf_header in pty.h libutil.h util.h
    do
    AC_TRY_LINK([
#include <$cf_header>
],[
    int x = openpty((int *)0, (int *)0, (char *)0,
                   (struct termios *)0, (struct winsize *)0);
],[
        cf_cv_func_openpty=$cf_header
        break
],[
        cf_cv_func_openpty=no
])
    done
    LIBS="$cf_save_LIBS"
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_POLL version: 4 updated: 2006/12/16 12:33:30
dnl ------------
dnl See if the poll function really works.  Some platforms have poll(), but
dnl it does not work for terminals or files.
AC_DEFUN([CF_FUNC_POLL],[
AC_CACHE_CHECK(if poll really works,cf_cv_working_poll,[
AC_TRY_RUN([
#include <stdio.h>
#ifdef HAVE_POLL_H
#include <poll.h>
#else
#include <sys/poll.h>
#endif
int main() {
	struct pollfd myfds;
	int ret;

	myfds.fd = 0;
	myfds.events = POLLIN;

	ret = poll(&myfds, 1, 100);
	${cf_cv_main_return:-return}(ret != 0);
}],
	[cf_cv_working_poll=yes],
	[cf_cv_working_poll=no],
	[cf_cv_working_poll=unknown])])
test "$cf_cv_working_poll" = "yes" && AC_DEFINE(HAVE_WORKING_POLL)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_TERMIOS version: 2 updated: 2000/07/22 23:37:24
dnl ---------------
dnl Some old/broken variations define tcgetattr() only as a macro in
dnl termio(s).h
AC_DEFUN([CF_FUNC_TERMIOS],[
AC_REQUIRE([CF_STRUCT_TERMIOS])
AC_CACHE_CHECK(for tcgetattr, cf_cv_have_tcgetattr,[
AC_TRY_LINK([
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#define TTY struct termios
#else
#ifdef HAVE_TERMIO_H
#include <termio.h>
#define TTY struct termio
#endif
#endif
],[
TTY foo;
tcgetattr(1, &foo);],
[cf_cv_have_tcgetattr=yes],
[cf_cv_have_tcgetattr=no])])
test "$cf_cv_have_tcgetattr" = yes && AC_DEFINE(HAVE_TCGETATTR)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FUNC_VSSCANF version: 3 updated: 2001/12/19 00:50:10
dnl ---------------
dnl Check for vsscanf() function, which is in c9x but generally not in earlier
dnl versions of C.  It is in the GNU C library, and can often be simulated by
dnl other functions.
AC_DEFUN([CF_FUNC_VSSCANF],
[
AC_CACHE_CHECK(for vsscanf function or workaround,cf_cv_func_vsscanf,[
AC_TRY_LINK([
#include <stdarg.h>
#include <stdio.h>],[
	va_list ap;
	vsscanf("from", "%d", ap)],[cf_cv_func_vsscanf=vsscanf],[
AC_TRY_LINK([
#include <stdarg.h>
#include <stdio.h>],[
    FILE strbuf;
    char *str = "from";

    strbuf._flag = _IOREAD;
    strbuf._ptr = strbuf._base = (unsigned char *) str;
    strbuf._cnt = strlen(str);
    strbuf._file = _NFILE;
    return (vfscanf(&strbuf, "%d", ap))],[cf_cv_func_vsscanf=vfscanf],[
AC_TRY_LINK([
#include <stdarg.h>
#include <stdio.h>],[
    FILE strbuf;
    char *str = "from";

    strbuf._flag = _IOREAD;
    strbuf._ptr = strbuf._base = (unsigned char *) str;
    strbuf._cnt = strlen(str);
    strbuf._file = _NFILE;
    return (_doscan(&strbuf, "%d", ap))],[cf_cv_func_vsscanf=_doscan],[
cf_cv_func_vsscanf=no])])])])

case $cf_cv_func_vsscanf in #(vi
vsscanf) AC_DEFINE(HAVE_VSSCANF);; #(vi
vfscanf) AC_DEFINE(HAVE_VFSCANF);; #(vi
_doscan) AC_DEFINE(HAVE__DOSCAN);;
esac

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_ATTRIBUTES version: 11 updated: 2007/07/29 09:55:12
dnl -----------------
dnl Test for availability of useful gcc __attribute__ directives to quiet
dnl compiler warnings.  Though useful, not all are supported -- and contrary
dnl to documentation, unrecognized directives cause older compilers to barf.
AC_DEFUN([CF_GCC_ATTRIBUTES],
[
if test "$GCC" = yes
then
cat > conftest.i <<EOF
#ifndef GCC_PRINTF
#define GCC_PRINTF 0
#endif
#ifndef GCC_SCANF
#define GCC_SCANF 0
#endif
#ifndef GCC_NORETURN
#define GCC_NORETURN /* nothing */
#endif
#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif
EOF
if test "$GCC" = yes
then
	AC_CHECKING([for $CC __attribute__ directives])
cat > conftest.$ac_ext <<EOF
#line __oline__ "${as_me-configure}"
#include "confdefs.h"
#include "conftest.h"
#include "conftest.i"
#if	GCC_PRINTF
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
#define GCC_PRINTFLIKE(fmt,var) /*nothing*/
#endif
#if	GCC_SCANF
#define GCC_SCANFLIKE(fmt,var)  __attribute__((format(scanf,fmt,var)))
#else
#define GCC_SCANFLIKE(fmt,var)  /*nothing*/
#endif
extern void wow(char *,...) GCC_SCANFLIKE(1,2);
extern void oops(char *,...) GCC_PRINTFLIKE(1,2) GCC_NORETURN;
extern void foo(void) GCC_NORETURN;
int main(int argc GCC_UNUSED, char *argv[[]] GCC_UNUSED) { return 0; }
EOF
	for cf_attribute in scanf printf unused noreturn
	do
		CF_UPPER(cf_ATTRIBUTE,$cf_attribute)
		cf_directive="__attribute__(($cf_attribute))"
		echo "checking for $CC $cf_directive" 1>&AC_FD_CC
		case $cf_attribute in
		scanf|printf)
		cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
			;;
		*)
		cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE $cf_directive
EOF
			;;
		esac
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... $cf_attribute)
			cat conftest.h >>confdefs.h
		fi
	done
else
	fgrep define conftest.i >>confdefs.h
fi
rm -rf conftest*
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_VERSION version: 4 updated: 2005/08/27 09:53:42
dnl --------------
dnl Find version of gcc
AC_DEFUN([CF_GCC_VERSION],[
AC_REQUIRE([AC_PROG_CC])
GCC_VERSION=none
if test "$GCC" = yes ; then
	AC_MSG_CHECKING(version of $CC)
	GCC_VERSION="`${CC} --version| sed -e '2,$d' -e 's/^.*(GCC) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GCC_VERSION" && GCC_VERSION=unknown
	AC_MSG_RESULT($GCC_VERSION)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_WARNINGS version: 23 updated: 2008/07/26 17:54:02
dnl ---------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally).  This
dnl		is enabled for ncurses using "--enable-const".
dnl	-pedantic
dnl
dnl Parameter:
dnl	$1 is an optional list of gcc warning flags that a particular
dnl		application might want to use, e.g., "no-unused" for
dnl		-Wno-unused
dnl Special:
dnl	If $with_ext_const is "yes", add a check for -Wwrite-strings
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[
AC_REQUIRE([CF_GCC_VERSION])
CF_INTEL_COMPILER(GCC,INTEL_COMPILER,CFLAGS)

cat > conftest.$ac_ext <<EOF
#line __oline__ "${as_me-configure}"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF

if test "$INTEL_COMPILER" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #1682: implicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1683: explicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1684: conversion from pointer to same-sized integral type (potential portability problem)
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #269: invalid format string conversion

	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="-Wall"
	for cf_opt in \
		wd1419 \
		wd1682 \
		wd1683 \
		wd1684 \
		wd193 \
		wd279 \
		wd593 \
		wd810 \
		wd869 \
		wd981
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"

elif test "$GCC" = yes
then
	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="-W -Wall"
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	for cf_opt in \
		Wbad-function-cast \
		Wcast-align \
		Wcast-qual \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes \
		Wundef $cf_warn_CONST $1
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			case $cf_opt in #(vi
			Wcast-qual) #(vi
				CPPFLAGS="$CPPFLAGS -DXTSTRINGDEFINES"
				;;
			Winline) #(vi
				case $GCC_VERSION in
				[[34]].*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			esac
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"
fi
rm -f conftest*

AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GNAT_TRY_LINK version: 1 updated: 2004/08/21 19:02:08
dnl ----------------
dnl Verify that a test program compiles/links with GNAT.
dnl $cf_ada_make is set to the program that compiles/links
dnl $ADAFLAGS may be set to the GNAT flags.
dnl
dnl $1 is the text of the spec
dnl $2 is the text of the body
dnl $3 is the shell command to execute if successful
dnl $4 is the shell command to execute if not successful
AC_DEFUN([CF_GNAT_TRY_LINK],
[
rm -f conftest*
cat >>conftest.ads <<CF_EOF
$1
CF_EOF
cat >>conftest.adb <<CF_EOF
$2
CF_EOF
if ( $cf_ada_make $ADAFLAGS conftest 1>&AC_FD_CC 2>&1 ) ; then
ifelse($3,,      :,[      $3])
ifelse($4,,,[else
   $4])
fi
rm -f conftest*
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GNAT_TRY_RUN version: 3 updated: 2004/08/21 19:02:08
dnl ---------------
dnl Verify that a test program compiles and runs with GNAT
dnl $cf_ada_make is set to the program that compiles/links
dnl $ADAFLAGS may be set to the GNAT flags.
dnl
dnl $1 is the text of the spec
dnl $2 is the text of the body
dnl $3 is the shell command to execute if successful
dnl $4 is the shell command to execute if not successful
AC_DEFUN([CF_GNAT_TRY_RUN],
[
rm -f conftest*
cat >>conftest.ads <<CF_EOF
$1
CF_EOF
cat >>conftest.adb <<CF_EOF
$2
CF_EOF
if ( $cf_ada_make $ADAFLAGS conftest 1>&AC_FD_CC 2>&1 ) ; then
   if ( ./conftest 1>&AC_FD_CC 2>&1 ) ; then
ifelse($3,,      :,[      $3])
ifelse($4,,,[   else
      $4])
   fi
ifelse($4,,,[else
   $4])
fi
rm -f conftest*
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GNAT_VERSION version: 12 updated: 2006/10/14 15:23:15
dnl ---------------
dnl Verify version of GNAT.
AC_DEFUN([CF_GNAT_VERSION],
[
AC_MSG_CHECKING(for gnat version)
cf_gnat_version=`${cf_ada_make-gnatmake} -v 2>&1 | grep '[[0-9]].[[0-9]][[0-9]]*' |\
  sed -e '2,$d' -e 's/[[^0-9 \.]]//g' -e 's/^[[ ]]*//' -e 's/ .*//'`
AC_MSG_RESULT($cf_gnat_version)

case $cf_gnat_version in
  3.1[[1-9]]*|3.[[2-9]]*|[[4-9]].*)
    cf_cv_prog_gnat_correct=yes
    ;;
  *) echo Unsupported GNAT version $cf_gnat_version. Required is 3.11 or better. Disabling Ada95 binding.
     cf_cv_prog_gnat_correct=no
     ;;
esac
case $cf_gnat_version in
  3.[[1-9]]*|[[4-9]].*)
      cf_compile_generics=generics
      cf_generic_objects="\${GENOBJS}"
      ;;
  *)  cf_compile_generics=
      cf_generic_objects=
      ;;
esac
])
dnl ---------------------------------------------------------------------------
dnl CF_GNU_SOURCE version: 6 updated: 2005/07/09 13:23:07
dnl -------------
dnl Check if we must define _GNU_SOURCE to get a reasonable value for
dnl _XOPEN_SOURCE, upon which many POSIX definitions depend.  This is a defect
dnl (or misfeature) of glibc2, which breaks portability of many applications,
dnl since it is interwoven with GNU extensions.
dnl
dnl Well, yes we could work around it...
AC_DEFUN([CF_GNU_SOURCE],
[
AC_CACHE_CHECK(if we must define _GNU_SOURCE,cf_cv_gnu_source,[
AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_gnu_source=no],
	[cf_save="$CPPFLAGS"
	 CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_gnu_source=no],
	[cf_cv_gnu_source=yes])
	CPPFLAGS="$cf_save"
	])
])
test "$cf_cv_gnu_source" = yes && CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GPP_LIBRARY version: 8 updated: 2003/02/02 01:41:46
dnl --------------
dnl If we're trying to use g++, test if libg++ is installed (a rather common
dnl problem :-).  If we have the compiler but no library, we'll be able to
dnl configure, but won't be able to build the c++ demo program.
AC_DEFUN([CF_GPP_LIBRARY],
[
cf_cxx_library=unknown
case $cf_cv_system_name in #(vi
os2*) #(vi
	cf_gpp_libname=gpp
	;;
*)
	cf_gpp_libname=g++
	;;
esac
if test "$GXX" = yes; then
	AC_MSG_CHECKING([for lib$cf_gpp_libname])
	cf_save="$LIBS"
	LIBS="$LIBS -l$cf_gpp_libname"
	AC_TRY_LINK([
#include <$cf_gpp_libname/builtin.h>
	],
	[two_arg_error_handler_t foo2 = lib_error_handler],
	[cf_cxx_library=yes
	 CXXLIBS="$CXXLIBS -l$cf_gpp_libname"
	 if test "$cf_gpp_libname" = cpp ; then
	    AC_DEFINE(HAVE_GPP_BUILTIN_H)
	 else
	    AC_DEFINE(HAVE_GXX_BUILTIN_H)
	 fi],
	[AC_TRY_LINK([
#include <builtin.h>
	],
	[two_arg_error_handler_t foo2 = lib_error_handler],
	[cf_cxx_library=yes
	 CXXLIBS="$CXXLIBS -l$cf_gpp_libname"
	 AC_DEFINE(HAVE_BUILTIN_H)],
	[cf_cxx_library=no])])
	LIBS="$cf_save"
	AC_MSG_RESULT($cf_cxx_library)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GXX_VERSION version: 5 updated: 2005/08/27 09:53:42
dnl --------------
dnl Check for version of g++
AC_DEFUN([CF_GXX_VERSION],[
AC_REQUIRE([AC_PROG_CPP])
GXX_VERSION=none
if test "$GXX" = yes; then
	AC_MSG_CHECKING(version of g++)
	GXX_VERSION="`${CXX-g++} --version| sed -e '2,$d' -e 's/^.*(GCC) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GXX_VERSION" && GXX_VERSION=unknown
	AC_MSG_RESULT($GXX_VERSION)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GXX_WARNINGS version: 5 updated: 2005/08/13 14:54:38
dnl ---------------
dnl Check if the compiler supports useful warning options.
dnl
dnl Most of gcc's options apply to g++, except:
dnl	-Wbad-function-cast
dnl	-Wmissing-declarations
dnl	-Wnested-externs
dnl
dnl Omit a few (for now):
dnl	-Winline
dnl
dnl Parameter:
dnl	$1 is an optional list of g++ warning flags that a particular
dnl		application might want to use, e.g., "no-unused" for
dnl		-Wno-unused
dnl Special:
dnl	If $with_ext_const is "yes", add a check for -Wwrite-strings
dnl
AC_DEFUN([CF_GXX_WARNINGS],
[

CF_INTEL_COMPILER(GXX,INTEL_CPLUSPLUS,CXXFLAGS)

AC_REQUIRE([CF_GXX_VERSION])

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF

if test "$INTEL_CPLUSPLUS" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #1682: implicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1683: explicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1684: conversion from pointer to same-sized integral type (potential portability problem)
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #269: invalid format string conversion

	AC_CHECKING([for $CC warning options])
	cf_save_CXXFLAGS="$CXXFLAGS"
	EXTRA_CXXFLAGS="-Wall"
	for cf_opt in \
		wd1419 \
		wd1682 \
		wd1683 \
		wd1684 \
		wd193 \
		wd279 \
		wd593 \
		wd810 \
		wd869 \
		wd981
	do
		CXXFLAGS="$cf_save_CXXFLAGS $EXTRA_CXXFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CXXFLAGS="$EXTRA_CXXFLAGS -$cf_opt"
		fi
	done
	CXXFLAGS="$cf_save_CXXFLAGS"

elif test "$GXX" = yes
then
	AC_CHECKING([for $CXX warning options])
	cf_save_CXXFLAGS="$CXXFLAGS"
	EXTRA_CXXFLAGS="-W -Wall"
	cf_gxx_extra_warnings=""
	test "$with_ext_const" = yes && cf_gxx_extra_warnings="Wwrite-strings"
	case "$GCC_VERSION" in
	[[1-2]].*)
		;;
	*)
		cf_gxx_extra_warnings="$cf_gxx_extra_warnings Weffc++"
		;;
	esac
	for cf_opt in \
		Wabi \
		fabi-version=0 \
		Woverloaded-virtual \
		Wsign-promo \
		Wsynth \
		Wold-style-cast \
		Wcast-align \
		Wcast-qual \
		Wmissing-prototypes \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes \
		Wundef $cf_gxx_extra_warnings $1
	do
		CXXFLAGS="$cf_save_CXXFLAGS $EXTRA_CXXFLAGS -Werror -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CXXFLAGS="$EXTRA_CXXFLAGS -$cf_opt"
		else
			test -n "$verbose" && AC_MSG_RESULT(... no -$cf_opt)
		fi
	done
	CXXFLAGS="$cf_save_CXXFLAGS"
fi

rm -f conftest*
AC_LANG_RESTORE
AC_SUBST(EXTRA_CXXFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HASHED_DB version: 3 updated: 2007/11/24 17:43:37
dnl ------------
dnl Look for an instance of the Berkeley hashed database.
dnl
dnl $1 = optional parameter, to specify install-prefix for the database.
AC_DEFUN([CF_HASHED_DB],
[
ifelse([$1],,,[
case $1 in #(vi
yes|*able*) #(vi
    ;;
*)
    if test -d "$1" ; then
        CF_ADD_INCDIR($1/include)
        CF_ADD_LIBDIR($1/lib)
    fi
esac
])
AC_CHECK_HEADER(db.h,[
CF_HASHED_DB_VERSION
if test "$cf_cv_hashed_db_version" = unknown ; then
	AC_MSG_ERROR(Cannot determine version of db)
else
	CF_HASHED_DB_LIBS
	if test "$cf_cv_hashed_db_libs" = unknown ; then
		AC_MSG_ERROR(Cannot determine library for db)
	elif test "$cf_cv_hashed_db_libs" != default ; then
		LIBS="-l$cf_cv_hashed_db_libs $LIBS"
	fi
fi
],[
	AC_MSG_ERROR(Cannot find db.h)
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HASHED_DB_LIBS version: 8 updated: 2008/08/04 06:18:06
dnl -----------------
dnl Given that we have the header and version for hashed database, find the
dnl library information.
AC_DEFUN([CF_HASHED_DB_LIBS],
[
AC_CACHE_CHECK(for db libraries, cf_cv_hashed_db_libs,[
cf_cv_hashed_db_libs=unknown
for cf_db_libs in "" db$cf_cv_hashed_db_version db-$cf_cv_hashed_db_version db ''
do
	cf_save_libs="$LIBS"
	if test -n "$cf_db_libs"; then
		LIBS="-l$cf_db_libs $LIBS"
	fi
	CF_MSG_LOG(checking for library "$cf_db_libs")
	AC_TRY_LINK([
$ac_includes_default
#include <db.h>
],[
	char *path = "/tmp/foo";
#ifdef DB_VERSION_MAJOR
#if DB_VERSION_MAJOR >= 4
	DB *result = 0;
	db_create(&result, NULL, 0);
	result->open(result,
		NULL,
		path,
		path,
		DB_HASH,
		DB_CREATE,
		0644);
#elif DB_VERSION_MAJOR >= 3
	DB *result = 0;
	db_create(&result, NULL, 0);
	result->open(result,
		path,
		path,
		DB_HASH,
		DB_CREATE,
		0644);
#elif DB_VERSION_MAJOR >= 2
	DB *result = 0;
	db_open(path,
		DB_HASH,
		DB_CREATE,
		0644,
		(DB_ENV *) 0,
		(DB_INFO *) 0,
		&result);
#endif /* DB_VERSION_MAJOR */
#else
	DB *result = dbopen(path,
		     2,
		     0644,
		     DB_HASH,
		     0);
#endif
	${cf_cv_main_return:-return}(result != 0)
],[
	if test -n "$cf_db_libs" ; then
		cf_cv_hashed_db_libs=$cf_db_libs
	else
		cf_cv_hashed_db_libs=default
	fi
	LIBS="$cf_save_libs"
	break
])
	LIBS="$cf_save_libs"
done
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HASHED_DB_VERSION version: 3 updated: 2007/12/01 15:01:37
dnl --------------------
dnl Given that we have the header file for hashed database, find the version
dnl information.
AC_DEFUN([CF_HASHED_DB_VERSION],
[
AC_CACHE_CHECK(for version of db, cf_cv_hashed_db_version,[
cf_cv_hashed_db_version=unknown

for cf_db_version in 1 2 3 4 5
do
	CF_MSG_LOG(checking for db version $cf_db_version)
	AC_TRY_COMPILE([
$ac_includes_default
#include <db.h>

#ifdef DB_VERSION_MAJOR
	/* db2 (DB_VERSION_MAJOR=2) has also DB_VERSION_MINOR, tested with 7 */
#if $cf_db_version == DB_VERSION_MAJOR
	/* ok */
#else
	make an error
#endif
#else
#if $cf_db_version == 1
	/* ok: assuming this is DB 1.8.5 */
#else
	make an error
#endif
#endif
],[DBT *foo = 0],[
	cf_cv_hashed_db_version=$cf_db_version
	break
	])
done
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HEADER_PATH version: 8 updated: 2002/11/10 14:46:59
dnl --------------
dnl Construct a search-list for a nonstandard header-file
AC_DEFUN([CF_HEADER_PATH],
[CF_SUBDIR_PATH($1,$2,include)
test "$includedir" != NONE && \
test "$includedir" != "/usr/include" && \
test -d "$includedir" && {
	test -d $includedir &&    $1="[$]$1 $includedir"
	test -d $includedir/$2 && $1="[$]$1 $includedir/$2"
}

test "$oldincludedir" != NONE && \
test "$oldincludedir" != "/usr/include" && \
test -d "$oldincludedir" && {
	test -d $oldincludedir    && $1="[$]$1 $oldincludedir"
	test -d $oldincludedir/$2 && $1="[$]$1 $oldincludedir/$2"
}

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_HELP_MESSAGE version: 3 updated: 1998/01/14 10:56:23
dnl ---------------
dnl Insert text into the help-message, for readability, from AC_ARG_WITH.
AC_DEFUN([CF_HELP_MESSAGE],
[AC_DIVERT_HELP([$1])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INCLUDE_DIRS version: 5 updated: 2006/10/14 15:23:15
dnl ---------------
dnl Construct the list of include-options according to whether we're building
dnl in the source directory or using '--srcdir=DIR' option.  If we're building
dnl with gcc, don't append the includedir if it happens to be /usr/include,
dnl since that usually breaks gcc's shadow-includes.
AC_DEFUN([CF_INCLUDE_DIRS],
[
CPPFLAGS="-I. -I../include $CPPFLAGS"
if test "$srcdir" != "."; then
	CPPFLAGS="-I\${srcdir}/../include $CPPFLAGS"
fi
if test "$GCC" != yes; then
	CPPFLAGS="$CPPFLAGS -I\${includedir}"
elif test "$includedir" != "/usr/include"; then
	if test "$includedir" = '${prefix}/include' ; then
		if test $prefix != /usr ; then
			CPPFLAGS="$CPPFLAGS -I\${includedir}"
		fi
	else
		CPPFLAGS="$CPPFLAGS -I\${includedir}"
	fi
fi
AC_SUBST(CPPFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 3 updated: 2005/08/06 18:37:29
dnl -----------------
dnl Check if the given compiler is really the Intel compiler for Linux.  It
dnl tries to imitate gcc, but does not return an error when it finds a mismatch
dnl between prototypes, e.g., as exercised by CF_MISSING_CHECK.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = INTEL_COMPILER (default) or INTEL_CPLUSPLUS
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_INTEL_COMPILER],[
ifelse($2,,INTEL_COMPILER,[$2])=no

if test "$ifelse($1,,[$1],GCC)" = yes ; then
	case $host_os in
	linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel ifelse($1,GXX,C++,C) compiler)
		cf_save_CFLAGS="$ifelse($3,,CFLAGS,[$3])"
		ifelse($3,,CFLAGS,[$3])="$ifelse($3,,CFLAGS,[$3]) -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
make an error
#endif
],[ifelse($2,,INTEL_COMPILER,[$2])=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147 -no-gcc"
],[])
		ifelse($3,,CFLAGS,[$3])="$cf_save_CFLAGS"
		AC_MSG_RESULT($ifelse($2,,INTEL_COMPILER,[$2]))
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ISASCII version: 3 updated: 2000/08/12 23:18:52
dnl ----------
dnl Check if we have either a function or macro for 'isascii()'.
AC_DEFUN([CF_ISASCII],
[
AC_MSG_CHECKING(for isascii)
AC_CACHE_VAL(cf_cv_have_isascii,[
	AC_TRY_LINK([#include <ctype.h>],[int x = isascii(' ')],
	[cf_cv_have_isascii=yes],
	[cf_cv_have_isascii=no])
])dnl
AC_MSG_RESULT($cf_cv_have_isascii)
test "$cf_cv_have_isascii" = yes && AC_DEFINE(HAVE_ISASCII)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LARGEFILE version: 7 updated: 2007/06/02 11:58:50
dnl ------------
dnl Add checks for large file support.
AC_DEFUN([CF_LARGEFILE],[
ifdef([AC_FUNC_FSEEKO],[
    AC_SYS_LARGEFILE
    if test "$enable_largefile" != no ; then
	AC_FUNC_FSEEKO

	# Normally we would collect these definitions in the config.h,
	# but (like _XOPEN_SOURCE), some environments rely on having these
	# defined before any of the system headers are included.  Another
	# case comes up with C++, e.g., on AIX the compiler compiles the
	# header files by themselves before looking at the body files it is
	# told to compile.  For ncurses, those header files do not include
	# the config.h
	test "$ac_cv_sys_large_files"      != no && CPPFLAGS="$CPPFLAGS -D_LARGE_FILES "
	test "$ac_cv_sys_largefile_source" != no && CPPFLAGS="$CPPFLAGS -D_LARGEFILE_SOURCE "
	test "$ac_cv_sys_file_offset_bits" != no && CPPFLAGS="$CPPFLAGS -D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits "

	AC_CACHE_CHECK(whether to use struct dirent64, cf_cv_struct_dirent64,[
		AC_TRY_COMPILE([
#include <sys/types.h>
#include <dirent.h>
		],[
		/* if transitional largefile support is setup, this is true */
		extern struct dirent64 * readdir(DIR *);
		struct dirent64 *x = readdir((DIR *)0);
		struct dirent *y = readdir((DIR *)0);
		int z = x - y;
		],
		[cf_cv_struct_dirent64=yes],
		[cf_cv_struct_dirent64=no])
	])
	test "$cf_cv_struct_dirent64" = yes && AC_DEFINE(HAVE_STRUCT_DIRENT64)
    fi
])
])
dnl ---------------------------------------------------------------------------
dnl CF_LDFLAGS_STATIC version: 4 updated: 2008/10/18 17:58:20
dnl -----------------
dnl Check for compiler/linker flags used to temporarily force usage of static
dnl libraries.  This depends on the compiler and platform.  Use this to help
dnl ensure that the linker picks up a given library based on its position in
dnl the list of linker options and libraries.
AC_DEFUN([CF_LDFLAGS_STATIC],[

if test "$GCC" = yes ; then
	case $cf_cv_system_name in #(
	OS/2*|os2*|aix[[45]]*) 	#( vi
		LDFLAGS_STATIC=
		LDFLAGS_SHARED=
		;;
    *) 	#( normally, except when broken
        LDFLAGS_STATIC=-static
        LDFLAGS_SHARED=-dynamic
        ;;
    esac
else
	case $cf_cv_system_name in #(
	aix[[45]]*) 	#( from ld manpage
		LDFLAGS_STATIC=-bstatic
		LDFLAGS_SHARED=-bdynamic
		;;
	hpux*)		#( from ld manpage for hpux10.20, hpux11.11
		# We could also use just "archive" and "shared".
		LDFLAGS_STATIC=-Wl,-a,archive_shared
		LDFLAGS_SHARED=-Wl,-a,shared_archive
		;;
	irix*)		#( from ld manpage IRIX64
		LDFLAGS_STATIC=-Bstatic
		LDFLAGS_SHARED=-Bdynamic
		;;
	osf[[45]]*)	#( from ld manpage osf4.0d, osf5.1
		# alternative "-oldstyle_liblookup" (not in cc manpage)
		LDFLAGS_STATIC=-noso 
		LDFLAGS_SHARED=-so_archive
		;;
	solaris2*)
		LDFLAGS_STATIC=-Bstatic
		LDFLAGS_SHARED=-Bdynamic
		;;
	esac
fi

AC_SUBST(LDFLAGS_STATIC)
AC_SUBST(LDFLAGS_SHARED)
])
dnl ---------------------------------------------------------------------------
dnl CF_LIBRARY_PATH version: 7 updated: 2002/11/10 14:46:59
dnl ---------------
dnl Construct a search-list for a nonstandard library-file
AC_DEFUN([CF_LIBRARY_PATH],
[CF_SUBDIR_PATH($1,$2,lib)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LIB_PREFIX version: 8 updated: 2008/09/13 11:34:16
dnl -------------
dnl Compute the library-prefix for the given host system
dnl $1 = variable to set
AC_DEFUN([CF_LIB_PREFIX],
[
	case $cf_cv_system_name in #(vi
	OS/2*|os2*) #(vi
        LIB_PREFIX=''
        ;;
	*)	LIB_PREFIX='lib'
        ;;
	esac
ifelse($1,,,[$1=$LIB_PREFIX])
	AC_SUBST(LIB_PREFIX)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LIB_RULES version: 53 updated: 2008/09/20 19:51:59
dnl ------------
dnl Append definitions and rules for the given models to the subdirectory
dnl Makefiles, and the recursion rule for the top-level Makefile.  If the
dnl subdirectory is a library-source directory, modify the LIBS_TO_MAKE list in
dnl the corresponding makefile to list the models that we'll generate.
dnl
dnl For shared libraries, make a list of symbolic links to construct when
dnl generating each library.  The convention used for Linux is the simplest
dnl one:
dnl	lib<name>.so	->
dnl	lib<name>.so.<major>	->
dnl	lib<name>.so.<maj>.<minor>
AC_DEFUN([CF_LIB_RULES],
[
CF_LIB_PREFIX(cf_prefix)
AC_REQUIRE([CF_SUBST_NCURSES_VERSION])

if test $cf_cv_shlib_version = cygdll ; then
	TINFO_NAME=$TINFO_ARG_SUFFIX
	TINFO_SUFFIX=.dll
fi

for cf_dir in $SRC_SUBDIRS
do
	if test ! -d $srcdir/$cf_dir ; then
		continue
	elif test -f $srcdir/$cf_dir/modules; then

		SHARED_LIB=
		LIBS_TO_MAKE=
		for cf_item in $cf_LIST_MODELS
		do
			CF_LIB_SUFFIX($cf_item,cf_suffix,cf_depsuf)
			if test $cf_item = shared ; then
			if test "$cf_cv_do_symlinks" = yes ; then
				case "$cf_cv_shlib_version" in #(vi
				rel) #(vi
					case "$cf_cv_system_name" in #(vi
					darwin*)
					case .${LIB_SUFFIX} in
					.w*)
						cf_suffix=`echo $cf_suffix | sed 's/^w//'`
						cf_suffix=w'.${REL_VERSION}'"$cf_suffix"
						;;
					*)
						cf_suffix='.${REL_VERSION}'"$cf_suffix"
						;;
					esac
					;; #(vi
					*) cf_suffix="$cf_suffix"'.${REL_VERSION}' ;;
					esac
					;;
				abi)
					case "$cf_cv_system_name" in #(vi
					darwin*)
					case .${LIB_SUFFIX} in
					.w*)
						cf_suffix=`echo $cf_suffix | sed 's/^w//'`
						cf_suffix=w'.${ABI_VERSION}'"$cf_suffix"
						;;
					*)
						cf_suffix='.${ABI_VERSION}'"$cf_suffix"
						;;
					esac
					;; #(vi
					*) cf_suffix="$cf_suffix"'.${ABI_VERSION}' ;;
					esac
					;;
				esac
			fi
			# cygwin needs import library, and has unique naming convention
			# use autodetected ${cf_prefix} for import lib and static lib, but
			# use 'cyg' prefix for shared lib.
			if test $cf_cv_shlib_version = cygdll ; then
				cf_cygsuf=`echo "$cf_suffix" | sed -e 's/\.dll/\${ABI_VERSION}.dll/'`
				LIBS_TO_MAKE="$LIBS_TO_MAKE ../lib/cyg${cf_dir}${cf_cygsuf}"
				continue
			fi
			fi
			LIBS_TO_MAKE="$LIBS_TO_MAKE ../lib/${cf_prefix}${cf_dir}${cf_suffix}"
		done

		if test $cf_dir = ncurses ; then
			cf_subsets="$LIB_SUBSETS"
			cf_r_parts="$cf_subsets"
			cf_liblist="$LIBS_TO_MAKE"

			while test -n "$cf_r_parts"
			do
				cf_l_parts=`echo "$cf_r_parts" |sed -e 's/ .*$//'`
				cf_r_parts=`echo "$cf_r_parts" |sed -e 's/^[[^ ]]* //'`
				if test "$cf_l_parts" != "$cf_r_parts" ; then
					cf_item=
					case $cf_l_parts in #(vi
					*termlib*) #(vi
						cf_item=`echo $cf_liblist |sed -e s%${LIB_NAME}${LIB_SUFFIX}%${TINFO_LIB_SUFFIX}%g`
						;;
					*ticlib*)
						cf_item=`echo $cf_liblist |sed -e s%${LIB_NAME}${LIB_SUFFIX}%${TICS_LIB_SUFFIX}%g`
						;;
					*)
						break
						;;
					esac
					if test -n "$cf_item"; then
						LIBS_TO_MAKE="$cf_item $LIBS_TO_MAKE"
					fi
				else
					break
				fi
			done
		else
			cf_subsets=`echo "$LIB_SUBSETS" | sed -e 's/^termlib.* //'`
		fi

		sed -e "s%@LIBS_TO_MAKE@%$LIBS_TO_MAKE%" \
		    -e "s%@SHARED_LIB@%$SHARED_LIB%" \
			$cf_dir/Makefile >$cf_dir/Makefile.out
		mv $cf_dir/Makefile.out $cf_dir/Makefile

		$AWK -f $srcdir/mk-0th.awk \
			libname="${cf_dir}${LIB_SUFFIX}" subsets="$LIB_SUBSETS" \
			$srcdir/$cf_dir/modules >>$cf_dir/Makefile

		for cf_subset in $cf_subsets
		do
			cf_subdirs=
			for cf_item in $cf_LIST_MODELS
			do
			echo "Appending rules for ${cf_item} model (${cf_dir}: ${cf_subset})"
			CF_UPPER(cf_ITEM,$cf_item)
			CF_LIB_SUFFIX($cf_item,cf_suffix,cf_depsuf)
			CF_OBJ_SUBDIR($cf_item,cf_subdir)

			# Test for case where we build libtinfo with a different name.
			cf_libname=$cf_dir
			if test $cf_dir = ncurses ; then
				case $cf_subset in
				*base*)
					cf_libname=${cf_libname}$LIB_SUFFIX
					;;
				*termlib*)
					cf_libname=$TINFO_LIB_SUFFIX
					;;
				ticlib*)
					cf_libname=$TICS_LIB_SUFFIX
					;;
				esac
			else
				cf_libname=${cf_libname}$LIB_SUFFIX
			fi
			if test -n "${DFT_ARG_SUFFIX}" ; then
				# undo $LIB_SUFFIX add-on in CF_LIB_SUFFIX
				cf_suffix=`echo $cf_suffix |sed -e "s%^${LIB_SUFFIX}%%"`
			fi

			# These dependencies really are for development, not
			# builds, but they are useful in porting, too.
			cf_depend="../include/ncurses_cfg.h"
			if test "$srcdir" = "."; then
				cf_reldir="."
			else
				cf_reldir="\${srcdir}"
			fi

			if test -f $srcdir/$cf_dir/$cf_dir.priv.h; then
				cf_depend="$cf_depend $cf_reldir/$cf_dir.priv.h"
			elif test -f $srcdir/$cf_dir/curses.priv.h; then
				cf_depend="$cf_depend $cf_reldir/curses.priv.h"
			fi

 			cf_dir_suffix=
 			old_cf_suffix="$cf_suffix"
 			if test "$cf_cv_shlib_version_infix" = yes ; then
			if test -n "$LIB_SUFFIX" ; then
				case $LIB_SUFFIX in
				w*)
					cf_libname=`echo $cf_libname | sed 's/w$//'`
					cf_suffix=`echo $cf_suffix | sed 's/^w//'`
					cf_dir_suffix=w
					;;
				esac
			fi
 			fi

			$AWK -f $srcdir/mk-1st.awk \
				name=${cf_libname}${cf_dir_suffix} \
				traces=$LIB_TRACING \
				MODEL=$cf_ITEM \
				model=$cf_subdir \
				prefix=$cf_prefix \
				suffix=$cf_suffix \
				subset=$cf_subset \
				TermlibRoot=$TINFO_NAME \
				TermlibSuffix=$TINFO_SUFFIX \
				ShlibVer=$cf_cv_shlib_version \
				ShlibVerInfix=$cf_cv_shlib_version_infix \
				ReLink=${cf_cv_do_relink-no} \
				DoLinks=$cf_cv_do_symlinks \
				rmSoLocs=$cf_cv_rm_so_locs \
				ldconfig="$LDCONFIG" \
				overwrite=$WITH_OVERWRITE \
				depend="$cf_depend" \
				host="$host" \
				$srcdir/$cf_dir/modules >>$cf_dir/Makefile

			cf_suffix="$old_cf_suffix"

			for cf_subdir2 in $cf_subdirs lib
			do
				test $cf_subdir = $cf_subdir2 && break
			done
			test "${cf_subset}.${cf_subdir2}" != "${cf_subset}.${cf_subdir}" && \
			$AWK -f $srcdir/mk-2nd.awk \
				name=$cf_dir \
				traces=$LIB_TRACING \
				MODEL=$cf_ITEM \
				model=$cf_subdir \
				subset=$cf_subset \
				srcdir=$srcdir \
				echo=$WITH_ECHO \
				crenames=$cf_cv_prog_CC_c_o \
				cxxrenames=$cf_cv_prog_CXX_c_o \
				$srcdir/$cf_dir/modules >>$cf_dir/Makefile
			cf_subdirs="$cf_subdirs $cf_subdir"
			done
		done
	fi

	echo '	cd '$cf_dir' && ${MAKE} ${CF_MFLAGS} [$]@' >>Makefile
done

for cf_dir in $SRC_SUBDIRS
do
	if test ! -d $srcdir/$cf_dir ; then
		continue
	fi

	if test -f $cf_dir/Makefile ; then
		case "$cf_dir" in
		Ada95) #(vi
			echo 'libs \' >> Makefile
			echo 'install.libs \' >> Makefile
			echo 'uninstall.libs ::' >> Makefile
			echo '	cd '$cf_dir' && ${MAKE} ${CF_MFLAGS} [$]@' >> Makefile
			;;
		esac
	fi

	if test -f $srcdir/$cf_dir/modules; then
		echo >> Makefile
		if test -f $srcdir/$cf_dir/headers; then
cat >> Makefile <<CF_EOF
install.includes \\
uninstall.includes \\
CF_EOF
		fi
if test "$cf_dir" != "c++" ; then
echo 'lint \' >> Makefile
fi
cat >> Makefile <<CF_EOF
libs \\
lintlib \\
install.libs \\
uninstall.libs \\
install.$cf_dir \\
uninstall.$cf_dir ::
	cd $cf_dir && \${MAKE} \${CF_MFLAGS} \[$]@
CF_EOF
	elif test -f $srcdir/$cf_dir/headers; then
cat >> Makefile <<CF_EOF

libs \\
install.libs \\
uninstall.libs \\
install.includes \\
uninstall.includes ::
	cd $cf_dir && \${MAKE} \${CF_MFLAGS} \[$]@
CF_EOF
fi
done

cat >> Makefile <<CF_EOF

install.libs uninstall.libs \\
install.data uninstall.data ::
$MAKE_TERMINFO	cd misc && \${MAKE} \${CF_MFLAGS} \[$]@

install.man \\
uninstall.man ::
	cd man && \${MAKE} \${CF_MFLAGS} \[$]@

distclean ::
	rm -f config.cache config.log config.status Makefile include/ncurses_cfg.h
	rm -f headers.sh headers.sed mk_shared_lib.sh
	rm -rf \${DIRS_TO_MAKE}
CF_EOF

# Special case: tack's manpage lives in its own directory.
if test -d tack ; then
if test -f $srcdir/$tack.h; then
cat >> Makefile <<CF_EOF

install.man \\
uninstall.man ::
	cd tack && \${MAKE} \${CF_MFLAGS} \[$]@
CF_EOF
fi
fi

dnl If we're installing into a subdirectory of /usr/include, etc., we should
dnl prepend the subdirectory's name to the "#include" paths.  It won't hurt
dnl anything, and will make it more standardized.  It's awkward to decide this
dnl at configuration because of quoting, so we'll simply make all headers
dnl installed via a script that can do the right thing.

rm -f headers.sed headers.sh

dnl ( generating this script makes the makefiles a little tidier :-)
echo creating headers.sh
cat >headers.sh <<CF_EOF
#! /bin/sh
# This shell script is generated by the 'configure' script.  It is invoked in a
# subdirectory of the build tree.  It generates a sed-script in the parent
# directory that is used to adjust includes for header files that reside in a
# subdirectory of /usr/include, etc.
PRG=""
while test \[$]# != 3
do
PRG="\$PRG \[$]1"; shift
done
DST=\[$]1
REF=\[$]2
SRC=\[$]3
TMPSRC=\${TMPDIR-/tmp}/\`basename \$SRC\`\$\$
TMPSED=\${TMPDIR-/tmp}/headers.sed\$\$
echo installing \$SRC in \$DST
CF_EOF

if test $WITH_CURSES_H = yes; then
	cat >>headers.sh <<CF_EOF
case \$DST in
/*/include/*)
	END=\`basename \$DST\`
	for i in \`cat \$REF/../*/headers |fgrep -v "#"\`
	do
		NAME=\`basename \$i\`
		echo "s/<\$NAME>/<\$END\/\$NAME>/" >> \$TMPSED
	done
	;;
*)
	echo "" >> \$TMPSED
	;;
esac
CF_EOF

else
	cat >>headers.sh <<CF_EOF
case \$DST in
/*/include/*)
	END=\`basename \$DST\`
	for i in \`cat \$REF/../*/headers |fgrep -v "#"\`
	do
		NAME=\`basename \$i\`
		if test "\$NAME" = "curses.h"
		then
			echo "s/<curses.h>/<ncurses.h>/" >> \$TMPSED
			NAME=ncurses.h
		fi
		echo "s/<\$NAME>/<\$END\/\$NAME>/" >> \$TMPSED
	done
	;;
*)
	echo "s/<curses.h>/<ncurses.h>/" >> \$TMPSED
	;;
esac
CF_EOF
fi
cat >>headers.sh <<CF_EOF
rm -f \$TMPSRC
sed -f \$TMPSED \$SRC > \$TMPSRC
NAME=\`basename \$SRC\`
CF_EOF
if test $WITH_CURSES_H != yes; then
	cat >>headers.sh <<CF_EOF
test "\$NAME" = "curses.h" && NAME=ncurses.h
CF_EOF
fi
cat >>headers.sh <<CF_EOF
# Just in case someone gzip'd manpages, remove the conflicting copy.
test -f \$DST/\$NAME.gz && rm -f \$DST/\$NAME.gz

eval \$PRG \$TMPSRC \$DST/\$NAME
rm -f \$TMPSRC \$TMPSED
CF_EOF

chmod 0755 headers.sh

for cf_dir in $SRC_SUBDIRS
do
	if test ! -d $srcdir/$cf_dir ; then
		continue
	fi

	if test -f $srcdir/$cf_dir/headers; then
		$AWK -f $srcdir/mk-hdr.awk \
			subset="$LIB_SUBSETS" \
			compat="$WITH_CURSES_H" \
			$srcdir/$cf_dir/headers >>$cf_dir/Makefile
	fi

	if test -f $srcdir/$cf_dir/modules; then
		if test "$cf_dir" != "c++" ; then
			cat >>$cf_dir/Makefile <<"CF_EOF"
depend : ${AUTO_SRC}
	makedepend -- ${CPPFLAGS} -- ${C_SRC}

# DO NOT DELETE THIS LINE -- make depend depends on it.
CF_EOF
		fi
	fi
done

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LIB_SONAME version: 3 updated: 2006/12/16 15:55:46
dnl -------------
dnl Find the and soname for the given shared library.  Set the cache variable
dnl cf_cv_$3_soname to this, unless it is not found.  Then set the cache
dnl variable to "unknown".
dnl
dnl $1 = headers
dnl $2 = code
dnl $3 = library name
AC_DEFUN([CF_LIB_SONAME],
[
AC_CACHE_CHECK(for soname of $3 library,cf_cv_$3_soname,[

cf_cv_$3_soname=unknown
if test "$cross_compiling" != yes ; then
cat >conftest.$ac_ext <<CF_EOF
$1
int main()
{
$2
	${cf_cv_main_return:-return}(0);
}
CF_EOF
cf_save_LIBS="$LIBS"
	LIBS="-l$3 $LIBS"
	if AC_TRY_EVAL(ac_compile) ; then
		if AC_TRY_EVAL(ac_link) ; then
			cf_cv_$3_soname=`ldd conftest$ac_exeext 2>/dev/null | sed -e 's,^.*/,,' -e 's, .*$,,' | fgrep lib$3.`
			test -z "$cf_cv_$3_soname" && cf_cv_$3_soname=unknown
		fi
	fi
rm -f conftest*
LIBS="$cf_save_LIBS"
fi
])
])
dnl ---------------------------------------------------------------------------
dnl CF_LIB_SUFFIX version: 15 updated: 2008/09/13 11:54:48
dnl -------------
dnl Compute the library file-suffix from the given model name
dnl $1 = model name
dnl $2 = variable to set (the nominal library suffix)
dnl $3 = dependency variable to set (actual filename)
dnl The variable $LIB_SUFFIX, if set, prepends the variable to set.
AC_DEFUN([CF_LIB_SUFFIX],
[
	AC_REQUIRE([CF_SUBST_NCURSES_VERSION])
	case $1 in
	libtool)
		$2='.la'
		$3=[$]$2
		;;
	normal)
		$2='.a'
		$3=[$]$2
		;;
	debug)
		$2='_g.a'
		$3=[$]$2
		;;
	profile)
		$2='_p.a'
		$3=[$]$2
		;;
	shared)
		case $cf_cv_system_name in
		cygwin*)
			$2='.dll'
			$3='.dll.a'
			;;
		darwin*)
			$2='.dylib'
			$3=[$]$2
			;;
		hpux*)
			case $target in
			ia64*)
				$2='.so'
				$3=[$]$2
				;;
			*)
				$2='.sl'
				$3=[$]$2
				;;
			esac
			;;
		*)	$2='.so'
			$3=[$]$2
			;;
		esac
	esac
	test -n "$LIB_SUFFIX" && $2="${LIB_SUFFIX}[$]{$2}"
	test -n "$LIB_SUFFIX" && $3="${LIB_SUFFIX}[$]{$3}"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LIB_TYPE version: 4 updated: 2000/10/20 22:57:49
dnl -----------
dnl Compute the string to append to -library from the given model name
dnl $1 = model name
dnl $2 = variable to set
dnl The variable $LIB_SUFFIX, if set, prepends the variable to set.
AC_DEFUN([CF_LIB_TYPE],
[
	case $1 in
	libtool) $2=''   ;;
	normal)  $2=''   ;;
	debug)   $2='_g' ;;
	profile) $2='_p' ;;
	shared)  $2=''   ;;
	esac
	test -n "$LIB_SUFFIX" && $2="${LIB_SUFFIX}[$]{$2}"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LINK_DATAONLY version: 8 updated: 2006/12/16 12:33:30
dnl ----------------
dnl Some systems have a non-ANSI linker that doesn't pull in modules that have
dnl only data (i.e., no functions), for example NeXT.  On those systems we'll
dnl have to provide wrappers for global tables to ensure they're linked
dnl properly.
AC_DEFUN([CF_LINK_DATAONLY],
[
AC_MSG_CHECKING([if data-only library module links])
AC_CACHE_VAL(cf_cv_link_dataonly,[
	rm -f conftest.a
	cat >conftest.$ac_ext <<EOF
#line __oline__ "configure"
int	testdata[[3]] = { 123, 456, 789 };
EOF
	if AC_TRY_EVAL(ac_compile) ; then
		mv conftest.o data.o && \
		( $AR $AR_OPTS conftest.a data.o ) 2>&AC_FD_CC 1>/dev/null
	fi
	rm -f conftest.$ac_ext data.o
	cat >conftest.$ac_ext <<EOF
#line __oline__ "configure"
int	testfunc()
{
#if defined(NeXT)
	${cf_cv_main_return:-return}(1);	/* I'm told this linker is broken */
#else
	extern int testdata[[3]];
	return testdata[[0]] == 123
	   &&  testdata[[1]] == 456
	   &&  testdata[[2]] == 789;
#endif
}
EOF
	if AC_TRY_EVAL(ac_compile); then
		mv conftest.o func.o && \
		( $AR $AR_OPTS conftest.a func.o ) 2>&AC_FD_CC 1>/dev/null
	fi
	rm -f conftest.$ac_ext func.o
	( eval $RANLIB conftest.a ) 2>&AC_FD_CC >/dev/null
	cf_saveLIBS="$LIBS"
	LIBS="conftest.a $LIBS"
	AC_TRY_RUN([
	int main()
	{
		extern int testfunc();
		${cf_cv_main_return:-return} (!testfunc());
	}
	],
	[cf_cv_link_dataonly=yes],
	[cf_cv_link_dataonly=no],
	[cf_cv_link_dataonly=unknown])
	LIBS="$cf_saveLIBS"
	])
AC_MSG_RESULT($cf_cv_link_dataonly)

if test "$cf_cv_link_dataonly" = no ; then
	AC_DEFINE(BROKEN_LINKER)
	BROKEN_LINKER=1
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_LINK_FUNCS version: 7 updated: 2006/12/16 12:33:30
dnl -------------
dnl Most Unix systems have both link and symlink, a few don't have symlink.
dnl A few non-Unix systems implement symlink, but not link.
dnl A few non-systems implement neither (or have nonfunctional versions).
AC_DEFUN([CF_LINK_FUNCS],
[
AC_CHECK_FUNCS( \
	remove \
	unlink )

if test "$cross_compiling" = yes ; then
	AC_CHECK_FUNCS( \
		link \
		symlink )
else
	AC_CACHE_CHECK(if link/symlink functions work,cf_cv_link_funcs,[
		cf_cv_link_funcs=
		for cf_func in link symlink ; do
			AC_TRY_RUN([
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
int main()
{
	int fail = 0;
	char *src = "config.log";
	char *dst = "conftest.chk";
	struct stat src_sb;
	struct stat dst_sb;

	stat(src, &src_sb);
	fail = ($cf_func("config.log", "conftest.chk") < 0)
	    || (stat(dst, &dst_sb) < 0)
	    || (dst_sb.st_mtime != src_sb.st_mtime);
#ifdef HAVE_UNLINK
	unlink(dst);
#else
	remove(dst);
#endif
	${cf_cv_main_return:-return} (fail);
}
			],[
			cf_cv_link_funcs="$cf_cv_link_funcs $cf_func"
			eval 'ac_cv_func_'$cf_func'=yes'],[
			eval 'ac_cv_func_'$cf_func'=no'],[
			eval 'ac_cv_func_'$cf_func'=error'])
		done
		test -z "$cf_cv_link_funcs" && cf_cv_link_funcs=no
	])
	test "$ac_cv_func_link"    = yes && AC_DEFINE(HAVE_LINK)
	test "$ac_cv_func_symlink" = yes && AC_DEFINE(HAVE_SYMLINK)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAIN_RETURN version: 1 updated: 2006/12/10 09:51:54
dnl --------------
dnl Check if a return from main to the shell actually returns the same exit
dnl code.  This is true for almost any POSIX environment.
dnl
dnl Some very old environments did not flush stdout, etc., on an exit.  That
dnl would be a useful case to test for also.
AC_DEFUN([CF_MAIN_RETURN],
[
cf_cv_main_return=return
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKEFLAGS version: 12 updated: 2006/10/21 08:27:03
dnl ------------
dnl Some 'make' programs support ${MAKEFLAGS}, some ${MFLAGS}, to pass 'make'
dnl options to lower-levels.  It's very useful for "make -n" -- if we have it.
dnl (GNU 'make' does both, something POSIX 'make', which happens to make the
dnl ${MAKEFLAGS} variable incompatible because it adds the assignments :-)
AC_DEFUN([CF_MAKEFLAGS],
[
AC_CACHE_CHECK(for makeflags variable, cf_cv_makeflags,[
	cf_cv_makeflags=''
	for cf_option in '-${MAKEFLAGS}' '${MFLAGS}'
	do
		cat >cf_makeflags.tmp <<CF_EOF
SHELL = /bin/sh
all :
	@ echo '.$cf_option'
CF_EOF
		cf_result=`${MAKE-make} -k -f cf_makeflags.tmp 2>/dev/null | sed -e 's,[[ 	]]*$,,'`
		case "$cf_result" in
		.*k)
			cf_result=`${MAKE-make} -k -f cf_makeflags.tmp CC=cc 2>/dev/null`
			case "$cf_result" in
			.*CC=*)	cf_cv_makeflags=
				;;
			*)	cf_cv_makeflags=$cf_option
				;;
			esac
			break
			;;
		.-)	;;
		*)	echo "given option \"$cf_option\", no match \"$cf_result\""
			;;
		esac
	done
	rm -f cf_makeflags.tmp
])

AC_SUBST(cf_cv_makeflags)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKE_TAGS version: 2 updated: 2000/10/04 09:18:40
dnl ------------
dnl Generate tags/TAGS targets for makefiles.  Do not generate TAGS if we have
dnl a monocase filesystem.
AC_DEFUN([CF_MAKE_TAGS],[
AC_REQUIRE([CF_MIXEDCASE_FILENAMES])
AC_CHECK_PROG(MAKE_LOWER_TAGS, ctags, yes, no)

if test "$cf_cv_mixedcase" = yes ; then
	AC_CHECK_PROG(MAKE_UPPER_TAGS, etags, yes, no)
else
	MAKE_UPPER_TAGS=no
fi

if test "$MAKE_UPPER_TAGS" = yes ; then
	MAKE_UPPER_TAGS=
else
	MAKE_UPPER_TAGS="#"
fi
AC_SUBST(MAKE_UPPER_TAGS)

if test "$MAKE_LOWER_TAGS" = yes ; then
	MAKE_LOWER_TAGS=
else
	MAKE_LOWER_TAGS="#"
fi
AC_SUBST(MAKE_LOWER_TAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MANPAGE_FORMAT version: 7 updated: 2003/12/20 19:30:34
dnl -----------------
dnl Option to allow user to override automatic configuration of manpage format.
dnl There are several special cases:
dnl
dnl	gzip - man checks for, can display gzip'd files
dnl	compress - man checks for, can display compressed files
dnl	BSDI - files in the cat-directories are suffixed ".0"
dnl	formatted - installer should format (put files in cat-directory)
dnl	catonly - installer should only format, e.g., for a turnkey system.
dnl
dnl There are other configurations which this macro does not test, e.g., HPUX's
dnl compressed manpages (but uncompressed manpages are fine, and HPUX's naming
dnl convention would not match our use).
AC_DEFUN([CF_MANPAGE_FORMAT],
[
AC_REQUIRE([CF_PATHSEP])
AC_MSG_CHECKING(format of man-pages)

AC_ARG_WITH(manpage-format,
	[  --with-manpage-format   specify manpage-format: gzip/compress/BSDI/normal and
                          optionally formatted/catonly, e.g., gzip,formatted],
	[MANPAGE_FORMAT=$withval],
	[MANPAGE_FORMAT=unknown])

test -z "$MANPAGE_FORMAT" && MANPAGE_FORMAT=unknown
MANPAGE_FORMAT=`echo "$MANPAGE_FORMAT" | sed -e 's/,/ /g'`

cf_unknown=

case $MANPAGE_FORMAT in
unknown)
  if test -z "$MANPATH" ; then
    MANPATH="/usr/man:/usr/share/man"
  fi

  # look for the 'date' man-page (it's most likely to be installed!)
  MANPAGE_FORMAT=
  cf_preform=no
  cf_catonly=yes
  cf_example=date

  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATHSEP}"
  for cf_dir in $MANPATH; do
    test -z "$cf_dir" && cf_dir=/usr/man
    for cf_name in $cf_dir/man*/$cf_example.[[01]]* $cf_dir/cat*/$cf_example.[[01]]* $cf_dir/man*/$cf_example $cf_dir/cat*/$cf_example
    do
      cf_test=`echo $cf_name | sed -e 's/*//'`
      if test "x$cf_test" = "x$cf_name" ; then

	case "$cf_name" in
	*.gz) MANPAGE_FORMAT="$MANPAGE_FORMAT gzip";;
	*.Z)  MANPAGE_FORMAT="$MANPAGE_FORMAT compress";;
	*.0)	MANPAGE_FORMAT="$MANPAGE_FORMAT BSDI";;
	*)    MANPAGE_FORMAT="$MANPAGE_FORMAT normal";;
	esac

	case "$cf_name" in
	$cf_dir/man*)
	  cf_catonly=no
	  ;;
	$cf_dir/cat*)
	  cf_preform=yes
	  ;;
	esac
	break
      fi

      # if we found a match in either man* or cat*, stop looking
      if test -n "$MANPAGE_FORMAT" ; then
	cf_found=no
	test "$cf_preform" = yes && MANPAGE_FORMAT="$MANPAGE_FORMAT formatted"
	test "$cf_catonly" = yes && MANPAGE_FORMAT="$MANPAGE_FORMAT catonly"
	case "$cf_name" in
	$cf_dir/cat*)
	  cf_found=yes
	  ;;
	esac
	test $cf_found=yes && break
      fi
    done
    # only check the first directory in $MANPATH where we find manpages
    if test -n "$MANPAGE_FORMAT" ; then
       break
    fi
  done
  # if we did not find the example, just assume it is normal
  test -z "$MANPAGE_FORMAT" && MANPAGE_FORMAT=normal
  IFS="$ac_save_ifs"
  ;;
*)
  for cf_option in $MANPAGE_FORMAT; do
     case $cf_option in #(vi
     gzip|compress|BSDI|normal|formatted|catonly)
       ;;
     *)
       cf_unknown="$cf_unknown $cf_option"
       ;;
     esac
  done
  ;;
esac

AC_MSG_RESULT($MANPAGE_FORMAT)
if test -n "$cf_unknown" ; then
  AC_MSG_WARN(Unexpected manpage-format $cf_unknown)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MANPAGE_RENAMES version: 7 updated: 2005/06/18 18:51:57
dnl ------------------
dnl The Debian people have their own naming convention for manpages.  This
dnl option lets us override the name of the file containing renaming, or
dnl disable it altogether.
AC_DEFUN([CF_MANPAGE_RENAMES],
[
AC_MSG_CHECKING(for manpage renaming)

AC_ARG_WITH(manpage-renames,
	[  --with-manpage-renames  specify manpage-renaming],
	[MANPAGE_RENAMES=$withval],
	[MANPAGE_RENAMES=yes])

case ".$MANPAGE_RENAMES" in #(vi
.no) #(vi
  ;;
.|.yes)
  # Debian 'man' program?
  if test -f /etc/debian_version ; then
    MANPAGE_RENAMES=`cd $srcdir && pwd`/man/man_db.renames
  else
    MANPAGE_RENAMES=no
  fi
  ;;
esac

if test "$MANPAGE_RENAMES" != no ; then
  if test -f $srcdir/man/$MANPAGE_RENAMES ; then
    MANPAGE_RENAMES=`cd $srcdir/man && pwd`/$MANPAGE_RENAMES
  elif test ! -f $MANPAGE_RENAMES ; then
    AC_MSG_ERROR(not a filename: $MANPAGE_RENAMES)
  fi

  test ! -d man && mkdir man

  # Construct a sed-script to perform renaming within man-pages
  if test -n "$MANPAGE_RENAMES" ; then
    test ! -d man && mkdir man
    sh $srcdir/man/make_sed.sh $MANPAGE_RENAMES >./edit_man.sed
  fi
fi

AC_MSG_RESULT($MANPAGE_RENAMES)
AC_SUBST(MANPAGE_RENAMES)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MANPAGE_SYMLINKS version: 4 updated: 2003/12/13 18:01:58
dnl -------------------
dnl Some people expect each tool to make all aliases for manpages in the
dnl man-directory.  This accommodates the older, less-capable implementations
dnl of 'man', and is optional.
AC_DEFUN([CF_MANPAGE_SYMLINKS],
[
AC_MSG_CHECKING(if manpage aliases will be installed)

AC_ARG_WITH(manpage-aliases,
	[  --with-manpage-aliases  specify manpage-aliases using .so],
	[MANPAGE_ALIASES=$withval],
	[MANPAGE_ALIASES=yes])

AC_MSG_RESULT($MANPAGE_ALIASES)

if test "$LN_S" = "ln -s"; then
	cf_use_symlinks=yes
else
	cf_use_symlinks=no
fi

MANPAGE_SYMLINKS=no
if test "$MANPAGE_ALIASES" = yes ; then
AC_MSG_CHECKING(if manpage symlinks should be used)

AC_ARG_WITH(manpage-symlinks,
	[  --with-manpage-symlinks specify manpage-aliases using symlinks],
	[MANPAGE_SYMLINKS=$withval],
	[MANPAGE_SYMLINKS=$cf_use_symlinks])

if test "$$cf_use_symlinks" = no; then
if test "$MANPAGE_SYMLINKS" = yes ; then
	AC_MSG_WARN(cannot make symlinks, will use .so files)
	MANPAGE_SYMLINKS=no
fi
fi

AC_MSG_RESULT($MANPAGE_SYMLINKS)
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MANPAGE_TBL version: 3 updated: 2002/01/19 22:51:32
dnl --------------
dnl This option causes manpages to be run through tbl(1) to generate tables
dnl correctly.
AC_DEFUN([CF_MANPAGE_TBL],
[
AC_MSG_CHECKING(for manpage tbl)

AC_ARG_WITH(manpage-tbl,
	[  --with-manpage-tbl      specify manpage processing with tbl],
	[MANPAGE_TBL=$withval],
	[MANPAGE_TBL=no])

AC_MSG_RESULT($MANPAGE_TBL)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAN_PAGES version: 35 updated: 2007/03/31 11:47:29
dnl ------------
dnl Try to determine if the man-pages on the system are compressed, and if
dnl so, what format is used.  Use this information to construct a script that
dnl will install man-pages.
AC_DEFUN([CF_MAN_PAGES],
[
CF_HELP_MESSAGE(Options to Specify How Manpages are Installed:)
CF_MANPAGE_FORMAT
CF_MANPAGE_RENAMES
CF_MANPAGE_SYMLINKS
CF_MANPAGE_TBL

  if test "$prefix" = "NONE" ; then
     cf_prefix="$ac_default_prefix"
  else
     cf_prefix="$prefix"
  fi

  case "$MANPAGE_FORMAT" in # (vi
  *catonly*) # (vi
    cf_format=yes
    cf_inboth=no
    ;;
  *formatted*) # (vi
    cf_format=yes
    cf_inboth=yes
    ;;
  *)
    cf_format=no
    cf_inboth=no
    ;;
  esac

test ! -d man && mkdir man

cf_so_strip=
cf_compress=
case "$MANPAGE_FORMAT" in #(vi
*compress*) #(vi
	cf_so_strip="Z"
	cf_compress=compress
  ;;
*gzip*) #(vi
	cf_so_strip="gz"
	cf_compress=gzip
  ;;
esac

cf_edit_man=./edit_man.sh
cf_man_alias=`pwd`/man_alias.sed

cat >$cf_edit_man <<CF_EOF
#! /bin/sh
# this script is generated by the configure-script CF_MAN_PAGES macro.

prefix="$cf_prefix"
datadir="$datadir"

NCURSES_MAJOR="$NCURSES_MAJOR"
NCURSES_MINOR="$NCURSES_MINOR"
NCURSES_PATCH="$NCURSES_PATCH"

NCURSES_OSPEED="$NCURSES_OSPEED"
TERMINFO="$TERMINFO"

MKDIRS="sh `cd $srcdir && pwd`/mkdirs.sh"

INSTALL="$INSTALL"
INSTALL_DATA="$INSTALL_DATA"

transform="$program_transform_name"

TMP=\${TMPDIR-/tmp}/man\$\$
trap "rm -f \$TMP" 0 1 2 5 15

form=\[$]1
shift || exit 1

verb=\[$]1
shift || exit 1

mandir=\[$]1
shift || exit 1

srcdir=\[$]1
top_srcdir=\[$]srcdir/..
shift || exit 1

if test "\$form" = normal ; then
	if test "$cf_format" = yes ; then
	if test "$cf_inboth" = no ; then
		sh \[$]0 format \$verb \$mandir \$srcdir \[$]*
		exit $?
	fi
	fi
	cf_subdir=\$mandir/man
	cf_tables=$MANPAGE_TBL
else
	cf_subdir=\$mandir/cat
	cf_tables=yes
fi

# process the list of source-files
for i in \[$]* ; do
case \$i in #(vi
*.orig|*.rej) ;; #(vi
*.[[0-9]]*)
	section=\`expr "\$i" : '.*\\.\\([[0-9]]\\)[[xm]]*'\`;
	if test \$verb = installing ; then
	if test ! -d \$cf_subdir\${section} ; then
		\$MKDIRS \$cf_subdir\$section
	fi
	fi

	# replace variables in man page
	if test ! -f $cf_man_alias ; then
cat >>$cf_man_alias <<-CF_EOF2
		s,@DATADIR@,\$datadir,g
		s,@TERMINFO@,\$TERMINFO,g
		s,@NCURSES_MAJOR@,\$NCURSES_MAJOR,g
		s,@NCURSES_MINOR@,\$NCURSES_MINOR,g
		s,@NCURSES_PATCH@,\$NCURSES_PATCH,g
		s,@NCURSES_OSPEED@,\$NCURSES_OSPEED,g
CF_EOF
	ifelse($1,,,[
	for cf_name in $1
	do
		cf_NAME=`echo "$cf_name" | sed y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%`
		cf_name=`echo $cf_name|sed "$program_transform_name"`
cat >>$cf_edit_man <<-CF_EOF
		s,@$cf_NAME@,$cf_name,
CF_EOF
	done
	])
cat >>$cf_edit_man <<CF_EOF
CF_EOF2
		echo "...made $cf_man_alias"
	fi

	aliases=
	cf_source=\`basename \$i\`
	inalias=\$cf_source
	test ! -f \$inalias && inalias="\$srcdir/\$inalias"
	if test ! -f \$inalias ; then
		echo .. skipped \$cf_source
		continue
	fi
CF_EOF

if test "$MANPAGE_ALIASES" != no ; then
cat >>$cf_edit_man <<CF_EOF
	aliases=\`sed -f \$top_srcdir/man/manlinks.sed \$inalias |sed -f $cf_man_alias | sort -u\`
CF_EOF
fi

if test "$MANPAGE_RENAMES" = no ; then
cat >>$cf_edit_man <<CF_EOF
	# perform program transformations for section 1 man pages
	if test \$section = 1 ; then
		cf_target=\$cf_subdir\${section}/\`echo \$cf_source|sed "\${transform}"\`
	else
		cf_target=\$cf_subdir\${section}/\$cf_source
	fi
CF_EOF
else
cat >>$cf_edit_man <<CF_EOF
	cf_target=\`grep "^\$cf_source" $MANPAGE_RENAMES | $AWK '{print \[$]2}'\`
	if test -z "\$cf_target" ; then
		echo '? missing rename for '\$cf_source
		cf_target="\$cf_source"
	fi
	cf_target="\$cf_subdir\${section}/\${cf_target}"

CF_EOF
fi

cat >>$cf_edit_man <<CF_EOF
	sed	-f $cf_man_alias \\
CF_EOF

if test -f $MANPAGE_RENAMES ; then
cat >>$cf_edit_man <<CF_EOF
		< \$i | sed -f `pwd`/edit_man.sed >\$TMP
CF_EOF
else
cat >>$cf_edit_man <<CF_EOF
		< \$i >\$TMP
CF_EOF
fi

cat >>$cf_edit_man <<CF_EOF
if test \$cf_tables = yes ; then
	tbl \$TMP >\$TMP.out
	mv \$TMP.out \$TMP
fi
CF_EOF

if test $with_curses_h != yes ; then
cat >>$cf_edit_man <<CF_EOF
	sed -e "/\#[    ]*include/s,curses.h,ncurses.h," < \$TMP >\$TMP.out
	mv \$TMP.out \$TMP
CF_EOF
fi

cat >>$cf_edit_man <<CF_EOF
	if test \$form = format ; then
		nroff -man \$TMP >\$TMP.out
		mv \$TMP.out \$TMP
	fi
CF_EOF

if test -n "$cf_compress" ; then
cat >>$cf_edit_man <<CF_EOF
	if test \$verb = installing ; then
	if ( $cf_compress -f \$TMP )
	then
		mv \$TMP.$cf_so_strip \$TMP
	fi
	fi
	cf_target="\$cf_target.$cf_so_strip"
CF_EOF
fi

case "$MANPAGE_FORMAT" in #(vi
*BSDI*)
cat >>$cf_edit_man <<CF_EOF
	if test \$form = format ; then
		# BSDI installs only .0 suffixes in the cat directories
		cf_target="\`echo \$cf_target|sed -e 's/\.[[1-9]]\+[[a-z]]*/.0/'\`"
	fi
CF_EOF
  ;;
esac

cat >>$cf_edit_man <<CF_EOF
	suffix=\`basename \$cf_target | sed -e 's%^[[^.]]*%%'\`
	if test \$verb = installing ; then
		echo \$verb \$cf_target
		\$INSTALL_DATA \$TMP \$cf_target
		test -d \$cf_subdir\${section} &&
		test -n "\$aliases" && (
			cd \$cf_subdir\${section} && (
				cf_source=\`echo \$cf_target |sed -e 's%^.*/\([[^/]][[^/]]*/[[^/]][[^/]]*$\)%\1%'\`
				test -n "$cf_so_strip" && cf_source=\`echo \$cf_source |sed -e 's%\.$cf_so_strip\$%%'\`
				cf_target=\`basename \$cf_target\`
				for cf_alias in \$aliases
				do
					if test \$section = 1 ; then
						cf_alias=\`echo \$cf_alias|sed "\${transform}"\`
					fi

					if test "$MANPAGE_SYMLINKS" = yes ; then
						if test -f \$cf_alias\${suffix} ; then
							if ( cmp -s \$cf_target \$cf_alias\${suffix} )
							then
								continue
							fi
						fi
						echo .. \$verb alias \$cf_alias\${suffix}
						rm -f \$cf_alias\${suffix}
						$LN_S \$cf_target \$cf_alias\${suffix}
					elif test "\$cf_target" != "\$cf_alias\${suffix}" ; then
						echo ".so \$cf_source" >\$TMP
CF_EOF
if test -n "$cf_compress" ; then
cat >>$cf_edit_man <<CF_EOF
						if test -n "$cf_so_strip" ; then
							$cf_compress -f \$TMP
							mv \$TMP.$cf_so_strip \$TMP
						fi
CF_EOF
fi
cat >>$cf_edit_man <<CF_EOF
						echo .. \$verb alias \$cf_alias\${suffix}
						rm -f \$cf_alias\${suffix}
						\$INSTALL_DATA \$TMP \$cf_alias\${suffix}
					fi
				done
			)
		)
	elif test \$verb = removing ; then
		test -f \$cf_target && (
			echo \$verb \$cf_target
			rm -f \$cf_target
		)
		test -d \$cf_subdir\${section} &&
		test -n "\$aliases" && (
			cd \$cf_subdir\${section} && (
				for cf_alias in \$aliases
				do
					if test \$section = 1 ; then
						cf_alias=\`echo \$cf_alias|sed "\${transform}"\`
					fi

					echo .. \$verb alias \$cf_alias\${suffix}
					rm -f \$cf_alias\${suffix}
				done
			)
		)
	else
#		echo ".hy 0"
		cat \$TMP
	fi
	;;
esac
done

if test $cf_inboth = yes ; then
if test \$form != format ; then
	sh \[$]0 format \$verb \$mandir \$srcdir \[$]*
fi
fi

exit 0
CF_EOF
chmod 755 $cf_edit_man

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MATH_LIB version: 5 updated: 2000/05/28 01:39:10
dnl -----------
dnl Checks for libraries.  At least one UNIX system, Apple Macintosh
dnl Rhapsody 5.5, does not have -lm.  We cannot use the simpler
dnl AC_CHECK_LIB(m,sin), because that fails for C++.
AC_DEFUN([CF_MATH_LIB],
[
AC_CACHE_CHECK(if -lm needed for math functions,
	cf_cv_need_libm,[
	AC_TRY_LINK([
	#include <stdio.h>
	#include <math.h>
	],
	[double x = rand(); printf("result = %g\n", ]ifelse($2,,sin(x),$2)[)],
	[cf_cv_need_libm=no],
	[cf_cv_need_libm=yes])])
if test "$cf_cv_need_libm" = yes
then
ifelse($1,,[
	LIBS="$LIBS -lm"
],[$1=-lm])
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_MIXEDCASE_FILENAMES version: 3 updated: 2003/09/20 17:07:55
dnl ----------------------
dnl Check if the file-system supports mixed-case filenames.  If we're able to
dnl create a lowercase name and see it as uppercase, it doesn't support that.
AC_DEFUN([CF_MIXEDCASE_FILENAMES],
[
AC_CACHE_CHECK(if filesystem supports mixed-case filenames,cf_cv_mixedcase,[
if test "$cross_compiling" = yes ; then
	case $target_alias in #(vi
	*-os2-emx*|*-msdosdjgpp*|*-cygwin*|*-mingw32*|*-uwin*) #(vi
		cf_cv_mixedcase=no
		;;
	*)
		cf_cv_mixedcase=yes
		;;
	esac
else
	rm -f conftest CONFTEST
	echo test >conftest
	if test -f CONFTEST ; then
		cf_cv_mixedcase=no
	else
		cf_cv_mixedcase=yes
	fi
	rm -f conftest CONFTEST
fi
])
test "$cf_cv_mixedcase" = yes && AC_DEFINE(MIXEDCASE_FILENAMES)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MKSTEMP version: 5 updated: 2006/12/16 12:33:30
dnl ----------
dnl Check for a working mkstemp.  This creates two files, checks that they are
dnl successfully created and distinct (AmigaOS apparently fails on the last).
AC_DEFUN([CF_MKSTEMP],[
AC_CACHE_CHECK(for working mkstemp, cf_cv_func_mkstemp,[
rm -f conftest*
AC_TRY_RUN([
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
int main()
{
	char *tmpl = "conftestXXXXXX";
	char name[2][80];
	int n;
	int result = 0;
	int fd;
	struct stat sb;

	umask(077);
	for (n = 0; n < 2; ++n) {
		strcpy(name[n], tmpl);
		if ((fd = mkstemp(name[n])) >= 0) {
			if (!strcmp(name[n], tmpl)
			 || stat(name[n], &sb) != 0
			 || (sb.st_mode & S_IFMT) != S_IFREG
			 || (sb.st_mode & 077) != 0) {
				result = 1;
			}
			close(fd);
		}
	}
	if (result == 0
	 && !strcmp(name[0], name[1]))
		result = 1;
	${cf_cv_main_return:-return}(result);
}
],[cf_cv_func_mkstemp=yes
],[cf_cv_func_mkstemp=no
],[AC_CHECK_FUNC(mkstemp)
])
])
if test "$cf_cv_func_mkstemp" = yes ; then
	AC_DEFINE(HAVE_MKSTEMP)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MSG_LOG version: 4 updated: 2007/07/29 09:55:12
dnl ----------
dnl Write a debug message to config.log, along with the line number in the
dnl configure script.
AC_DEFUN([CF_MSG_LOG],[
echo "${as_me-configure}:__oline__: testing $* ..." 1>&AC_FD_CC
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NCURSES_ABI_6 version: 1 updated: 2005/09/17 18:42:49
dnl ----------------
dnl Set ncurses' ABI to 6 unless overridden by explicit configure option, and
dnl warn about this.
AC_DEFUN([CF_NCURSES_ABI_6],[
if test "${with_abi_version+set}" != set; then
	case $cf_cv_rel_version in
	5.*)
		cf_cv_rel_version=6.0
		cf_cv_abi_version=6
		AC_MSG_WARN(Overriding ABI version to $cf_cv_abi_version)
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NO_LEAKS_OPTION version: 4 updated: 2006/12/16 14:24:05
dnl ------------------
dnl see CF_WITH_NO_LEAKS
AC_DEFUN([CF_NO_LEAKS_OPTION],[
AC_MSG_CHECKING(if you want to use $1 for testing)
AC_ARG_WITH($1,
	[$2],
	[AC_DEFINE($3)ifelse([$4],,[
	 $4
])
	: ${with_cflags:=-g}
	: ${with_no_leaks:=yes}
	 with_$1=yes],
	[with_$1=])
AC_MSG_RESULT(${with_$1:-no})

case .$with_cflags in #(vi
.*-g*)
	case .$CFLAGS in #(vi
	.*-g*) #(vi
		;;
	*)
		CF_ADD_CFLAGS([-g])
		;;
	esac
	;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_NUMBER_SYNTAX version: 1 updated: 2003/09/20 18:12:49
dnl ----------------
dnl Check if the given variable is a number.  If not, report an error.
dnl $1 is the variable
dnl $2 is the message
AC_DEFUN([CF_NUMBER_SYNTAX],[
if test -n "$1" ; then
  case $1 in #(vi
  [[0-9]]*) #(vi
 	;;
  *)
	AC_MSG_ERROR($2 is not a number: $1)
 	;;
  esac
else
  AC_MSG_ERROR($2 value is empty)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_OBJ_SUBDIR version: 4 updated: 2002/02/23 20:38:31
dnl -------------
dnl Compute the object-directory name from the given model name
AC_DEFUN([CF_OBJ_SUBDIR],
[
	case $1 in
	libtool) $2='obj_lo'  ;;
	normal)  $2='objects' ;;
	debug)   $2='obj_g' ;;
	profile) $2='obj_p' ;;
	shared)
		case $cf_cv_system_name in #(vi
		cygwin) #(vi
			$2='objects' ;;
		*)
			$2='obj_s' ;;
		esac
	esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATHSEP version: 3 updated: 2001/01/12 01:23:53
dnl ----------
dnl Provide a value for the $PATH and similar separator
AC_DEFUN([CF_PATHSEP],
[
	case $cf_cv_system_name in
	os2*)	PATHSEP=';'  ;;
	*)	PATHSEP=':'  ;;
	esac
ifelse($1,,,[$1=$PATHSEP])
	AC_SUBST(PATHSEP)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PATH_SYNTAX version: 12 updated: 2008/03/23 14:45:59
dnl --------------
dnl Check the argument to see that it looks like a pathname.  Rewrite it if it
dnl begins with one of the prefix/exec_prefix variables, and then again if the
dnl result begins with 'NONE'.  This is necessary to work around autoconf's
dnl delayed evaluation of those symbols.
AC_DEFUN([CF_PATH_SYNTAX],[
if test "x$prefix" != xNONE; then
  cf_path_syntax="$prefix"
else
  cf_path_syntax="$ac_default_prefix"
fi

case ".[$]$1" in #(vi
.\[$]\(*\)*|.\'*\'*) #(vi
  ;;
..|./*|.\\*) #(vi
  ;;
.[[a-zA-Z]]:[[\\/]]*) #(vi OS/2 EMX
  ;;
.\[$]{*prefix}*) #(vi
  eval $1="[$]$1"
  case ".[$]$1" in #(vi
  .NONE/*)
    $1=`echo [$]$1 | sed -e s%NONE%$cf_path_syntax%`
    ;;
  esac
  ;; #(vi
.no|.NONE/*)
  $1=`echo [$]$1 | sed -e s%NONE%$cf_path_syntax%`
  ;;
*)
  ifelse($2,,[AC_MSG_ERROR([expected a pathname, not \"[$]$1\"])],$2)
  ;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_C_SOURCE version: 6 updated: 2005/07/14 20:25:10
dnl -----------------
dnl Define _POSIX_C_SOURCE to the given level, and _POSIX_SOURCE if needed.
dnl
dnl	POSIX.1-1990				_POSIX_SOURCE
dnl	POSIX.1-1990 and			_POSIX_SOURCE and
dnl		POSIX.2-1992 C-Language			_POSIX_C_SOURCE=2
dnl		Bindings Option
dnl	POSIX.1b-1993				_POSIX_C_SOURCE=199309L
dnl	POSIX.1c-1996				_POSIX_C_SOURCE=199506L
dnl	X/Open 2000				_POSIX_C_SOURCE=200112L
dnl
dnl Parameters:
dnl	$1 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_POSIX_C_SOURCE],
[
cf_POSIX_C_SOURCE=ifelse($1,,199506L,$1)

cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"

CF_REMOVE_DEFINE(cf_trim_CFLAGS,$cf_save_CFLAGS,_POSIX_C_SOURCE)
CF_REMOVE_DEFINE(cf_trim_CPPFLAGS,$cf_save_CPPFLAGS,_POSIX_C_SOURCE)

AC_CACHE_CHECK(if we should define _POSIX_C_SOURCE,cf_cv_posix_c_source,[
	CF_MSG_LOG(if the symbol is already defined go no further)
	AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
make an error
#endif],
	[cf_cv_posix_c_source=no],
	[cf_want_posix_source=no
	 case .$cf_POSIX_C_SOURCE in #(vi
	 .[[12]]??*) #(vi
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		;;
	 .2) #(vi
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		cf_want_posix_source=yes
		;;
	 .*)
		cf_want_posix_source=yes
		;;
	 esac
	 if test "$cf_want_posix_source" = yes ; then
		AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _POSIX_SOURCE
make an error
#endif],[],
		cf_cv_posix_c_source="$cf_cv_posix_c_source -D_POSIX_SOURCE")
	 fi
	 CF_MSG_LOG(ifdef from value $cf_POSIX_C_SOURCE)
	 CFLAGS="$cf_trim_CFLAGS"
	 CPPFLAGS="$cf_trim_CPPFLAGS $cf_cv_posix_c_source"
	 CF_MSG_LOG(if the second compile does not leave our definition intact error)
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
make an error
#endif],,
	 [cf_cv_posix_c_source=no])
	 CFLAGS="$cf_save_CFLAGS"
	 CPPFLAGS="$cf_save_CPPFLAGS"
	])
])

if test "$cf_cv_posix_c_source" != no ; then
	CFLAGS="$cf_trim_CFLAGS"
	CPPFLAGS="$cf_trim_CPPFLAGS"
	if test "$cf_cv_cc_u_d_options" = yes ; then
		cf_temp_posix_c_source=`echo "$cf_cv_posix_c_source" | \
				sed -e 's/-D/-U/g' -e 's/=[[^ 	]]*//g'`
		CPPFLAGS="$CPPFLAGS $cf_temp_posix_c_source"
	fi
	CPPFLAGS="$CPPFLAGS $cf_cv_posix_c_source"
fi

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PREDEFINE version: 1 updated: 2003/07/26 17:53:56
dnl ------------
dnl Add definitions to CPPFLAGS to ensure they're predefined for all compiles.
dnl
dnl $1 = symbol to test
dnl $2 = value (if any) to use for a predefinition
AC_DEFUN([CF_PREDEFINE],
[
AC_MSG_CHECKING(if we must define $1)
AC_TRY_COMPILE([#include <sys/types.h>
],[
#ifndef $1
make an error
#endif],[cf_result=no],[cf_result=yes])
AC_MSG_RESULT($cf_result)

if test "$cf_result" = yes ; then
	CPPFLAGS="$CPPFLAGS ifelse($2,,-D$1,[-D$1=$2])"
elif test "x$2" != "x" ; then
	AC_MSG_CHECKING(checking for compatible value versus $2)
	AC_TRY_COMPILE([#include <sys/types.h>
],[
#if $1-$2 < 0
make an error
#endif],[cf_result=yes],[cf_result=no])
	AC_MSG_RESULT($cf_result)
	if test "$cf_result" = no ; then
		# perhaps we can override it - try...
		CPPFLAGS="$CPPFLAGS -D$1=$2"
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PRG_RULES version: 1 updated: 2006/06/03 11:45:08
dnl ------------
dnl Append definitions and rules for the given programs to the subdirectory
dnl Makefiles, and the recursion rule for the top-level Makefile.
dnl
dnl parameters
dnl	$1 = script to run
dnl	$2 = list of subdirectories
dnl
dnl variables
dnl	$AWK
AC_DEFUN([CF_PRG_RULES],
[
for cf_dir in $2
do
	if test ! -d $srcdir/$cf_dir; then
		continue
	elif test -f $srcdir/$cf_dir/programs; then
		$AWK -f $1 $srcdir/$cf_dir/programs >>$cf_dir/Makefile
	fi
done

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_AWK version: 1 updated: 2006/09/16 11:40:59
dnl -----------
dnl Check for awk, ensure that the check found something.
AC_DEFUN([CF_PROG_AWK],
[
AC_PROG_AWK
test -z "$AWK" && AC_MSG_ERROR(No awk program found)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_CC_C_O version: 2 updated: 2006/12/16 15:55:46
dnl --------------
dnl Analogous to AC_PROG_CC_C_O, but more useful: tests only $CC, ensures that
dnl the output file can be renamed, and allows for a shell variable that can
dnl be used later.  The parameter is either CC or CXX.  The result is the
dnl cache variable:
dnl	$cf_cv_prog_CC_c_o
dnl	$cf_cv_prog_CXX_c_o
AC_DEFUN([CF_PROG_CC_C_O],
[AC_REQUIRE([AC_PROG_CC])dnl
AC_MSG_CHECKING([whether [$]$1 understands -c and -o together])
AC_CACHE_VAL(cf_cv_prog_$1_c_o,
[
cat > conftest.$ac_ext <<CF_EOF
#include <stdio.h>
int main()
{
	${cf_cv_main_return:-return}(0);
}
CF_EOF
# We do the test twice because some compilers refuse to overwrite an
# existing .o file with -o, though they will create one.
ac_try='[$]$1 -c conftest.$ac_ext -o conftest2.$ac_objext >&AC_FD_CC'
if AC_TRY_EVAL(ac_try) &&
  test -f conftest2.$ac_objext && AC_TRY_EVAL(ac_try);
then
  eval cf_cv_prog_$1_c_o=yes
else
  eval cf_cv_prog_$1_c_o=no
fi
rm -f conftest*
])dnl
if test $cf_cv_prog_$1_c_o = yes; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_CC_U_D version: 1 updated: 2005/07/14 16:59:30
dnl --------------
dnl Check if C (preprocessor) -U and -D options are processed in the order
dnl given rather than by type of option.  Some compilers insist on apply all
dnl of the -U options after all of the -D options.  Others allow mixing them,
dnl and may predefine symbols that conflict with those we define.
AC_DEFUN([CF_PROG_CC_U_D],
[
AC_CACHE_CHECK(if $CC -U and -D options work together,cf_cv_cc_u_d_options,[
	cf_save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="-UU_D_OPTIONS -DU_D_OPTIONS -DD_U_OPTIONS -UD_U_OPTIONS"
	AC_TRY_COMPILE([],[
#ifndef U_D_OPTIONS
make an undefined-error
#endif
#ifdef  D_U_OPTIONS
make a defined-error
#endif
	],[
	cf_cv_cc_u_d_options=yes],[
	cf_cv_cc_u_d_options=no])
	CPPFLAGS="$cf_save_CPPFLAGS"
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_EGREP version: 1 updated: 2006/09/16 11:40:59
dnl -------------
dnl AC_PROG_EGREP was introduced in autoconf 2.53.
dnl This macro adds a check to ensure the script found something.
AC_DEFUN([CF_PROG_EGREP],
[AC_CACHE_CHECK([for egrep], [ac_cv_prog_egrep],
   [if echo a | (grep -E '(a|b)') >/dev/null 2>&1
    then ac_cv_prog_egrep='grep -E'
    else ac_cv_prog_egrep='egrep'
    fi])
 EGREP=$ac_cv_prog_egrep
 AC_SUBST([EGREP])
test -z "$EGREP" && AC_MSG_ERROR(No egrep program found)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_EXT version: 10 updated: 2004/01/03 19:28:18
dnl -----------
dnl Compute $PROG_EXT, used for non-Unix ports, such as OS/2 EMX.
AC_DEFUN([CF_PROG_EXT],
[
AC_REQUIRE([CF_CHECK_CACHE])
case $cf_cv_system_name in
os2*)
    CFLAGS="$CFLAGS -Zmt"
    CPPFLAGS="$CPPFLAGS -D__ST_MT_ERRNO__"
    CXXFLAGS="$CXXFLAGS -Zmt"
    # autoconf's macro sets -Zexe and suffix both, which conflict:w
    LDFLAGS="$LDFLAGS -Zmt -Zcrtdll"
    ac_cv_exeext=.exe
    ;;
esac

AC_EXEEXT
AC_OBJEXT

PROG_EXT="$EXEEXT"
AC_SUBST(PROG_EXT)
test -n "$PROG_EXT" && AC_DEFINE_UNQUOTED(PROG_EXT,"$PROG_EXT")
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_INSTALL version: 5 updated: 2002/12/21 22:46:07
dnl ---------------
dnl Force $INSTALL to be an absolute-path.  Otherwise, edit_man.sh and the
dnl misc/tabset install won't work properly.  Usually this happens only when
dnl using the fallback mkinstalldirs script
AC_DEFUN([CF_PROG_INSTALL],
[AC_PROG_INSTALL
case $INSTALL in
/*)
  ;;
*)
  CF_DIRNAME(cf_dir,$INSTALL)
  test -z "$cf_dir" && cf_dir=.
  INSTALL=`cd $cf_dir && pwd`/`echo $INSTALL | sed -e 's%^.*/%%'`
  ;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_LDCONFIG version: 1 updated: 2003/09/20 17:07:55
dnl ----------------
dnl Check for ldconfig, needed to fixup shared libraries that would be built
dnl and then used in the install.
AC_DEFUN([CF_PROG_LDCONFIG],[
if test "$cross_compiling" = yes ; then
  LDCONFIG=:
else
case "$cf_cv_system_name" in #(vi
freebsd*) #(vi
  test -z "$LDCONFIG" && LDCONFIG="/sbin/ldconfig -R"
  ;;
*) LDPATH=$PATH:/sbin:/usr/sbin
  AC_PATH_PROG(LDCONFIG,ldconfig,,$LDPATH)
  ;;
esac
fi
AC_SUBST(LDCONFIG)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_LINT version: 1 updated: 2006/09/16 11:40:59
dnl ------------
AC_DEFUN([CF_PROG_LINT],
[
AC_CHECK_PROGS(LINT, tdlint lint alint)
AC_SUBST(LINT_OPTS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REGEX version: 3 updated: 1997/11/01 14:26:01
dnl --------
dnl Attempt to determine if we've got one of the flavors of regular-expression
dnl code that we can support.
AC_DEFUN([CF_REGEX],
[
AC_MSG_CHECKING([for regular-expression headers])
AC_CACHE_VAL(cf_cv_regex,[
AC_TRY_LINK([#include <sys/types.h>
#include <regex.h>],[
	regex_t *p;
	int x = regcomp(p, "", 0);
	int y = regexec(p, "", 0, 0, 0);
	regfree(p);
	],[cf_cv_regex="regex.h"],[
	AC_TRY_LINK([#include <regexp.h>],[
		char *p = compile("", "", "", 0);
		int x = step("", "");
	],[cf_cv_regex="regexp.h"],[
		cf_save_LIBS="$LIBS"
		LIBS="-lgen $LIBS"
		AC_TRY_LINK([#include <regexpr.h>],[
			char *p = compile("", "", "");
			int x = step("", "");
		],[cf_cv_regex="regexpr.h"],[LIBS="$cf_save_LIBS"])])])
])
AC_MSG_RESULT($cf_cv_regex)
case $cf_cv_regex in
	regex.h)   AC_DEFINE(HAVE_REGEX_H_FUNCS) ;;
	regexp.h)  AC_DEFINE(HAVE_REGEXP_H_FUNCS) ;;
	regexpr.h) AC_DEFINE(HAVE_REGEXPR_H_FUNCS) ;;
esac
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_DEFINE version: 2 updated: 2005/07/09 16:12:18
dnl ----------------
dnl Remove all -U and -D options that refer to the given symbol from a list
dnl of C compiler options.  This works around the problem that not all
dnl compilers process -U and -D options from left-to-right, so a -U option
dnl cannot be used to cancel the effect of a preceding -D option.
dnl
dnl $1 = target (which could be the same as the source variable)
dnl $2 = source (including '$')
dnl $3 = symbol to remove
define([CF_REMOVE_DEFINE],
[
# remove $3 symbol from $2
$1=`echo "$2" | \
	sed	-e 's/-[[UD]]$3\(=[[^ 	]]*\)\?[[ 	]]/ /g' \
		-e 's/-[[UD]]$3\(=[[^ 	]]*\)\?[$]//g'`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_LIB version: 1 updated: 2007/02/17 14:11:52
dnl -------------
dnl Remove the given library from the symbol
dnl
dnl $1 = target (which could be the same as the source variable)
dnl $2 = source (including '$')
dnl $3 = library to remove
define([CF_REMOVE_LIB],
[
# remove $3 library from $2
$1=`echo "$2" | sed -e 's/-l$3[[ 	]]//g' -e 's/-l$3[$]//'`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_RPATH_HACK version: 4 updated: 2008/09/13 12:53:26
dnl -------------
AC_DEFUN([CF_RPATH_HACK],
[
AC_REQUIRE([CF_SHARED_OPTS])
AC_MSG_CHECKING(for updated LDFLAGS)
if test -n "$LDFLAGS" ; then
AC_MSG_RESULT(maybe)
CF_VERBOSE(...checking LDFLAGS $LDFLAGS)
CF_VERBOSE(...checking EXTRA_LDFLAGS $EXTRA_LDFLAGS)
case "$EXTRA_LDFLAGS" in #(vi
-Wl,-rpath,*) #(vi
	cf_rpath_hack="-Wl,-rpath,"
	;;
-R\ *)
	cf_rpath_hack="-R "
	;;
-R*)
	cf_rpath_hack="-R"
	;;
*)
	cf_rpath_hack=
	;;
esac
if test -n "$cf_rpath_hack" ; then
	cf_rpath_dst=
	for cf_rpath_src in $LDFLAGS
	do
		CF_VERBOSE(Filtering $cf_rpath_src)
		case $cf_rpath_src in #(vi
		-L*) #(vi
			if test "$cf_rpath_hack" = "-R " ; then
				cf_rpath_tmp=`echo "$cf_rpath_src" |sed -e 's%-L%-R %'`
			else
				cf_rpath_tmp=`echo "$cf_rpath_src" |sed -e s%-L%$cf_rpath_hack%`
			fi
			CF_VERBOSE(...Filter $cf_rpath_tmp)
			EXTRA_LDFLAGS="$cf_rpath_tmp $EXTRA_LDFLAGS"
			;;
		esac
		cf_rpath_dst="$cf_rpath_dst $cf_rpath_src"
	done
	LDFLAGS=$cf_rpath_dst
	CF_VERBOSE(...checked LDFLAGS $LDFLAGS)
	CF_VERBOSE(...checked EXTRA_LDFLAGS $EXTRA_LDFLAGS)
fi
else
AC_MSG_RESULT(no)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SHARED_OPTS version: 53 updated: 2008/10/25 18:14:20
dnl --------------
dnl --------------
dnl Attempt to determine the appropriate CC/LD options for creating a shared
dnl library.
dnl
dnl Note: ${LOCAL_LDFLAGS} is used to link executables that will run within the
dnl build-tree, i.e., by making use of the libraries that are compiled in ../lib
dnl We avoid compiling-in a ../lib path for the shared library since that can
dnl lead to unexpected results at runtime.
dnl ${LOCAL_LDFLAGS2} has the same intention but assumes that the shared libraries
dnl are compiled in ../../lib
dnl
dnl The variable 'cf_cv_do_symlinks' is used to control whether we configure
dnl to install symbolic links to the rel/abi versions of shared libraries.
dnl
dnl The variable 'cf_cv_shlib_version' controls whether we use the rel or abi
dnl version when making symbolic links.
dnl
dnl The variable 'cf_cv_shlib_version_infix' controls whether shared library
dnl version numbers are infix (ex: libncurses.<ver>.dylib) or postfix
dnl (ex: libncurses.so.<ver>).
dnl
dnl Some loaders leave 'so_locations' lying around.  It's nice to clean up.
AC_DEFUN([CF_SHARED_OPTS],
[
	AC_REQUIRE([CF_SUBST_NCURSES_VERSION])
	LOCAL_LDFLAGS=
	LOCAL_LDFLAGS2=
	LD_SHARED_OPTS=
	INSTALL_LIB="-m 644"

	cf_cv_do_symlinks=no

	AC_MSG_CHECKING(if release/abi version should be used for shared libs)
	AC_ARG_WITH(shlib-version,
	[  --with-shlib-version=X  Specify rel or abi version for shared libs],
	[test -z "$withval" && withval=auto
	case $withval in #(vi
	yes) #(vi
		cf_cv_shlib_version=auto
		;;
	rel|abi|auto|no) #(vi
		cf_cv_shlib_version=$withval
		;;
	*)
		AC_MSG_ERROR([option value must be one of: rel, abi, auto or no])
		;;
	esac
	],[cf_cv_shlib_version=auto])
	AC_MSG_RESULT($cf_cv_shlib_version)

	cf_cv_rm_so_locs=no

	# Some less-capable ports of gcc support only -fpic
	CC_SHARED_OPTS=
	if test "$GCC" = yes
	then
		AC_MSG_CHECKING(which $CC option to use)
		cf_save_CFLAGS="$CFLAGS"
		for CC_SHARED_OPTS in -fPIC -fpic ''
		do
			CFLAGS="$cf_save_CFLAGS $CC_SHARED_OPTS"
			AC_TRY_COMPILE([#include <stdio.h>],[int x = 1],[break],[])
		done
		AC_MSG_RESULT($CC_SHARED_OPTS)
		CFLAGS="$cf_save_CFLAGS"
	fi

	cf_cv_shlib_version_infix=no

	case $cf_cv_system_name in
	beos*)
		MK_SHARED_LIB='${CC} ${CFLAGS} -o $[@] -Xlinker -soname=`basename $[@]` -nostart -e 0'
		;;
	cygwin*)
		CC_SHARED_OPTS=
		MK_SHARED_LIB='sh ../mk_shared_lib.sh [$]@ [$]{CC} [$]{CFLAGS}'
		cf_cv_shlib_version=cygdll
		cf_cv_shlib_version_infix=cygdll
		cat >mk_shared_lib.sh <<-CF_EOF
		#!/bin/sh
		SHARED_LIB=\[$]1
		IMPORT_LIB=\`echo "\[$]1" | sed -e 's/cyg/lib/' -e 's/[[0-9]]*\.dll[$]/.dll.a/'\`
		shift
		cat <<-EOF
		Linking shared library
		** SHARED_LIB \[$]SHARED_LIB
		** IMPORT_LIB \[$]IMPORT_LIB
EOF
		exec \[$]* -shared -Wl,--out-implib=../lib/\[$]{IMPORT_LIB} -Wl,--export-all-symbols -o ../lib/\[$]{SHARED_LIB}
CF_EOF
		chmod +x mk_shared_lib.sh 
		;;
	darwin*)
		EXTRA_CFLAGS="-no-cpp-precomp"
		CC_SHARED_OPTS="-dynamic"
		MK_SHARED_LIB='${CC} ${CFLAGS} -dynamiclib -install_name ${libdir}/`basename $[@]` -compatibility_version ${ABI_VERSION} -current_version ${ABI_VERSION} -o $[@]'
		test "$cf_cv_shlib_version" = auto && cf_cv_shlib_version=abi
		cf_cv_shlib_version_infix=yes
		AC_CACHE_CHECK([if ld -search_paths_first works], cf_cv_ldflags_search_paths_first, [
			cf_save_LDFLAGS=$LDFLAGS
			LDFLAGS="$LDFLAGS -Wl,-search_paths_first"
			AC_TRY_LINK(, [int i;], cf_cv_ldflags_search_paths_first=yes, cf_cv_ldflags_search_paths_first=no)
				LDFLAGS=$cf_save_LDFLAGS])
		if test $cf_cv_ldflags_search_paths_first = yes; then
			LDFLAGS="$LDFLAGS -Wl,-search_paths_first"
		fi
		;;
	hpux*)
		# (tested with gcc 2.7.2 -- I don't have c89)
		if test "$GCC" = yes; then
			LD_SHARED_OPTS='-Xlinker +b -Xlinker ${libdir}'
		else
			CC_SHARED_OPTS='+Z'
			LD_SHARED_OPTS='-Wl,+b,${libdir}'
		fi
		MK_SHARED_LIB='${LD} +b ${libdir} -b -o $[@]'
		# HP-UX shared libraries must be executable, and should be
		# readonly to exploit a quirk in the memory manager.
		INSTALL_LIB="-m 555"
		;;
	irix*)
		if test "$cf_cv_ld_rpath" = yes ; then
			if test "$GCC" = yes; then
				cf_ld_rpath_opt="-Wl,-rpath,"
				EXTRA_LDFLAGS="-Wl,-rpath,\${libdir} $EXTRA_LDFLAGS"
			else
				cf_ld_rpath_opt="-rpath "
				EXTRA_LDFLAGS="-rpath \${libdir} $EXTRA_LDFLAGS"
			fi
		fi
		# tested with IRIX 5.2 and 'cc'.
		if test "$GCC" != yes; then
			CC_SHARED_OPTS='-KPIC'
			MK_SHARED_LIB='${CC} -shared -rdata_shared -soname `basename $[@]` -o $[@]'
		else
			MK_SHARED_LIB='${CC} -shared -Wl,-soname,`basename $[@]` -o $[@]'
		fi
		cf_cv_rm_so_locs=yes
		;;
	linux*|gnu*|k*bsd*-gnu)
		if test "$DFT_LWR_MODEL" = "shared" ; then
			LOCAL_LDFLAGS="-Wl,-rpath,\$(LOCAL_LIBDIR)"
			LOCAL_LDFLAGS2="$LOCAL_LDFLAGS"
		fi
		if test "$cf_cv_ld_rpath" = yes ; then
			cf_ld_rpath_opt="-Wl,-rpath,"
			EXTRA_LDFLAGS="-Wl,-rpath,\${libdir} $EXTRA_LDFLAGS"
		fi
		CF_SHARED_SONAME
		MK_SHARED_LIB='${CC} ${CFLAGS} -shared -Wl,-soname,'$cf_cv_shared_soname',-stats,-lc -o $[@]'
		;;
	openbsd[[2-9]].*)
		if test "$DFT_LWR_MODEL" = "shared" ; then
			LOCAL_LDFLAGS="-Wl,-rpath,\$(LOCAL_LIBDIR)"
			LOCAL_LDFLAGS2="$LOCAL_LDFLAGS"
		fi
		if test "$cf_cv_ld_rpath" = yes ; then
			cf_ld_rpath_opt="-Wl,-rpath,"
			EXTRA_LDFLAGS="-Wl,-rpath,\${libdir} $EXTRA_LDFLAGS"
		fi
		CC_SHARED_OPTS="$CC_SHARED_OPTS -DPIC"
		CF_SHARED_SONAME
		MK_SHARED_LIB='${CC} ${CFLAGS} -Wl,-Bshareable,-soname,'$cf_cv_shared_soname',-stats,-lc -o $[@]'
		;;
	nto-qnx*|openbsd*|freebsd[[12]].*)
		CC_SHARED_OPTS="$CC_SHARED_OPTS -DPIC"
		MK_SHARED_LIB='${LD} -Bshareable -o $[@]'
		test "$cf_cv_shlib_version" = auto && cf_cv_shlib_version=rel
		;;
	freebsd*)
		CC_SHARED_OPTS="$CC_SHARED_OPTS -DPIC"
		if test "$DFT_LWR_MODEL" = "shared" && test "$cf_cv_ld_rpath" = yes ; then
			LOCAL_LDFLAGS="-rpath \$(LOCAL_LIBDIR)"
			LOCAL_LDFLAGS2="-rpath \${libdir} $LOCAL_LDFLAGS"
			cf_ld_rpath_opt="-rpath "
			EXTRA_LDFLAGS="-rpath \${libdir} $EXTRA_LDFLAGS"
		fi
		CF_SHARED_SONAME
		MK_SHARED_LIB='${LD} -Bshareable -soname=`basename $[@]` -o $[@]'
		;;
	netbsd*)
		CC_SHARED_OPTS="$CC_SHARED_OPTS -DPIC"
		test "$cf_cv_ld_rpath" = yes && cf_ld_rpath_opt="-Wl,-rpath,"
		if test "$DFT_LWR_MODEL" = "shared" && test "$cf_cv_ld_rpath" = yes ; then
			LOCAL_LDFLAGS="-Wl,-rpath,\$(LOCAL_LIBDIR)"
			LOCAL_LDFLAGS2="$LOCAL_LDFLAGS"
			EXTRA_LDFLAGS="-Wl,-rpath,\${libdir} $EXTRA_LDFLAGS"
			if test "$cf_cv_shlib_version" = auto; then
			if test -f /usr/libexec/ld.elf_so; then
				cf_cv_shlib_version=abi
			else
				cf_cv_shlib_version=rel
			fi
			fi
			CF_SHARED_SONAME
			MK_SHARED_LIB='${CC} ${CFLAGS} -shared -Wl,-soname,'$cf_cv_shared_soname' -o $[@]'
		else
			MK_SHARED_LIB='${LD} -Bshareable -o $[@]'
		fi
		;;
	osf*|mls+*)
		# tested with OSF/1 V3.2 and 'cc'
		# tested with OSF/1 V3.2 and gcc 2.6.3 (but the c++ demo didn't
		# link with shared libs).
		MK_SHARED_LIB='${LD} -set_version ${REL_VERSION}:${ABI_VERSION} -expect_unresolved "*" -shared -soname `basename $[@]`'
		case $host_os in
		osf4*)
			MK_SHARED_LIB="${MK_SHARED_LIB} -msym"
			;;
		esac
		MK_SHARED_LIB="${MK_SHARED_LIB}"' -o $[@]'
		if test "$DFT_LWR_MODEL" = "shared" ; then
			LOCAL_LDFLAGS="-Wl,-rpath,\$(LOCAL_LIBDIR)"
			LOCAL_LDFLAGS2="$LOCAL_LDFLAGS"
		fi
		if test "$cf_cv_ld_rpath" = yes ; then
			cf_ld_rpath_opt="-rpath"
			# EXTRA_LDFLAGS="$LOCAL_LDFLAGS $EXTRA_LDFLAGS"
		fi
		cf_cv_rm_so_locs=yes
		;;
	sco3.2v5*)  # (also uw2* and UW7) hops 13-Apr-98
		# tested with osr5.0.5
		if test "$GCC" != yes; then
			CC_SHARED_OPTS='-belf -KPIC'
		fi
		MK_SHARED_LIB='${LD} -dy -G -h `basename $[@] .${REL_VERSION}`.${ABI_VERSION} -o [$]@'
		if test "$cf_cv_ld_rpath" = yes ; then
			# only way is to set LD_RUN_PATH but no switch for it
			RUN_PATH=$libdir
		fi
		test "$cf_cv_shlib_version" = auto && cf_cv_shlib_version=rel
		LINK_PROGS='LD_RUN_PATH=${libdir}'
		LINK_TESTS='Pwd=`pwd`;LD_RUN_PATH=`dirname $${Pwd}`/lib'
		;;
	sunos4*)
		# tested with SunOS 4.1.1 and gcc 2.7.0
		if test "$GCC" != yes; then
			CC_SHARED_OPTS='-KPIC'
		fi
		MK_SHARED_LIB='${LD} -assert pure-text -o $[@]'
		test "$cf_cv_shlib_version" = auto && cf_cv_shlib_version=rel
		;;
	solaris2*)
		# tested with SunOS 5.5.1 (solaris 2.5.1) and gcc 2.7.2
		# tested with SunOS 5.10 (solaris 10) and gcc 3.4.3
		if test "$DFT_LWR_MODEL" = "shared" ; then
			LOCAL_LDFLAGS="-R \$(LOCAL_LIBDIR):\${libdir}"
			LOCAL_LDFLAGS2="$LOCAL_LDFLAGS"
		fi
		if test "$cf_cv_ld_rpath" = yes ; then
			cf_ld_rpath_opt="-R"
			EXTRA_LDFLAGS="$LOCAL_LDFLAGS $EXTRA_LDFLAGS"
		fi
		CF_SHARED_SONAME
		if test "$GCC" != yes; then
			CC_SHARED_OPTS='-xcode=pic32'
			MK_SHARED_LIB='${CC} -dy -G -h '$cf_cv_shared_soname' -o $[@]'
		else
			MK_SHARED_LIB='${CC} -shared -dy -G -h '$cf_cv_shared_soname' -o $[@]'
		fi
		;;
	sysv5uw7*|unix_sv*)
		# tested with UnixWare 7.1.0 (gcc 2.95.2 and cc)
		if test "$GCC" != yes; then
			CC_SHARED_OPTS='-KPIC'
		fi
		MK_SHARED_LIB='${LD} -d y -G -o [$]@'
		;;
	*)
		CC_SHARED_OPTS='unknown'
		MK_SHARED_LIB='echo unknown'
		;;
	esac

	# This works if the last tokens in $MK_SHARED_LIB are the -o target.
	case "$cf_cv_shlib_version" in #(vi
	rel|abi)
		case "$MK_SHARED_LIB" in #(vi
		*'-o $[@]')
			test "$cf_cv_do_symlinks" = no && cf_cv_do_symlinks=yes
			;;
		*)
			AC_MSG_WARN(ignored --with-shlib-version)
			;;
		esac
		;;
	esac

	if test -n "$cf_ld_rpath_opt" ; then
		AC_MSG_CHECKING(if we need a space after rpath option)
		cf_save_LIBS="$LIBS"
		LIBS="$LIBS ${cf_ld_rpath_opt}$libdir"
		AC_TRY_LINK(, , cf_rpath_space=no, cf_rpath_space=yes)
		LIBS="$cf_save_LIBS"
		AC_MSG_RESULT($cf_rpath_space)
		test "$cf_rpath_space" = yes && cf_ld_rpath_opt="$cf_ld_rpath_opt "
		MK_SHARED_LIB="$MK_SHARED_LIB $cf_ld_rpath_opt\${libdir}"
	fi

	AC_SUBST(CC_SHARED_OPTS)
	AC_SUBST(LD_SHARED_OPTS)
	AC_SUBST(MK_SHARED_LIB)
	AC_SUBST(LINK_PROGS)
	AC_SUBST(LINK_TESTS)
	AC_SUBST(EXTRA_LDFLAGS)
	AC_SUBST(LOCAL_LDFLAGS)
	AC_SUBST(LOCAL_LDFLAGS2)
	AC_SUBST(INSTALL_LIB)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SHARED_SONAME version: 3 updated: 2008/09/08 18:34:43
dnl ----------------
dnl utility macro for CF_SHARED_OPTS, constructs "$cf_cv_shared_soname" for
dnl substitution into MK_SHARED_LIB string for the "-soname" (or similar)
dnl option.
dnl
dnl $1 is the default that should be used for "$cf_cv_shlib_version".
dnl If missing, use "rel".
define([CF_SHARED_SONAME],
[
	test "$cf_cv_shlib_version" = auto && cf_cv_shlib_version=ifelse($1,,rel,$1)
	if test "$cf_cv_shlib_version" = rel; then
		cf_cv_shared_soname='`basename $[@] .${REL_VERSION}`.${ABI_VERSION}'
	else
		cf_cv_shared_soname='`basename $[@]`'
	fi
])
dnl ---------------------------------------------------------------------------
dnl CF_SIGWINCH version: 1 updated: 2006/04/02 16:41:09
dnl -----------
dnl Use this macro after CF_XOPEN_SOURCE, but do not require it (not all
dnl programs need this test).
dnl
dnl This is really a MacOS X 10.4.3 workaround.  Defining _POSIX_C_SOURCE
dnl forces SIGWINCH to be undefined (breaks xterm, ncurses).  Oddly, the struct
dnl winsize declaration is left alone - we may revisit this if Apple choose to
dnl break that part of the interface as well.
AC_DEFUN([CF_SIGWINCH],
[
AC_CACHE_CHECK(if SIGWINCH is defined,cf_cv_define_sigwinch,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/signal.h>
],[int x = SIGWINCH],
	[cf_cv_define_sigwinch=yes],
	[AC_TRY_COMPILE([
#undef _XOPEN_SOURCE
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/signal.h>
],[int x = SIGWINCH],
	[cf_cv_define_sigwinch=maybe],
	[cf_cv_define_sigwinch=no])
])
])

if test "$cf_cv_define_sigwinch" = maybe ; then
AC_CACHE_CHECK(for actual SIGWINCH definition,cf_cv_fixup_sigwinch,[
cf_cv_fixup_sigwinch=unknown
cf_sigwinch=32
while test $cf_sigwinch != 1
do
	AC_TRY_COMPILE([
#undef _XOPEN_SOURCE
#undef _POSIX_SOURCE
#undef _POSIX_C_SOURCE
#include <sys/types.h>
#include <sys/signal.h>
],[
#if SIGWINCH != $cf_sigwinch
make an error
#endif
int x = SIGWINCH],
	[cf_cv_fixup_sigwinch=$cf_sigwinch
	 break])

cf_sigwinch=`expr $cf_sigwinch - 1`
done
])

	if test "$cf_cv_fixup_sigwinch" != unknown ; then
		CPPFLAGS="$CPPFLAGS -DSIGWINCH=$cf_cv_fixup_sigwinch"
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIG_ATOMIC_T version: 2 updated: 2005/09/18 17:27:12
dnl ---------------
dnl signal handler, but there are some gcc depedencies in that recommendation.
dnl Try anyway.
AC_DEFUN([CF_SIG_ATOMIC_T],
[
AC_MSG_CHECKING(for signal global datatype)
AC_CACHE_VAL(cf_cv_sig_atomic_t,[
	for cf_type in \
		"volatile sig_atomic_t" \
		"sig_atomic_t" \
		"int"
	do
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

extern $cf_type x;
$cf_type x;
static void handler(int sig)
{
	x = 5;
}],
		[signal(SIGINT, handler);
		 x = 1],
		[cf_cv_sig_atomic_t=$cf_type],
		[cf_cv_sig_atomic_t=no])
		test "$cf_cv_sig_atomic_t" != no && break
	done
	])
AC_MSG_RESULT($cf_cv_sig_atomic_t)
test "$cf_cv_sig_atomic_t" != no && AC_DEFINE_UNQUOTED(SIG_ATOMIC_T, $cf_cv_sig_atomic_t)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SIZECHANGE version: 8 updated: 2000/11/04 12:22:16
dnl -------------
dnl Check for definitions & structures needed for window size-changing
dnl FIXME: check that this works with "snake" (HP-UX 10.x)
AC_DEFUN([CF_SIZECHANGE],
[
AC_REQUIRE([CF_STRUCT_TERMIOS])
AC_CACHE_CHECK(declaration of size-change, cf_cv_sizechange,[
    cf_cv_sizechange=unknown
    cf_save_CPPFLAGS="$CPPFLAGS"

for cf_opts in "" "NEED_PTEM_H"
do

    CPPFLAGS="$cf_save_CPPFLAGS"
    test -n "$cf_opts" && CPPFLAGS="$CPPFLAGS -D$cf_opts"
    AC_TRY_COMPILE([#include <sys/types.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#else
#ifdef HAVE_TERMIO_H
#include <termio.h>
#endif
#endif
#ifdef NEED_PTEM_H
/* This is a workaround for SCO:  they neglected to define struct winsize in
 * termios.h -- it's only in termio.h and ptem.h
 */
#include        <sys/stream.h>
#include        <sys/ptem.h>
#endif
#if !defined(sun) || !defined(HAVE_TERMIOS_H)
#include <sys/ioctl.h>
#endif
],[
#ifdef TIOCGSIZE
	struct ttysize win;	/* FIXME: what system is this? */
	int y = win.ts_lines;
	int x = win.ts_cols;
#else
#ifdef TIOCGWINSZ
	struct winsize win;
	int y = win.ws_row;
	int x = win.ws_col;
#else
	no TIOCGSIZE or TIOCGWINSZ
#endif /* TIOCGWINSZ */
#endif /* TIOCGSIZE */
	],
	[cf_cv_sizechange=yes],
	[cf_cv_sizechange=no])

	CPPFLAGS="$cf_save_CPPFLAGS"
	if test "$cf_cv_sizechange" = yes ; then
		echo "size-change succeeded ($cf_opts)" >&AC_FD_CC
		test -n "$cf_opts" && cf_cv_sizechange="$cf_opts"
		break
	fi
done
])
if test "$cf_cv_sizechange" != no ; then
	AC_DEFINE(HAVE_SIZECHANGE)
	case $cf_cv_sizechange in #(vi
	NEED*)
		AC_DEFINE_UNQUOTED($cf_cv_sizechange )
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SRC_MODULES version: 18 updated: 2005/05/28 12:58:54
dnl --------------
dnl For each parameter, test if the source-directory exists, and if it contains
dnl a 'modules' file.  If so, add to the list $cf_cv_src_modules which we'll
dnl use in CF_LIB_RULES.
dnl
dnl This uses the configured value to make the lists SRC_SUBDIRS and
dnl SUB_MAKEFILES which are used in the makefile-generation scheme.
AC_DEFUN([CF_SRC_MODULES],
[
AC_MSG_CHECKING(for src modules)

# dependencies and linker-arguments for test-programs
TEST_DEPS="${LIB_DIR}/${LIB_PREFIX}${LIB_NAME}${DFT_DEP_SUFFIX} $TEST_DEPS"
TEST_DEP2="${LIB_2ND}/${LIB_PREFIX}${LIB_NAME}${DFT_DEP_SUFFIX} $TEST_DEP2"
if test "$DFT_LWR_MODEL" = "libtool"; then
	TEST_ARGS="${TEST_DEPS}"
	TEST_ARG2="${TEST_DEP2}"
else
	TEST_ARGS="-l${LIB_NAME}${DFT_ARG_SUFFIX} $TEST_ARGS"
	TEST_ARG2="-l${LIB_NAME}${DFT_ARG_SUFFIX} $TEST_ARG2"
fi

cf_cv_src_modules=
for cf_dir in $1
do
	if test -f $srcdir/$cf_dir/modules; then

		# We may/may not have tack in the distribution, though the
		# makefile is.
		if test $cf_dir = tack ; then
			if test ! -f $srcdir/${cf_dir}/${cf_dir}.h; then
				continue
			fi
		fi

		if test -z "$cf_cv_src_modules"; then
			cf_cv_src_modules=$cf_dir
		else
			cf_cv_src_modules="$cf_cv_src_modules $cf_dir"
		fi

		# Make the ncurses_cfg.h file record the library interface files as
		# well.  These are header files that are the same name as their
		# directory.  Ncurses is the only library that does not follow
		# that pattern.
		if test $cf_dir = tack ; then
			continue
		elif test -f $srcdir/${cf_dir}/${cf_dir}.h; then
			CF_UPPER(cf_have_include,$cf_dir)
			AC_DEFINE_UNQUOTED(HAVE_${cf_have_include}_H)
			AC_DEFINE_UNQUOTED(HAVE_LIB${cf_have_include})
			TEST_DEPS="${LIB_DIR}/${LIB_PREFIX}${cf_dir}${DFT_DEP_SUFFIX} $TEST_DEPS"
			TEST_DEP2="${LIB_2ND}/${LIB_PREFIX}${cf_dir}${DFT_DEP_SUFFIX} $TEST_DEP2"
			if test "$DFT_LWR_MODEL" = "libtool"; then
				TEST_ARGS="${TEST_DEPS}"
				TEST_ARG2="${TEST_DEP2}"
			else
				TEST_ARGS="-l${cf_dir}${DFT_ARG_SUFFIX} $TEST_ARGS"
				TEST_ARG2="-l${cf_dir}${DFT_ARG_SUFFIX} $TEST_ARG2"
			fi
		fi
	fi
done
AC_MSG_RESULT($cf_cv_src_modules)

TEST_ARGS="-L${LIB_DIR} $TEST_ARGS"
TEST_ARG2="-L${LIB_2ND} $TEST_ARG2"

AC_SUBST(TEST_ARGS)
AC_SUBST(TEST_DEPS)

AC_SUBST(TEST_ARG2)
AC_SUBST(TEST_DEP2)

SRC_SUBDIRS="man include"
for cf_dir in $cf_cv_src_modules
do
	SRC_SUBDIRS="$SRC_SUBDIRS $cf_dir"
done
SRC_SUBDIRS="$SRC_SUBDIRS test"
test -z "$MAKE_TERMINFO" && SRC_SUBDIRS="$SRC_SUBDIRS misc"
test "$cf_with_cxx_binding" != no && SRC_SUBDIRS="$SRC_SUBDIRS c++"

ADA_SUBDIRS=
if test "$cf_cv_prog_gnat_correct" = yes && test -f $srcdir/Ada95/Makefile.in; then
   SRC_SUBDIRS="$SRC_SUBDIRS Ada95"
   ADA_SUBDIRS="gen src samples"
fi

SUB_MAKEFILES=
for cf_dir in $SRC_SUBDIRS
do
	SUB_MAKEFILES="$SUB_MAKEFILES $cf_dir/Makefile"
done

if test -n "$ADA_SUBDIRS"; then
   for cf_dir in $ADA_SUBDIRS
   do
      SUB_MAKEFILES="$SUB_MAKEFILES Ada95/$cf_dir/Makefile"
   done
   AC_SUBST(ADA_SUBDIRS)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STDCPP_LIBRARY version: 5 updated: 2000/08/12 23:18:52
dnl -----------------
dnl Check for -lstdc++, which is GNU's standard C++ library.
AC_DEFUN([CF_STDCPP_LIBRARY],
[
if test -n "$GXX" ; then
case $cf_cv_system_name in #(vi
os2*) #(vi
	cf_stdcpp_libname=stdcpp
	;;
*)
	cf_stdcpp_libname=stdc++
	;;
esac
AC_CACHE_CHECK(for library $cf_stdcpp_libname,cf_cv_libstdcpp,[
	cf_save="$LIBS"
	LIBS="$LIBS -l$cf_stdcpp_libname"
AC_TRY_LINK([
#include <strstream.h>],[
char buf[80];
strstreambuf foo(buf, sizeof(buf))
],
	[cf_cv_libstdcpp=yes],
	[cf_cv_libstdcpp=no])
	LIBS="$cf_save"
])
test "$cf_cv_libstdcpp" = yes && CXXLIBS="$CXXLIBS -l$cf_stdcpp_libname"
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRIP_G_OPT version: 3 updated: 2002/12/21 19:25:52
dnl --------------
dnl	Remove "-g" option from the compiler options
AC_DEFUN([CF_STRIP_G_OPT],
[$1=`echo ${$1} | sed -e 's%-g %%' -e 's%-g$%%'`])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRUCT_SIGACTION version: 3 updated: 2000/08/12 23:18:52
dnl -------------------
dnl Check if we need _POSIX_SOURCE defined to use struct sigaction.  We'll only
dnl do this if we've found the sigaction function.
dnl
dnl If needed, define SVR4_ACTION.
AC_DEFUN([CF_STRUCT_SIGACTION],[
if test "$ac_cv_func_sigaction" = yes; then
AC_MSG_CHECKING(whether sigaction needs _POSIX_SOURCE)
AC_TRY_COMPILE([
#include <sys/types.h>
#include <signal.h>],
	[struct sigaction act],
	[sigact_bad=no],
	[
AC_TRY_COMPILE([
#define _POSIX_SOURCE
#include <sys/types.h>
#include <signal.h>],
	[struct sigaction act],
	[sigact_bad=yes
	 AC_DEFINE(SVR4_ACTION)],
	 [sigact_bad=unknown])])
AC_MSG_RESULT($sigact_bad)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_STRUCT_TERMIOS version: 5 updated: 2000/11/04 12:22:46
dnl -----------------
dnl Some machines require _POSIX_SOURCE to completely define struct termios.
dnl If so, define SVR4_TERMIO
AC_DEFUN([CF_STRUCT_TERMIOS],[
AC_CHECK_HEADERS( \
termio.h \
termios.h \
unistd.h \
)
if test "$ISC" = yes ; then
	AC_CHECK_HEADERS( sys/termio.h )
fi
if test "$ac_cv_header_termios_h" = yes ; then
	case "$CFLAGS $CPPFLAGS" in
	*-D_POSIX_SOURCE*)
		termios_bad=dunno ;;
	*)	termios_bad=maybe ;;
	esac
	if test "$termios_bad" = maybe ; then
	AC_MSG_CHECKING(whether termios.h needs _POSIX_SOURCE)
	AC_TRY_COMPILE([#include <termios.h>],
		[struct termios foo; int x = foo.c_iflag],
		termios_bad=no, [
		AC_TRY_COMPILE([
#define _POSIX_SOURCE
#include <termios.h>],
			[struct termios foo; int x = foo.c_iflag],
			termios_bad=unknown,
			termios_bad=yes AC_DEFINE(SVR4_TERMIO))
			])
	AC_MSG_RESULT($termios_bad)
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SUBDIR_PATH version: 5 updated: 2007/07/29 09:55:12
dnl --------------
dnl Construct a search-list for a nonstandard header/lib-file
dnl	$1 = the variable to return as result
dnl	$2 = the package name
dnl	$3 = the subdirectory, e.g., bin, include or lib
AC_DEFUN([CF_SUBDIR_PATH],
[$1=""

CF_ADD_SUBDIR_PATH($1,$2,$3,/usr,$prefix)
CF_ADD_SUBDIR_PATH($1,$2,$3,$prefix,NONE)
CF_ADD_SUBDIR_PATH($1,$2,$3,/usr/local,$prefix)
CF_ADD_SUBDIR_PATH($1,$2,$3,/opt,$prefix)
CF_ADD_SUBDIR_PATH($1,$2,$3,[$]HOME,$prefix)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SUBST version: 4 updated: 2006/06/17 12:33:03
dnl --------
dnl	Shorthand macro for substituting things that the user may override
dnl	with an environment variable.
dnl
dnl	$1 = long/descriptive name
dnl	$2 = environment variable
dnl	$3 = default value
AC_DEFUN([CF_SUBST],
[AC_CACHE_VAL(cf_cv_subst_$2,[
AC_MSG_CHECKING(for $1 (symbol $2))
CF_SUBST_IF([-z "[$]$2"], [$2], [$3])
cf_cv_subst_$2=[$]$2
AC_MSG_RESULT([$]$2)
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SUBST_IF version: 2 updated: 2006/06/17 12:33:03
dnl -----------
dnl	Shorthand macro for substituting things that the user may override
dnl	with an environment variable.
dnl
dnl	$1 = condition to pass to "test"
dnl	$2 = environment variable
dnl	$3 = value if the test succeeds
dnl	$4 = value if the test fails
AC_DEFUN([CF_SUBST_IF],
[
if test $1 ; then
	$2=$3
ifelse($4,,,[else
	$2=$4])
fi
AC_SUBST($2)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SUBST_NCURSES_VERSION version: 8 updated: 2006/09/16 11:40:59
dnl ------------------------
dnl Get the version-number for use in shared-library naming, etc.
AC_DEFUN([CF_SUBST_NCURSES_VERSION],
[
AC_REQUIRE([CF_PROG_EGREP])
NCURSES_MAJOR="`$ac_cv_prog_egrep '^NCURSES_MAJOR[[ 	]]*=' $srcdir/dist.mk | sed -e 's/^[[^0-9]]*//'`"
NCURSES_MINOR="`$ac_cv_prog_egrep '^NCURSES_MINOR[[ 	]]*=' $srcdir/dist.mk | sed -e 's/^[[^0-9]]*//'`"
NCURSES_PATCH="`$ac_cv_prog_egrep '^NCURSES_PATCH[[ 	]]*=' $srcdir/dist.mk | sed -e 's/^[[^0-9]]*//'`"
cf_cv_abi_version=${NCURSES_MAJOR}
cf_cv_rel_version=${NCURSES_MAJOR}.${NCURSES_MINOR}
dnl Show the computed version, for logging
cf_cv_timestamp=`date`
AC_MSG_RESULT(Configuring NCURSES $cf_cv_rel_version ABI $cf_cv_abi_version ($cf_cv_timestamp))
dnl We need these values in the generated headers
AC_SUBST(NCURSES_MAJOR)
AC_SUBST(NCURSES_MINOR)
AC_SUBST(NCURSES_PATCH)
dnl We need these values in the generated makefiles
AC_SUBST(cf_cv_rel_version)
AC_SUBST(cf_cv_abi_version)
AC_SUBST(cf_cv_builtin_bool)
AC_SUBST(cf_cv_header_stdbool_h)
AC_SUBST(cf_cv_type_of_bool)dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_SYS_TIME_SELECT version: 4 updated: 2000/10/04 09:18:40
dnl ------------------
dnl Check if we can include <sys/time.h> with <sys/select.h>; this breaks on
dnl older SCO configurations.
AC_DEFUN([CF_SYS_TIME_SELECT],
[
AC_MSG_CHECKING(if sys/time.h works with sys/select.h)
AC_CACHE_VAL(cf_cv_sys_time_select,[
AC_TRY_COMPILE([
#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
],[],[cf_cv_sys_time_select=yes],
     [cf_cv_sys_time_select=no])
     ])
AC_MSG_RESULT($cf_cv_sys_time_select)
test "$cf_cv_sys_time_select" = yes && AC_DEFINE(HAVE_SYS_TIME_SELECT)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TOP_BUILDDIR version: 1 updated: 2006/10/15 16:33:23
dnl ---------------
dnl Define a top_builddir symbol, for applications that need an absolute path.
AC_DEFUN([CF_TOP_BUILDDIR],
[
top_builddir=`pwd`
AC_SUBST(top_builddir)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TYPEOF_CHTYPE version: 8 updated: 2006/12/16 12:33:30
dnl ----------------
dnl Determine the type we should use for chtype (and attr_t, which is treated
dnl as the same thing).  We want around 32 bits, so on most machines want a
dnl long, but on newer 64-bit machines, probably want an int.  If we're using
dnl wide characters, we have to have a type compatible with that, as well.
AC_DEFUN([CF_TYPEOF_CHTYPE],
[
AC_MSG_CHECKING([for type of chtype])
AC_CACHE_VAL(cf_cv_typeof_chtype,[
		AC_TRY_RUN([
#define WANT_BITS 31
#include <stdio.h>
int main()
{
	FILE *fp = fopen("cf_test.out", "w");
	if (fp != 0) {
		char *result = "long";
		if (sizeof(unsigned long) > sizeof(unsigned int)) {
			int n;
			unsigned int x, y;
			for (n = 0; n < WANT_BITS; n++) {
				x = (1 << n);
				y = (x >> n);
				if (y != 1 || x == 0) {
					x = 0;
					break;
				}
			}
			/*
			 * If x is nonzero, an int is big enough for the bits
			 * that we want.
			 */
			result = (x != 0) ? "int" : "long";
		}
		fputs(result, fp);
		fclose(fp);
	}
	${cf_cv_main_return:-return}(0);
}
		],
		[cf_cv_typeof_chtype=`cat cf_test.out`],
		[cf_cv_typeof_chtype=long],
		[cf_cv_typeof_chtype=long])
		rm -f cf_test.out
	])
AC_MSG_RESULT($cf_cv_typeof_chtype)

AC_SUBST(cf_cv_typeof_chtype)
AC_DEFINE_UNQUOTED(TYPEOF_CHTYPE,$cf_cv_typeof_chtype)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_TYPE_SIGACTION version: 3 updated: 2000/08/12 23:18:52
dnl -----------------
dnl
AC_DEFUN([CF_TYPE_SIGACTION],
[
AC_MSG_CHECKING([for type sigaction_t])
AC_CACHE_VAL(cf_cv_type_sigaction,[
	AC_TRY_COMPILE([
#include <signal.h>],
		[sigaction_t x],
		[cf_cv_type_sigaction=yes],
		[cf_cv_type_sigaction=no])])
AC_MSG_RESULT($cf_cv_type_sigaction)
test "$cf_cv_type_sigaction" = yes && AC_DEFINE(HAVE_TYPE_SIGACTION)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UNSIGNED_LITERALS version: 2 updated: 1998/02/07 22:10:16
dnl --------------------
dnl Test if the compiler supports 'U' and 'L' suffixes.  Only old compilers
dnl won't, but they're still there.
AC_DEFUN([CF_UNSIGNED_LITERALS],
[
AC_MSG_CHECKING([if unsigned literals are legal])
AC_CACHE_VAL(cf_cv_unsigned_literals,[
	AC_TRY_COMPILE([],[long x = 1L + 1UL + 1U + 1],
		[cf_cv_unsigned_literals=yes],
		[cf_cv_unsigned_literals=no])
	])
AC_MSG_RESULT($cf_cv_unsigned_literals)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UPPER version: 5 updated: 2001/01/29 23:40:59
dnl --------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
$1=`echo "$2" | sed y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_UTF8_LIB version: 5 updated: 2008/10/17 19:37:52
dnl -----------
dnl Check for multibyte support, and if not found, utf8 compatibility library
AC_DEFUN([CF_UTF8_LIB],
[
AC_CACHE_CHECK(for multibyte character support,cf_cv_utf8_lib,[
	cf_save_LIBS="$LIBS"
	AC_TRY_LINK([
#include <stdlib.h>],[putwc(0,0);],
	[cf_cv_utf8_lib=yes],
	[CF_FIND_LINKAGE([
#include <libutf8.h>],[putwc(0,0);],utf8,
		[cf_cv_utf8_lib=add-on],
		[cf_cv_utf8_lib=no])
])])

# HAVE_LIBUTF8_H is used by ncurses if curses.h is shared between
# ncurses/ncursesw:
if test "$cf_cv_utf8_lib" = "add-on" ; then
	AC_DEFINE(HAVE_LIBUTF8_H)
	CF_ADD_INCDIR($cf_cv_header_path_utf8)
	CF_ADD_LIBDIR($cf_cv_library_path_utf8)
	LIBS="-lutf8 $LIBS"
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_VERBOSE version: 3 updated: 2007/07/29 09:55:12
dnl ----------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
CF_MSG_LOG([$1])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WCHAR_TYPE version: 2 updated: 2004/01/17 19:18:20
dnl -------------
dnl Check if type wide-character type $1 is declared, and if so, which header
dnl file is needed.  The second parameter is used to set a shell variable when
dnl the type is not found.  The first parameter sets a shell variable for the
dnl opposite sense.
AC_DEFUN([CF_WCHAR_TYPE],
[
# This is needed on Tru64 5.0 to declare $1
AC_CACHE_CHECK(if we must include wchar.h to declare $1,cf_cv_$1,[
AC_TRY_COMPILE([
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef HAVE_LIBUTF8_H
#include <libutf8.h>
#endif],
	[$1 state],
	[cf_cv_$1=no],
	[AC_TRY_COMPILE([
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#ifdef HAVE_LIBUTF8_H
#include <libutf8.h>
#endif],
	[$1 value],
	[cf_cv_$1=yes],
	[cf_cv_$1=unknown])])])

if test "$cf_cv_$1" = yes ; then
	AC_DEFINE(NEED_WCHAR_H)
	NEED_WCHAR_H=1
fi

ifelse($2,,,[
# if we do not find $1 in either place, use substitution to provide a fallback.
if test "$cf_cv_$1" = unknown ; then
	$2=1
fi
])
ifelse($3,,,[
# if we find $1 in either place, use substitution to provide a fallback.
if test "$cf_cv_$1" != unknown ; then
	$3=1
fi
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WEAK_SYMBOLS version: 1 updated: 2008/08/16 19:18:06
dnl ---------------
dnl Check for compiler-support for weak symbols.
dnl This works with "recent" gcc.
AC_DEFUN([CF_WEAK_SYMBOLS],[
AC_CACHE_CHECK(if $CC supports weak symbols,cf_cv_weak_symbols,[

AC_TRY_COMPILE([
#include <stdio.h>],
[
#if defined(__GNUC__)
#  if defined __USE_ISOC99
#    define _cat_pragma(exp)	_Pragma(#exp)
#    define _weak_pragma(exp)	_cat_pragma(weak name)
#  else
#    define _weak_pragma(exp)
#  endif
#  define _declare(name)	__extension__ extern __typeof__(name) name
#  define weak_symbol(name)	_weak_pragma(name) _declare(name) __attribute__((weak))
#endif

weak_symbol(fopen);
],[cf_cv_weak_symbols=yes],[cf_cv_weak_symbols=no])
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_ABI_VERSION version: 1 updated: 2003/09/20 18:12:49
dnl -------------------
dnl Allow library's ABI to be overridden.  Generally this happens when a
dnl packager has incremented the ABI past that used in the original package,
dnl and wishes to keep doing this.
dnl
dnl $1 is the package name, if any, to derive a corresponding {package}_ABI
dnl symbol.
AC_DEFUN([CF_WITH_ABI_VERSION],[
test -z "$cf_cv_abi_version" && cf_cv_abi_version=0
AC_ARG_WITH(abi-version,
[  --with-abi-version=XXX  override derived ABI version],
[AC_MSG_WARN(overriding ABI version $cf_cv_abi_version to $withval)
 cf_cv_abi_version=$withval])
 CF_NUMBER_SYNTAX($cf_cv_abi_version,ABI version)
ifelse($1,,,[
$1_ABI=$cf_cv_abi_version
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DBMALLOC version: 6 updated: 2006/12/16 14:24:05
dnl ----------------
dnl Configure-option for dbmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DBMALLOC],[
CF_NO_LEAKS_OPTION(dbmalloc,
	[  --with-dbmalloc         test: use Conor Cahill's dbmalloc library],
	[USE_DBMALLOC])

if test "$with_dbmalloc" = yes ; then
	AC_CHECK_HEADER(dbmalloc.h,
		[AC_CHECK_LIB(dbmalloc,[debug_malloc]ifelse($1,,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_DMALLOC version: 6 updated: 2006/12/16 14:24:05
dnl ---------------
dnl Configure-option for dmalloc.  The optional parameter is used to override
dnl the updating of $LIBS, e.g., to avoid conflict with subsequent tests.
AC_DEFUN([CF_WITH_DMALLOC],[
CF_NO_LEAKS_OPTION(dmalloc,
	[  --with-dmalloc          test: use Gray Watson's dmalloc library],
	[USE_DMALLOC])

if test "$with_dmalloc" = yes ; then
	AC_CHECK_HEADER(dmalloc.h,
		[AC_CHECK_LIB(dmalloc,[dmalloc_debug]ifelse($1,,[],[,$1]))])
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_GPM version: 7 updated: 2008/03/23 14:48:54
dnl -----------
dnl
dnl The option parameter (if neither yes/no) is assumed to be the name of
dnl the gpm library, e.g., for dynamic loading.
AC_DEFUN([CF_WITH_GPM],
[
AC_MSG_CHECKING(if you want to link with the GPM mouse library)
AC_ARG_WITH(gpm,
	[  --with-gpm              use Alessandro Rubini's GPM library],
	[with_gpm=$withval],
	[with_gpm=maybe])
AC_MSG_RESULT($with_gpm)

if test "$with_gpm" != no ; then
	AC_CHECK_HEADER(gpm.h,[
		AC_DEFINE(HAVE_GPM_H)
		if test "$with_gpm" != yes && test "$with_gpm" != maybe ; then
			CF_VERBOSE(assuming we really have GPM library)
			AC_DEFINE(HAVE_LIBGPM)
		else
			AC_CHECK_LIB(gpm,Gpm_Open,[:],[
				AC_MSG_ERROR(Cannot link with GPM library)
		fi
		with_gpm=yes
		])
	],[
		test "$with_gpm" != maybe && AC_MSG_WARN(Cannot find GPM header)
		with_gpm=no
	])
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_WITH_LIBTOOL version: 19 updated: 2008/03/29 15:46:43
dnl ---------------
dnl Provide a configure option to incorporate libtool.  Define several useful
dnl symbols for the makefile rules.
dnl
dnl The reference to AC_PROG_LIBTOOL does not normally work, since it uses
dnl macros from libtool.m4 which is in the aclocal directory of automake.
dnl Following is a simple script which turns on the AC_PROG_LIBTOOL macro.
dnl But that still does not work properly since the macro is expanded outside
dnl the CF_WITH_LIBTOOL macro:
dnl
dnl	#!/bin/sh
dnl	ACLOCAL=`aclocal --print-ac-dir`
dnl	if test -z "$ACLOCAL" ; then
dnl		echo cannot find aclocal directory
dnl		exit 1
dnl	elif test ! -f $ACLOCAL/libtool.m4 ; then
dnl		echo cannot find libtool.m4 file
dnl		exit 1
dnl	fi
dnl	
dnl	LOCAL=aclocal.m4
dnl	ORIG=aclocal.m4.orig
dnl	
dnl	trap "mv $ORIG $LOCAL" 0 1 2 5 15
dnl	rm -f $ORIG
dnl	mv $LOCAL $ORIG
dnl	
dnl	# sed the LIBTOOL= assignment to omit the current directory?
dnl	sed -e 's/^LIBTOOL=.*/LIBTOOL=${LIBTOOL-libtool}/' $ACLOCAL/libtool.m4 >>$LOCAL
dnl	cat $ORIG >>$LOCAL
dnl	
dnl	autoconf-257 $*
dnl
AC_DEFUN([CF_WITH_LIBTOOL],
[
ifdef([AC_PROG_LIBTOOL],,[
LIBTOOL=
])
# common library maintenance symbols that are convenient for libtool scripts:
LIB_CREATE='${AR} -cr'
LIB_OBJECT='${OBJECTS}'
LIB_SUFFIX=.a
LIB_PREP="$RANLIB"

# symbols used to prop libtool up to enable it to determine what it should be
# doing:
LIB_CLEAN=
LIB_COMPILE=
LIB_LINK='${CC}'
LIB_INSTALL=
LIB_UNINSTALL=

AC_MSG_CHECKING(if you want to build libraries with libtool)
AC_ARG_WITH(libtool,
	[  --with-libtool          generate libraries with libtool],
	[with_libtool=$withval],
	[with_libtool=no])
AC_MSG_RESULT($with_libtool)
if test "$with_libtool" != "no"; then
ifdef([AC_PROG_LIBTOOL],[
	# missing_content_AC_PROG_LIBTOOL{{
	AC_PROG_LIBTOOL
	# missing_content_AC_PROG_LIBTOOL}}
],[
 	if test "$with_libtool" != "yes" ; then
		CF_PATH_SYNTAX(with_libtool)
		LIBTOOL=$with_libtool
	else
 		AC_PATH_PROG(LIBTOOL,libtool)
 	fi
 	if test -z "$LIBTOOL" ; then
 		AC_MSG_ERROR(Cannot find libtool)
 	fi
])dnl
	LIB_CREATE='${LIBTOOL} --mode=link ${CC} -rpath ${DESTDIR}${libdir} -version-info `cut -f1 ${srcdir}/VERSION` ${LIBTOOL_OPTS} -o'
	LIB_OBJECT='${OBJECTS:.o=.lo}'
	LIB_SUFFIX=.la
	LIB_CLEAN='${LIBTOOL} --mode=clean'
	LIB_COMPILE='${LIBTOOL} --mode=compile'
	LIB_LINK='${LIBTOOL} --mode=link ${CC} ${LIBTOOL_OPTS}'
	LIB_INSTALL='${LIBTOOL} --mode=install'
	LIB_UNINSTALL='${LIBTOOL} --mode=uninstall'
	LIB_PREP=:

	# Show the version of libtool
	AC_MSG_CHECKING(version of libtool)

	# Save the version in a cache variable - this is not entirely a good
	# thing, but the version string from libtool is very ugly, and for
	# bug reports it might be useful to have the original string.
	cf_cv_libtool_version=`$LIBTOOL --version 2>&1 | sed -e '/^$/d' |sed -e '2,$d' -e 's/([[^)]]*)//g' -e 's/^[[^1-9]]*//' -e 's/[[^0-9.]].*//'`
	AC_MSG_RESULT($cf_cv_libtool_version)
	if test -z "$cf_cv_libtool_version" ; then
		AC_MSG_ERROR(This is not GNU libtool)
	fi

	# special hack to add --tag option for C++ compiler
	case $cf_cv_libtool_version in
	1.[[5-9]]*|[[2-9]]*)
		LIBTOOL_CXX="$LIBTOOL --tag=CXX"
		LIBTOOL="$LIBTOOL --tag=CC"
		;;
	*)
		LIBTOOL_CXX="$LIBTOOL"
		;;
	esac
else
	LIBTOOL=""
	LIBTOOL_CXX=""
fi

test -z "$LIBTOOL" && ECHO_LT=

AC_SUBST(LIBTOOL)
AC_SUBST(LIBTOOL_CXX)
AC_SUBST(LIBTOOL_OPTS)

AC_SUBST(LIB_CREATE)
AC_SUBST(LIB_OBJECT)
AC_SUBST(LIB_SUFFIX)
AC_SUBST(LIB_PREP)

AC_SUBST(LIB_CLEAN)
AC_SUBST(LIB_COMPILE)
AC_SUBST(LIB_LINK)
AC_SUBST(LIB_INSTALL)
AC_SUBST(LIB_UNINSTALL)

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_PATH version: 8 updated: 2007/05/13 13:16:35
dnl ------------
dnl Wrapper for AC_ARG_WITH to ensure that user supplies a pathname, not just
dnl defaulting to yes/no.
dnl
dnl $1 = option name
dnl $2 = help-text
dnl $3 = environment variable to set
dnl $4 = default value, shown in the help-message, must be a constant
dnl $5 = default value, if it's an expression & cannot be in the help-message
dnl
AC_DEFUN([CF_WITH_PATH],
[AC_ARG_WITH($1,[$2 ](default: ifelse($4,,empty,$4)),,
ifelse($4,,[withval="${$3}"],[withval="${$3-ifelse($5,,$4,$5)}"]))dnl
if ifelse($5,,true,[test -n "$5"]) ; then
CF_PATH_SYNTAX(withval)
fi
$3="$withval"
AC_SUBST($3)dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_PATHLIST version: 5 updated: 2001/12/10 01:28:30
dnl ----------------
dnl Process an option specifying a list of colon-separated paths.
dnl
dnl $1 = option name
dnl $2 = help-text
dnl $3 = environment variable to set
dnl $4 = default value, shown in the help-message, must be a constant
dnl $5 = default value, if it's an expression & cannot be in the help-message
dnl $6 = flag to tell if we want to define or substitute
dnl
AC_DEFUN([CF_WITH_PATHLIST],[
AC_REQUIRE([CF_PATHSEP])
AC_ARG_WITH($1,[$2 ](default: ifelse($4,,empty,$4)),,
ifelse($4,,[withval=${$3}],[withval=${$3-ifelse($5,,$4,$5)}]))dnl

IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${PATHSEP}"
cf_dst_path=
for cf_src_path in $withval
do
  CF_PATH_SYNTAX(cf_src_path)
  test -n "$cf_dst_path" && cf_dst_path="${cf_dst_path}:"
  cf_dst_path="${cf_dst_path}${cf_src_path}"
done
IFS="$ac_save_ifs"

ifelse($6,define,[
# Strip single quotes from the value, e.g., when it was supplied as a literal
# for $4 or $5.
case $cf_dst_path in #(vi
\'*)
  cf_dst_path=`echo $cf_dst_path |sed -e s/\'// -e s/\'\$//`
  ;;
esac
cf_dst_path=`echo "$cf_dst_path" | sed -e 's/\\\\/\\\\\\\\/g'`
])

eval '$3="$cf_dst_path"'
AC_SUBST($3)dnl

])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_PTHREAD version: 2 updated: 2008/08/23 18:26:05
dnl ---------------
dnl Check for POSIX thread library.
AC_DEFUN([CF_WITH_PTHREAD],
[
AC_MSG_CHECKING(if you want to link with the pthread library)
AC_ARG_WITH(pthread,
    [  --with-pthread          use POSIX thread library],
    [with_pthread=$withval],
    [with_pthread=no])
AC_MSG_RESULT($with_pthread)

if test "$with_pthread" != no ; then
    AC_CHECK_HEADER(pthread.h,[
        AC_DEFINE(HAVE_PTHREADS_H)

        AC_MSG_CHECKING(if we can link with the pthread library)
        cf_save_LIBS="$LIBS"
        LIBS="-lpthread $LIBS"
        AC_TRY_LINK([
#include <pthread.h>
],[
        int rc = pthread_create(0,0,0,0);
],[with_pthread=yes],[with_pthread=no])
        LIBS="$cf_save_LIBS"
        AC_MSG_RESULT($with_pthread)

        if test "$with_pthread" = yes ; then
            LIBS="-lpthread $LIBS"
            AC_DEFINE(HAVE_LIBPTHREADS)
        else
            AC_MSG_ERROR(Cannot link with pthread library)
        fi
    ])
fi
])
dnl ---------------------------------------------------------------------------
dnl CF_WITH_REL_VERSION version: 1 updated: 2003/09/20 18:12:49
dnl -------------------
dnl Allow library's release-version to be overridden.  Generally this happens when a
dnl packager has incremented the release-version past that used in the original package,
dnl and wishes to keep doing this.
dnl
dnl $1 is the package name, if any, to derive corresponding {package}_MAJOR
dnl and {package}_MINOR symbols
dnl symbol.
AC_DEFUN([CF_WITH_REL_VERSION],[
test -z "$cf_cv_rel_version" && cf_cv_rel_version=0.0
AC_ARG_WITH(rel-version,
[  --with-rel-version=XXX  override derived release version],
[AC_MSG_WARN(overriding release version $cf_cv_rel_version to $withval)
 cf_cv_rel_version=$withval])
ifelse($1,,[
 CF_NUMBER_SYNTAX($cf_cv_rel_version,Release version)
],[
 $1_MAJOR=`echo "$cf_cv_rel_version" | sed -e 's/\..*//'`
 $1_MINOR=`echo "$cf_cv_rel_version" | sed -e 's/^[[^.]]*//' -e 's/^\.//' -e 's/\..*//'`
 CF_NUMBER_SYNTAX([$]$1_MAJOR,Release major-version)
 CF_NUMBER_SYNTAX([$]$1_MINOR,Release minor-version)
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_SYSMOUSE version: 2 updated: 2003/03/22 19:13:43
dnl ----------------
dnl If we can compile with sysmouse, make it available unless it is not wanted.
AC_DEFUN([CF_WITH_SYSMOUSE],[
# not everyone has "test -c"
if test -c /dev/sysmouse 2>/dev/null ; then
AC_MSG_CHECKING(if you want to use sysmouse)
AC_ARG_WITH(sysmouse,
	[  --with-sysmouse         use sysmouse (FreeBSD console)],
	[cf_with_sysmouse=$withval],
	[cf_with_sysmouse=maybe])
	if test "$cf_with_sysmouse" != no ; then
	AC_TRY_COMPILE([
#include <osreldate.h>
#if (__FreeBSD_version >= 400017)
#include <sys/consio.h>
#include <sys/fbio.h>
#else
#include <machine/console.h>
#endif
],[
	struct mouse_info the_mouse;
	ioctl(0, CONS_MOUSECTL, &the_mouse);
],[cf_with_sysmouse=yes],[cf_with_sysmouse=no])
	fi
AC_MSG_RESULT($cf_with_sysmouse)
test "$cf_with_sysmouse" = yes && AC_DEFINE(USE_SYSMOUSE)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_VALGRIND version: 1 updated: 2006/12/14 18:00:21
dnl ----------------
AC_DEFUN([CF_WITH_VALGRIND],[
CF_NO_LEAKS_OPTION(valgrind,
	[  --with-valgrind         test: use valgrind],
	[USE_VALGRIND])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_XOPEN_SOURCE version: 26 updated: 2008/07/27 11:26:57
dnl ---------------
dnl Try to get _XOPEN_SOURCE defined properly that we can use POSIX functions,
dnl or adapt to the vendor's definitions to get equivalent functionality,
dnl without losing the common non-POSIX features.
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
dnl	$2 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_XOPEN_SOURCE],[

AC_REQUIRE([CF_PROG_CC_U_D])

cf_XOPEN_SOURCE=ifelse($1,,500,$1)
cf_POSIX_C_SOURCE=ifelse($2,,199506L,$2)

case $host_os in #(vi
aix[[45]]*) #(vi
	CPPFLAGS="$CPPFLAGS -D_ALL_SOURCE"
	;;
freebsd*|dragonfly*) #(vi
	# 5.x headers associate
	#	_XOPEN_SOURCE=600 with _POSIX_C_SOURCE=200112L
	#	_XOPEN_SOURCE=500 with _POSIX_C_SOURCE=199506L
	cf_POSIX_C_SOURCE=200112L
	cf_XOPEN_SOURCE=600
	CPPFLAGS="$CPPFLAGS -D_BSD_TYPES -D__BSD_VISIBLE -D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE -D_XOPEN_SOURCE=$cf_XOPEN_SOURCE"
	;;
hpux*) #(vi
	CPPFLAGS="$CPPFLAGS -D_HPUX_SOURCE"
	;;
irix[[56]].*) #(vi
	CPPFLAGS="$CPPFLAGS -D_SGI_SOURCE"
	;;
linux*|gnu*|k*bsd*-gnu) #(vi
	CF_GNU_SOURCE
	;;
mirbsd*) #(vi
	# setting _XOPEN_SOURCE or _POSIX_SOURCE breaks <arpa/inet.h>
	;;
netbsd*) #(vi
	# setting _XOPEN_SOURCE breaks IPv6 for lynx on NetBSD 1.6, breaks xterm, is not needed for ncursesw
	;;
openbsd*) #(vi
	# setting _XOPEN_SOURCE breaks xterm on OpenBSD 2.8, is not needed for ncursesw
	;;
osf[[45]]*) #(vi
	CPPFLAGS="$CPPFLAGS -D_OSF_SOURCE"
	;;
nto-qnx*) #(vi
	CPPFLAGS="$CPPFLAGS -D_QNX_SOURCE"
	;;
sco*) #(vi
	# setting _XOPEN_SOURCE breaks Lynx on SCO Unix / OpenServer
	;;
solaris*) #(vi
	CPPFLAGS="$CPPFLAGS -D__EXTENSIONS__"
	;;
*)
	AC_CACHE_CHECK(if we should define _XOPEN_SOURCE,cf_cv_xopen_source,[
	AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_xopen_source=no],
	[cf_save="$CPPFLAGS"
	 CPPFLAGS="$CPPFLAGS -D_XOPEN_SOURCE=$cf_XOPEN_SOURCE"
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_xopen_source=no],
	[cf_cv_xopen_source=$cf_XOPEN_SOURCE])
	CPPFLAGS="$cf_save"
	])
])
	if test "$cf_cv_xopen_source" != no ; then
		CF_REMOVE_DEFINE(CFLAGS,$CFLAGS,_XOPEN_SOURCE)
		CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,_XOPEN_SOURCE)
		test "$cf_cv_cc_u_d_options" = yes && \
			CPPFLAGS="$CPPFLAGS -U_XOPEN_SOURCE"
		CPPFLAGS="$CPPFLAGS -D_XOPEN_SOURCE=$cf_cv_xopen_source"
	fi
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	;;
esac
])
