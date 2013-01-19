/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

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

#ifndef MACH_PROC_H
#define MACH_PROC_H
#include <stdint.h>

extern "C"
{

// For proc_listpidspath
#define PROC_LISTPIDSPATH_PATH_IS_VOLUME 1
#define PROC_LISTPIDSPATH_EXCLUDE_EVTONLY 2 // exclude O_EVTONLY fds, not doable on Linux (no O_EVTONLY on Linux)

// For proc_listpids
#define PROC_ALL_PIDS 1
#define PROC_PGRP_ONLY 2
#define PROC_TTY_ONLY 3
#define PROC_UID_ONLY 4
#define PROC_RUID_ONLY 5

	int proc_name(int pid, void* buffer, uint32_t buffersize);
	int proc_pidpath(int pid, void* buffer, uint32_t buffersize);
	int proc_pidinfo(int pid, int flavor, uint64_t arg, void* buffer, int buffersize);
	int proc_listpids(uint32_t type, uint32_t typeinfo, void* buffer, int buffersize);
	int proc_pidfdinfo(int pid, int fd, int flavor, void* buffer, int buffersize);
	int proc_regionfilename(int pid, uint64_t address, void* buffer, uint32_t buffersize);
	int proc_kmsgbuf(void* buffer, uint32_t buffersize);
	int proc_libversion(int* major, int* minor);
	int proc_setpcontrol(const int control);
	int proc_listpidspath(uint32_t type, uint32_t typeInfo, const char* path, uint32_t pathFlags, void* buffer, int buffersize);

	struct proc_taskinfo
	{
		uint64_t pti_virtual_size,
				 pti_resident_size,
				 pti_total_user,
				 pti_total_system,
				 pti_threads_user,
				 pti_threads_system;
		int32_t pti_policy,
				pti_faults,
				pti_pageins,
				pti_cow_faults,
				pti_messages_sent,
				pti_messages_received,
				pti_syscalls_mach,
				pti_syscalls_unix,
				pti_csw,
				pti_threadnum,
				pti_numrunning,
				pti_priority;
	};

	struct proc_regioninfo
	{
		uint32_t pri_protection,
				 pri_max_protection,
				 pri_inheritance,
				 pri_flags;
		uint64_t pri_offset;
		uint32_t pri_behavior,
				 pri_user_wired_count,
				 pri_user_tag,
				 pri_pages_resident,
				 pri_pages_shared_now_private,
				 pri_pages_swapped_out,
				 pri_pages_dirtied,
				 pri_ref_count,
				 pri_shadow_depth,
				 pri_share_mode,
				 pri_private_pages_resident,
				 pri_shared_pages_resident,
				 pri_obj_id,
				 pri_depth;
		uint64_t pri_address,
				 pri_size;
	};
}

#endif

