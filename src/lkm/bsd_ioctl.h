/*
 * Darling Mach Linux Kernel Module
 * Copyright (C) 2015 Lubos Dolezel
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef BSD_IOCTL_H
#define BSD_IOCTL_H
#include "api.h"
#include <linux/file.h>

// If only translation of ioctl request number happens,
// retval is untouched and 0 is returned.
// If the ioctl is handled completely differently, retval contains the return
// value to be returned to userspace and function returns non-zero value.

int bsd_ioctl_xlate_socket(struct file* f, struct bsd_ioctl_args* args, long* retval);
int bsd_ioctl_xlate_tty(struct file* f, struct bsd_ioctl_args* args, long* retval);
int bsd_ioctl_xlate_pts(struct file* f, struct bsd_ioctl_args* args, long* retval);
int bsd_ioctl_xlate_generic(struct file* f, struct bsd_ioctl_args* args, long* retval);

#define BSD_IOCPARM_MASK    0x1fff
#define BSD_IOCPARM_LEN(x)  (((x) >> 16) & BSD_IOCPARM_MASK)
#define BSD_IOCBASECMD(x)   ((x) & ~(IBSD_OCPARM_MASK << 16))
#define BSD_IOCGROUP(x)     (((x) >> 8) & 0xff)

#define BSD_IOCPARM_MAX     (BSD_IOCPARM_MASK + 1)
#define BSD_IOC_VOID        (uint32_t)0x20000000
#define BSD_IOC_OUT         (uint32_t)0x40000000
#define BSD_IOC_IN          (uint32_t)0x80000000
#define BSD_IOC_INOUT       (BSD_IOC_IN|BSD_IOC_OUT)
#define BSD_IOC_DIRMASK     (uint32_t)0xe0000000

#define BSD_IOC(inout,group,num,len) \
        (inout | ((len & BSD_IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define BSD_IO(g,n)        BSD_IOC(BSD_IOC_VOID,  (g), (n), 0)
#define BSD_IOR(g,n,t)     BSD_IOC(BSD_IOC_OUT,   (g), (n), sizeof(t))
#define BSD_IOW(g,n,t)     BSD_IOC(BSD_IOC_IN,    (g), (n), sizeof(t))
#define BSD_IOWR(g,n,t)    BSD_IOC(BSD_IOC_INOUT, (g), (n), sizeof(t))


#define BSD_FIOCLEX          BSD_IO('f', 1)
#define BSD_FIONCLEX         BSD_IO('f', 2)
#define BSD_FIONREAD        BSD_IOR('f', 127, int)
#define BSD_FIONBIO         BSD_IOW('f', 126, int)
#define BSD_FIOASYNC        BSD_IOW('f', 125, int)
#define BSD_FIOSETOWN       BSD_IOW('f', 124, int)
#define BSD_FIOGETOWN       BSD_IOR('f', 123, int)
#define BSD_FIODTYPE        BSD_IOR('f', 122, int)

#endif
