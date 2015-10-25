/* Elementary types for the GNU UniString library.
   Copyright (C) 2002, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.  */

#ifndef _UNITYPES_H
#define _UNITYPES_H

/* Get uint8_t, uint16_t, uint32_t.  */
#include <stdint.h>

/* Type representing a Unicode character.  */
typedef uint32_t ucs4_t;

/* Common macro, to avoid compilation errors when we use a GCC extension and
   GCC is run in pedantic mode.  */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
# define UC_GNUC_EXTENSION __extension__
#else
# define UC_GNUC_EXTENSION
#endif

#endif /* _UNITYPES_H */
