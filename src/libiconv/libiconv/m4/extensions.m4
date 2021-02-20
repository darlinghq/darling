# Enable extensions on systems that normally disable them.

dnl Copyright (C) 2003, 2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This file is only needed in autoconf <= 2.59.  Newer versions of autoconf
dnl have a macro AC_USE_SYSTEM_EXTENSIONS with identical semantics.

# gl_USE_SYSTEM_EXTENSIONS
# ------------------------
# Enable extensions on systems that normally disable them,
# typically due to standards-conformance issues.
AC_DEFUN([gl_USE_SYSTEM_EXTENSIONS], [
  AC_BEFORE([$0], [AC_COMPILE_IFELSE])
  AC_BEFORE([$0], [AC_RUN_IFELSE])

  AC_REQUIRE([AC_GNU_SOURCE])
  AC_REQUIRE([AC_AIX])
  AC_REQUIRE([AC_MINIX])

  AH_VERBATIM([__EXTENSIONS__],
[/* Enable extensions on Solaris.  */
#ifndef __EXTENSIONS__
# undef __EXTENSIONS__
#endif])
  AC_DEFINE([__EXTENSIONS__])
])
