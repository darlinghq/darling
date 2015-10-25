/* Setting environment variables.
   Copyright (C) 2001-2004 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#if HAVE_SETENV || HAVE_UNSETENV

/* Get setenv(), unsetenv() declarations.  */
# include <stdlib.h>

#endif

#ifdef	__cplusplus
extern "C" {
#endif

#if !HAVE_SETENV

/* Set NAME to VALUE in the environment.
   If REPLACE is nonzero, overwrite an existing value.  */
extern int setenv (const char *name, const char *value, int replace);

#endif

#if HAVE_UNSETENV

# if VOID_UNSETENV
/* On some systems, unsetenv() returns void.
   This is the case for FreeBSD 4.8, NetBSD 1.6, OpenBSD 3.4.  */
#  define unsetenv(name) ((unsetenv)(name), 0)
# endif

#else

/* Remove the variable NAME from the environment.  */
extern int unsetenv (const char *name);

#endif

#ifdef	__cplusplus
}
#endif
