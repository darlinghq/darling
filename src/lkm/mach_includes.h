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

#ifndef MACH_INCLUDES_H
#define MACH_INCLUDES_H

#include <mach/port.h>
#include <mach/message.h>
#include <mach/mig.h>
#include <mach/mig_errors.h>
// #include <mach/mach_types.h>

/* Undef some names that conflict with Linux defines */
#undef __used
#undef __deprecated
#undef __pure
#undef __unused
#undef ntohs
#undef SIZE_MAX
#undef PAGE_SIZE
#undef PAGE_SHIFT
#undef PAGE_MASK
#undef NSEC_PER_USEC
#undef NSEC_PER_MSEC
#undef NSEC_PER_SEC
#undef USEC_PER_SEC

#endif
