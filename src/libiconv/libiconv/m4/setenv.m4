# setenv.m4 serial 5
dnl Copyright (C) 2001-2004 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gt_FUNC_SETENV],
[
  AC_REPLACE_FUNCS(setenv unsetenv)
  if test $ac_cv_func_setenv = no; then
    gl_PREREQ_SETENV
  fi
  if test $ac_cv_func_unsetenv = no; then
    gl_PREREQ_UNSETENV
  else
    AC_CACHE_CHECK([for unsetenv() return type], gt_cv_func_unsetenv_ret,
      [AC_TRY_COMPILE([#include <stdlib.h>
extern
#ifdef __cplusplus
"C"
#endif
#if defined(__STDC__) || defined(__cplusplus)
int unsetenv (const char *name);
#else
int unsetenv();
#endif
], , gt_cv_func_unsetenv_ret='int', gt_cv_func_unsetenv_ret='void')])
    if test $gt_cv_func_unsetenv_ret = 'void'; then
      AC_DEFINE(VOID_UNSETENV, 1, [Define if unsetenv() returns void, not int.])
    fi
  fi
])

# Check if a variable is properly declared.
# gt_CHECK_VAR_DECL(includes,variable)
AC_DEFUN([gt_CHECK_VAR_DECL],
[
  define([gt_cv_var], [gt_cv_var_]$2[_declaration])
  AC_MSG_CHECKING([if $2 is properly declared])
  AC_CACHE_VAL(gt_cv_var, [
    AC_TRY_COMPILE([$1
      extern struct { int foo; } $2;],
      [$2.foo = 1;],
      gt_cv_var=no,
      gt_cv_var=yes)])
  AC_MSG_RESULT($gt_cv_var)
  if test $gt_cv_var = yes; then
    AC_DEFINE([HAVE_]translit($2, [a-z], [A-Z])[_DECL], 1,
              [Define if you have the declaration of $2.])
  fi
])

# Prerequisites of lib/setenv.c.
AC_DEFUN([gl_PREREQ_SETENV],
[
  AC_REQUIRE([AC_FUNC_ALLOCA])
  AC_CHECK_HEADERS_ONCE(unistd.h)
  AC_CHECK_HEADERS(search.h)
  AC_CHECK_FUNCS(tsearch)
  gt_CHECK_VAR_DECL([#include <errno.h>], errno)
  gt_CHECK_VAR_DECL([#include <unistd.h>], environ)
])

# Prerequisites of lib/unsetenv.c.
AC_DEFUN([gl_PREREQ_UNSETENV],
[
  AC_CHECK_HEADERS_ONCE(unistd.h)
  gt_CHECK_VAR_DECL([#include <errno.h>], errno)
  gt_CHECK_VAR_DECL([#include <unistd.h>], environ)
])
