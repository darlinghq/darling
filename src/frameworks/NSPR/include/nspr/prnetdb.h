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

/* Netscape Portable Runtime (NSPR) network address API. */
#ifndef _PRNETDB_H_
#define _PRNETDB_H_

#include <nspr/prtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PR_AF_INET 2
#define PR_AF_INET6 24

typedef enum PRIPAddr {
    PR_IpAddrNull = 0,
    PR_IpAddrAny = 1,
    PR_IpAddrLoopback = 2
} PRIPAddr;

typedef struct PRIPv6AddrStr { PRUint8 _S6_addr[16]; } PRIPv6Addr;

union PRNetAddr {
    struct { PRUint16 family; char data[14]; } raw;
    struct { PRUint16 family; PRUint16 port; PRUint32 ip; char pad[8]; } inet;
    struct { PRUint16 family; PRUint16 port; PRUint32 flowinfo; PRIPv6Addr ip; } ipv6;
};
typedef union PRNetAddr PRNetAddr;

PRStatus PR_InitializeNetAddr(PRIPAddr ipaddress, PRUint16 port, PRNetAddr *addr);
PRStatus PR_SetNetAddr(PRIPAddr what, PRUint16 family, PRUint16 port, PRNetAddr *addr);
PRStatus PR_GetNetAddressByName(PRNetAddr *addr, const char *hostname);
PRUint16 PR_NetAddrFamily(const PRNetAddr *addr);

#ifdef __cplusplus
}
#endif

#endif
