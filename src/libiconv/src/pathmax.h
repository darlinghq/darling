/* Define PATH_MAX somehow.  Requires sys/types.h.
   Copyright (C) 1992, 1999, 2001, 2003, 2006 Free Software Foundation, Inc.

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

#ifndef _PATHMAX_H
# define _PATHMAX_H

# include <unistd.h>

/* Non-POSIX BSD systems might have gcc's limits.h, which doesn't define
   PATH_MAX but might cause redefinition warnings when sys/param.h is
   later included (as on MORE/BSD 4.3).  */
# if defined _POSIX_VERSION || !defined __GNUC__
#  include <limits.h>
# endif

# ifndef _POSIX_PATH_MAX
#  define _POSIX_PATH_MAX 255
# endif

# if !defined PATH_MAX && defined _PC_PATH_MAX
#  define PATH_MAX (pathconf ("/", _PC_PATH_MAX) < 1 ? 1024 \
		    : pathconf ("/", _PC_PATH_MAX))
# endif

/* Don't include sys/param.h if it already has been.  */
# if defined HAVE_SYS_PARAM_H && !defined PATH_MAX && !defined MAXPATHLEN
#  include <sys/param.h>
# endif

# if !defined PATH_MAX && defined MAXPATHLEN
#  define PATH_MAX MAXPATHLEN
# endif

# ifndef PATH_MAX
#  define PATH_MAX _POSIX_PATH_MAX
# endif

#endif /* _PATHMAX_H */
