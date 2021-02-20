# canonicalize.m4 serial 2 (gettext-0.13)
dnl Copyright (C) 2003 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_CANONICALIZE],
[
  dnl Do this replacement check manually because the file name is shorter
  dnl than the function name.
  AC_CHECK_FUNCS(canonicalize_file_name)
  if test $ac_cv_func_canonicalize_file_name = no; then
    AC_LIBOBJ(canonicalize)
    AC_DEFINE([realpath], [rpl_realpath],
      [Define to a replacement function name for realpath().])
    gl_PREREQ_CANONICALIZE
  fi
])

# Prerequisites of lib/canonicalize.c.
AC_DEFUN([gl_PREREQ_CANONICALIZE],
[
  AC_CHECK_HEADERS_ONCE(sys/param.h unistd.h)
  AC_CHECK_FUNCS(getcwd readlink)
])
