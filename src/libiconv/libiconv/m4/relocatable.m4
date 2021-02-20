# relocatable.m4 serial 5 (gettext-0.15)
dnl Copyright (C) 2003, 2005-2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.

dnl Support for relocateble programs.
AC_DEFUN([AC_RELOCATABLE],
[
  AC_REQUIRE([AC_PROG_INSTALL])
  AC_BEFORE([AC_PROG_INSTALL],[AC_RELOCATABLE])
  AC_REQUIRE([AC_LIB_LIBPATH])
  AC_REQUIRE([AC_RELOCATABLE_LIBRARY])
  AC_REQUIRE([AC_EXEEXT])
  is_noop=no
  use_elf_origin_trick=no
  if test $RELOCATABLE = yes; then
    # --enable-relocatable implies --disable-rpath
    enable_rpath=no
    AC_LIBOBJ([relocatable])
    AC_DEFINE([ENABLE_RELOCATABLE], 1,
      [Define to 1 if the package shall run at any location in the filesystem.])
    AC_CHECK_HEADERS([unistd.h mach-o/dyld.h])
    AC_CHECK_FUNCS([_NSGetExecutablePath])
    case "$host_os" in
      mingw*) is_noop=yes ;;
      linux*) use_elf_origin_trick=yes ;;
    esac
    if test $is_noop = yes; then
      SET_RELOCATABLE="RELOCATABLE_LDFLAGS = :"
    else
      if test $use_elf_origin_trick = yes; then
        dnl Use the dynamic linker's support for relocatable programs.
        case "$ac_aux_dir" in
          /*) reloc_ldflags="$ac_aux_dir/reloc-ldflags" ;;
          *) reloc_ldflags="\$(top_builddir)/$ac_aux_dir/reloc-ldflags" ;;
        esac
        SET_RELOCATABLE="RELOCATABLE_LDFLAGS = \"$reloc_ldflags\" \"\$(host)\" \"\$(RELOCATABLE_LIBRARY_PATH)\""
      else
        dnl Unfortunately we cannot define INSTALL_PROGRAM to a command
        dnl consisting of more than one word - libtool doesn't support this.
        dnl So we abuse the INSTALL_PROGRAM_ENV hook, originally meant for the
        dnl 'install-strip' target.
        SET_RELOCATABLE="INSTALL_PROGRAM_ENV = RELOC_LIBRARY_PATH_VAR=\"$shlibpath_var\" RELOC_LIBRARY_PATH_VALUE=\"\$(RELOCATABLE_LIBRARY_PATH)\" RELOC_PREFIX=\"\$(prefix)\" RELOC_COMPILE_COMMAND=\"\$(CC) \$(CPPFLAGS) \$(CFLAGS) \$(LDFLAGS)\" RELOC_SRCDIR=\"\$(RELOCATABLE_SRC_DIR)\" RELOC_BUILDDIR=\"\$(RELOCATABLE_BUILD_DIR)\" RELOC_CONFIG_H_DIR=\"\$(RELOCATABLE_CONFIG_H_DIR)\" RELOC_EXEEXT=\"\$(EXEEXT)\" RELOC_INSTALL_PROG=\"$INSTALL_PROGRAM\""
        case "$ac_aux_dir" in
          /*) INSTALL_PROGRAM="$ac_aux_dir/install-reloc" ;;
          *) INSTALL_PROGRAM="\$(top_builddir)/$ac_aux_dir/install-reloc" ;;
        esac
      fi
    fi
  else
    SET_RELOCATABLE=
  fi
  AC_SUBST([SET_RELOCATABLE])
  AM_CONDITIONAL([RELOCATABLE_VIA_LD],
    [test $is_noop = yes || test $use_elf_origin_trick = yes])
])

dnl Support for relocatable libraries.
AC_DEFUN([AC_RELOCATABLE_LIBRARY],
[
  AC_REQUIRE([AC_RELOCATABLE_NOP])
  dnl Easier to put this here once, instead of into the DEFS of each Makefile.
  if test "X$prefix" = "XNONE"; then
    reloc_final_prefix="$ac_default_prefix"
  else
    reloc_final_prefix="$prefix"
  fi
  AC_DEFINE_UNQUOTED([INSTALLPREFIX], ["${reloc_final_prefix}"],
    [Define to the value of ${prefix}, as a string.])
])

dnl Support for relocatable packages for which it is a nop.
AC_DEFUN([AC_RELOCATABLE_NOP],
[
  AC_MSG_CHECKING([whether to activate relocatable installation])
  AC_ARG_ENABLE(relocatable,
    [  --enable-relocatable    install a package that can be moved in the filesystem],
    [if test "$enableval" != no; then
       RELOCATABLE=yes
     else
       RELOCATABLE=no
     fi
    ], RELOCATABLE=no)
  AC_SUBST(RELOCATABLE)
  AC_MSG_RESULT([$RELOCATABLE])
])

dnl Determine the platform dependent parameters needed to use relocatability:
dnl shlibpath_var.
AC_DEFUN([AC_LIB_LIBPATH],
[
  AC_REQUIRE([AC_LIB_PROG_LD])            dnl we use $LD
  AC_REQUIRE([AC_CANONICAL_HOST])         dnl we use $host
  AC_REQUIRE([AC_CONFIG_AUX_DIR_DEFAULT]) dnl we use $ac_aux_dir
  AC_CACHE_CHECK([for shared library path variable], acl_cv_libpath, [
    LD="$LD" \
    ${CONFIG_SHELL-/bin/sh} "$ac_aux_dir/config.libpath" "$host" > conftest.sh
    . ./conftest.sh
    rm -f ./conftest.sh
    acl_cv_libpath=${acl_cv_shlibpath_var:-none}
  ])
  shlibpath_var="$acl_cv_shlibpath_var"
])
