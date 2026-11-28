/*
  This file is part of Darling.

  Copyright (C) 2026 Lubos Dolezel

  Darling is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Darling is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Netscape Portable Runtime (NSPR) file/socket API subset. */
#ifndef _PRIO_H_
#define _PRIO_H_

#include <nspr/prtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PRFileDescStr PRFileDesc;
typedef struct PRFileInfoStr PRFileInfo;

PRFileDesc *PR_OpenTCPSocket(int family);
PRFileDesc *PR_OpenUDPSocket(int family);
PRStatus PR_Close(PRFileDesc *fd);
PRStatus PR_Connect(PRFileDesc *fd, const void *addr, unsigned int timeout);
PRStatus PR_Bind(PRFileDesc *fd, const void *addr);
PRInt32 PR_Read(PRFileDesc *fd, void *buf, PRInt32 amount);
PRInt32 PR_Write(PRFileDesc *fd, const void *buf, PRInt32 amount);

#ifdef __cplusplus
}
#endif

#endif
