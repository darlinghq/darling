/*
This file is part of Darling.

Copyright (C) 2015 Lubos Dolezel

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

#ifndef DYLD_THREADS_H
#define DYLD_THREADS_H
#include <stdint.h>
#include "elfcalls.h"

#ifdef __cplusplus
extern "C" {
#endif

void* __darling_thread_create(unsigned long stack_size, unsigned long pthobj_size,
				void* entry_point, uintptr_t arg3,
				uintptr_t arg4, uintptr_t arg5, uintptr_t arg6,
				darling_thread_create_callbacks_t, void* dthread);
int __darling_thread_terminate(void* stackaddr,
				unsigned long freesize, unsigned long pthobj_size);
void* __darling_thread_get_stack(void);
int __darling_thread_rpc_socket(void);
void __darling_thread_rpc_socket_refresh(void);

#ifdef __cplusplus
}
#endif

#endif

