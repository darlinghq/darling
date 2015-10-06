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

#ifndef IPC_TYPES_H
#define IPC_TYPES_H

struct ipc_space;
typedef struct ipc_space ipc_namespace_t;

struct darling_mach_port;
typedef struct darling_mach_port darling_mach_port_t;

struct mach_task;
typedef struct mach_task mach_task_t;

struct mach_port_right;
typedef struct mach_port_right darling_mach_port_right_t;

#endif
