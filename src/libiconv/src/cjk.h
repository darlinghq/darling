/* Test for CJK encoding.
   Copyright (C) 2001-2002 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2002.

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

#include "streq.h"

/* Prototypes for local functions.  Needed to ensure compiler checking of
   function argument counts despite of K&R C function definition syntax.  */
static int is_cjk_encoding (const char *encoding);

static int
is_cjk_encoding (const char *encoding)
{
  if (0
      /* Legacy Japanese encodings */
      || STREQ (encoding, "EUC-JP", 'E', 'U', 'C', '-', 'J', 'P', 0, 0, 0)
      /* Legacy Chinese encodings */
      || STREQ (encoding, "GB2312", 'G', 'B', '2', '3', '1', '2', 0, 0, 0)
      || STREQ (encoding, "GBK", 'G', 'B', 'K', 0, 0, 0, 0, 0, 0)
      || STREQ (encoding, "EUC-TW", 'E', 'U', 'C', '-', 'T', 'W', 0, 0, 0)
      || STREQ (encoding, "BIG5", 'B', 'I', 'G', '5', 0, 0, 0, 0, 0)
      /* Legacy Korean encodings */
      || STREQ (encoding, "EUC-KR", 'E', 'U', 'C', '-', 'K', 'R', 0, 0, 0)
      || STREQ (encoding, "CP949", 'C', 'P', '9', '4', '9', 0, 0, 0, 0)
      || STREQ (encoding, "JOHAB", 'J', 'O', 'H', 'A', 'B', 0, 0, 0, 0))
    return 1;
  return 0;
}
