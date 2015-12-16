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

#ifndef PSYNCH_CV_H
#define PSYNCH_CV_H
#include "../api.h"
#include "../ipc_types.h"

int psynch_cvwait_trap(mach_task_t* task,
		struct psynch_cvwait_args* args);
int psynch_cvbroad_trap(mach_task_t* task,
		struct psynch_cvbroad_args* args);
int psynch_cvsignal_trap(mach_task_t* task,
		struct psynch_cvsignal_args* args);

#endif /* PSYNCH_CV_H */

