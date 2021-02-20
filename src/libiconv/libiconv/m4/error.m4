# error.m4 serial 10 (gettext-0.14.2)
dnl Copyright (C) 2002-2005 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_ERROR],
[
  AC_FUNC_ERROR_AT_LINE
  dnl Note: AC_FUNC_ERROR_AT_LINE does AC_LIBSOURCES([error.h, error.c]).
  gl_PREREQ_ERROR
])

# Prerequisites of lib/error.c.
AC_DEFUN([gl_PREREQ_ERROR],
[
  AC_REQUIRE([AC_FUNC_STRERROR_R])
  :
])
