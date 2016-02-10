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

#include "bsd_ioctl.h"
#include "debug.h"
#include <linux/fs.h>
#include <linux/dcache.h>

#define	D_TAPE	1
#define	D_DISK	2
#define	D_TTY	3

int bsd_ioctl_xlate_socket(struct file* f, struct bsd_ioctl_args* args, long* retval)
{
 	return bsd_ioctl_xlate_generic(f, args, retval);
}

int bsd_ioctl_xlate_tty(struct file* f, struct bsd_ioctl_args* args, long* retval)
{
	return bsd_ioctl_xlate_generic(f, args, retval);
}

int bsd_ioctl_xlate_pts(struct file* f, struct bsd_ioctl_args* args, long* retval)
{
	return bsd_ioctl_xlate_generic(f, args, retval);
}

int bsd_ioctl_xlate_generic(struct file* f, struct bsd_ioctl_args* args, long* retval)
{
	switch (args->request)
	{
		case BSD_FIODTYPE:
		{
			char name[60];
			
			if (d_path(&f->f_path, name, sizeof(name)))
			{	
				int type;
				if (strncmp(name, "/dev/pts", 8) == 0 || strncmp(name, "/dev/tty", 8) == 0)
					type = D_TTY;
				else if (strncmp(name, "/dev/st", 7) == 0 || strncmp(name, "/dev/nst", 8) == 0)
					type = D_TAPE;
				else
					type = D_DISK;
				*retval = type;
				
				return 1;
			}
			
			break;
		}
	}
	
	return 0;
}
