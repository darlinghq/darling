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

#ifndef DEBUG_H
#define	DEBUG_H
#undef __unused
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kernel.h>

#ifdef DARLING_DEBUG
#	define debug_msg(fmt, ...) printk(KERN_DEBUG "Darling Mach: <%d> " fmt, current->pid, ##__VA_ARGS__)
#else
#	define debug_msg(fmt, ...)
#endif

//#define DARLING_MEM_DEBUG // better use slub_debug
#ifdef DARLING_MEM_DEBUG
#	define kmalloc dbg_kmalloc
#	define kfree dbg_kfree
void* dbg_kmalloc(size_t size, int flags);
void dbg_kfree(const void* mem);
#endif

#endif	/* DEBUG_H */

