# allocsa.m4 serial 3
dnl Copyright (C) 2003-2004 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_ALLOCSA],
[
  dnl Use the autoconf tests for alloca(), but not the AC_SUBSTed variables
  dnl @ALLOCA@ and @LTALLOCA@.
  AC_REQUIRE([gl_FUNC_ALLOCA])
  AC_REQUIRE([gl_EEMALLOC])
  AC_REQUIRE([gl_AC_TYPE_LONG_LONG])
  AC_REQUIRE([gt_TYPE_LONGDOUBLE])
])
