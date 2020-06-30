# onceonly.m4 serial 4 (gettext-0.15)
dnl Copyright (C) 2002-2003, 2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This file defines some "once only" variants of standard autoconf macros.
dnl   AC_CHECK_HEADERS_ONCE         like  AC_CHECK_HEADERS
dnl   AC_CHECK_FUNCS_ONCE           like  AC_CHECK_FUNCS
dnl   AC_CHECK_DECLS_ONCE           like  AC_CHECK_DECLS
dnl   AC_REQUIRE([AC_HEADER_STDC])  like  AC_HEADER_STDC
dnl The advantage is that the check for each of the headers/functions/decls
dnl will be put only once into the 'configure' file. It keeps the size of
dnl the 'configure' file down, and avoids redundant output when 'configure'
dnl is run.
dnl The drawback is that the checks cannot be conditionalized. If you write
dnl   if some_condition; then gl_CHECK_HEADERS(stdlib.h); fi
dnl inside an AC_DEFUNed function, the gl_CHECK_HEADERS macro call expands to
dnl empty, and the check will be inserted before the body of the AC_DEFUNed
dnl function.

dnl This file is only needed in autoconf <= 2.59.  Newer versions of autoconf
dnl have this macro built-in.  But about AC_CHECK_DECLS_ONCE: note that in
dnl autoconf >= 2.60 the symbol separator is a comma, whereas here it is
dnl whitespace.

dnl Autoconf version 2.57 or newer is recommended.
AC_PREREQ(2.54)

# AC_CHECK_HEADERS_ONCE(HEADER1 HEADER2 ...) is a once-only variant of
# AC_CHECK_HEADERS(HEADER1 HEADER2 ...).
AC_DEFUN([AC_CHECK_HEADERS_ONCE], [
  :
  AC_FOREACH([gl_HEADER_NAME], [$1], [
    AC_DEFUN([gl_CHECK_HEADER_]m4_quote(translit(m4_defn([gl_HEADER_NAME]),
                                                 [-./], [___])), [
      AC_CHECK_HEADERS(gl_HEADER_NAME)
    ])
    AC_REQUIRE([gl_CHECK_HEADER_]m4_quote(translit(gl_HEADER_NAME,
                                                   [-./], [___])))
  ])
])

# AC_CHECK_FUNCS_ONCE(FUNC1 FUNC2 ...) is a once-only variant of
# AC_CHECK_FUNCS(FUNC1 FUNC2 ...).
AC_DEFUN([AC_CHECK_FUNCS_ONCE], [
  :
  AC_FOREACH([gl_FUNC_NAME], [$1], [
    AC_DEFUN([gl_CHECK_FUNC_]m4_defn([gl_FUNC_NAME]), [
      AC_CHECK_FUNCS(m4_defn([gl_FUNC_NAME]))
    ])
    AC_REQUIRE([gl_CHECK_FUNC_]m4_defn([gl_FUNC_NAME]))
  ])
])

# AC_CHECK_DECLS_ONCE(DECL1 DECL2 ...) is a once-only variant of
# AC_CHECK_DECLS(DECL1, DECL2, ...).
AC_DEFUN([AC_CHECK_DECLS_ONCE], [
  :
  AC_FOREACH([gl_DECL_NAME], [$1], [
    AC_DEFUN([gl_CHECK_DECL_]m4_defn([gl_DECL_NAME]), [
      AC_CHECK_DECLS(m4_defn([gl_DECL_NAME]))
    ])
    AC_REQUIRE([gl_CHECK_DECL_]m4_defn([gl_DECL_NAME]))
  ])
])
