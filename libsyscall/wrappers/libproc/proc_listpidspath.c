/*
 * Copyright (c) 2007, 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <libproc.h>


typedef struct {
	// process IDs
	int				*pids;
	int				pids_count;
	size_t				pids_size;

	// threads
	uint64_t			*threads;
	int				thr_count;
	size_t				thr_size;

	// open file descriptors
	struct proc_fdinfo		*fds;
	int				fds_count;
	size_t				fds_size;

	// file/volume of interest
	struct stat			match_stat;

	// flags
	uint32_t			flags;

} fdOpenInfo, *fdOpenInfoRef;


/*
 * check_init
 */
static fdOpenInfoRef
check_init(const char *path, uint32_t flags)
{
	fdOpenInfoRef	info;
	int		status;

	info = malloc(sizeof(*info));
	if (!info)
		return NULL;

	info->pids		= NULL;
	info->pids_count	= 0;
	info->pids_size		= 0;

	info->threads		= NULL;
	info->thr_count		= 0;
	info->thr_size		= 0;

	info->fds		= NULL;
	info->fds_count		= 0;
	info->fds_size		= 0;

	status = stat(path, &info->match_stat);
	if (status == -1) {
		goto fail;
	}

	info->flags		= flags;

	return info;

    fail :

	free(info);
	return NULL;
}


/*
 * check_free
 */
static void
check_free(fdOpenInfoRef info)
{
	if (info->pids != NULL) {
		free(info->pids);
	}

	if (info->threads != NULL) {
		free(info->threads);
	}

	if (info->fds != NULL) {
		free(info->fds);
	}

	free(info);

	return;
}


/*
 * check_file
 *   check if a process vnode is of interest
 *
 *   in  : vnode stat(2)
 *   out : -1 if error
 *          0 if no match
 *          1 if match
 */
static int
check_file(fdOpenInfoRef info, struct vinfo_stat *sb)
{
	if (sb->vst_dev == 0) {
		// if no info
		return 0;
	}

	if (sb->vst_dev != info->match_stat.st_dev) {
		// if not the requested filesystem
		return 0;
	}

	if (!(info->flags & PROC_LISTPIDSPATH_PATH_IS_VOLUME) &&
	    (sb->vst_ino != info->match_stat.st_ino)) {
		// if not the requested file
		return 0;
	}

	return 1;
}


/*
 * check_process_vnodes
 *   check [process] current working directory
 *   check [process] root directory
 *
 *   in  : pid
 *   out : -1 if error
 *          0 if no match
 *          1 if match
 */
static int
check_process_vnodes(fdOpenInfoRef info, int pid)
{
	int				buf_used;
	int				status;
	struct proc_vnodepathinfo	vpi;

	buf_used = proc_pidinfo(pid, PROC_PIDVNODEPATHINFO, 0, &vpi, sizeof(vpi));
	if (buf_used <= 0) {
		if (errno == ESRCH) {
			// if the process is gone
			return 0;
		}
		return -1;
	} else if (buf_used < sizeof(vpi)) {
		// if we didn't get enough information
		return -1;
	}

	// processing current working directory
	status = check_file(info, &vpi.pvi_cdir.vip_vi.vi_stat);
	if (status != 0) {
		// if error or match
		return status;
	}

	// processing root directory
	status = check_file(info, &vpi.pvi_rdir.vip_vi.vi_stat);
	if (status != 0) {
		// if error or match
		return status;
	}

	return 0;
}


/*
 * check_process_text
 *   check [process] text (memory)
 *
 *   in  : pid
 *   out : -1 if error
 *          0 if no match
 *          1 if match
 */
static int
check_process_text(fdOpenInfoRef info, int pid)
{
	int		status;
	int		buf_used;
	struct proc_regionwithpathinfo	rwpi;

	if (info->flags & PROC_LISTPIDSPATH_PATH_IS_VOLUME) {

		// ask for first memory region that matches mountpoint
		buf_used = proc_pidinfo(pid, PROC_PIDREGIONPATHINFO3, info->match_stat.st_dev, &rwpi, sizeof(rwpi));
		if (buf_used <= 0) {
			if ((errno == ESRCH) || (errno == EINVAL)) {
				// if no more text information is available for this process.
				return 0;
			}
			return -1;
		} else if (buf_used < sizeof(rwpi)) {
			// if we didn't get enough information
			return -1;
		}
		
		status = check_file(info, &rwpi.prp_vip.vip_vi.vi_stat);
		if (status != 0) {
			// if error or match
			return status;
		}
	} else {
		uint64_t	a	= 0;
		
		while (1) {	// for all memory regions
			// processing next address
			buf_used = proc_pidinfo(pid, PROC_PIDREGIONPATHINFO2, a, &rwpi, sizeof(rwpi));
			if (buf_used <= 0) {
				if ((errno == ESRCH) || (errno == EINVAL)) {
					// if no more text information is available for this process.
					break;
				}
				return -1;
			} else if (buf_used < sizeof(rwpi)) {
				// if we didn't get enough information
				return -1;
			}
			
			status = check_file(info, &rwpi.prp_vip.vip_vi.vi_stat);
			if (status != 0) {
				// if error or match
				return status;
			}
			
			a = rwpi.prp_prinfo.pri_address + rwpi.prp_prinfo.pri_size;
		}
	}

	return 0;
}


/*
 * check_process_fds
 *   check [process] open file descriptors
 *
 *   in  : pid
 *   out : -1 if error
 *          0 if no match
 *          1 if match
 */
static int
check_process_fds(fdOpenInfoRef info, int pid)
{
	int	buf_used;
	int	i;
	int	status;

	// get list of open file descriptors
	buf_used = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, NULL, 0);
	if (buf_used <= 0) {
		return -1;
	}

	while (1) {
		if (buf_used > info->fds_size) {
			// if we need to allocate [more] space
			while (buf_used > info->fds_size) {
				info->fds_size += (sizeof(struct proc_fdinfo) * 32);
			}

			if (info->fds == NULL) {
				info->fds = malloc(info->fds_size);
			} else {
				info->fds = reallocf(info->fds, info->fds_size);
			}
			if (info->fds == NULL) {
				return -1;
			}
		}

		buf_used = proc_pidinfo(pid, PROC_PIDLISTFDS, 0, info->fds, (int)info->fds_size);
		if (buf_used <= 0) {
			return -1;
		}

		if ((buf_used + sizeof(struct proc_fdinfo)) >= info->fds_size) {
			// if not enough room in the buffer for an extra fd
			buf_used = (int)(info->fds_size + sizeof(struct proc_fdinfo));
			continue;
		}

		info->fds_count = (int)(buf_used / sizeof(struct proc_fdinfo));
		break;
	}

	// iterate through each file descriptor
	for (i = 0; i < info->fds_count; i++) {
		struct proc_fdinfo	*fdp;

		fdp = &info->fds[i];
		switch (fdp->proc_fdtype) {
			case PROX_FDTYPE_VNODE : {
				int			buf_used;
				struct vnode_fdinfo	vi;

				buf_used = proc_pidfdinfo(pid, fdp->proc_fd, PROC_PIDFDVNODEINFO, &vi, sizeof(vi));
				if (buf_used <= 0) {
					if (errno == ENOENT) {
						/*
						 * The file descriptor's vnode may have been revoked. This is a
						 * bit of a hack, since an ENOENT error might not always mean the
						 * descriptor's vnode has been revoked. As the libproc API
						 * matures, this code may need to be revisited.
						 */
						continue;
					}
					return -1;
				} else if (buf_used < sizeof(vi)) {
					// if we didn't get enough information
					return -1;
				}

				if ((info->flags & PROC_LISTPIDSPATH_EXCLUDE_EVTONLY) &&
				    (vi.pfi.fi_openflags & O_EVTONLY)) {
					// if this file should be excluded
					continue;
				}

				status = check_file(info, &vi.pvi.vi_stat);
				if (status != 0) {
					// if error or match
					return status;
				}
				break;
			}
			default :
				break;
		}
	}

	return 0;
}


/*
 * check_process_threads
 *   check [process] thread working directories
 *
 *   in  : pid
 *   out : -1 if error
 *          0 if no match
 *          1 if match
 */
static int
check_process_threads(fdOpenInfoRef info, int pid)
{
	int				buf_used;
	int				status;
	struct proc_taskallinfo		tai;

	buf_used = proc_pidinfo(pid, PROC_PIDTASKALLINFO, 0, &tai, sizeof(tai));
	if (buf_used <= 0) {
		if (errno == ESRCH) {
			// if the process is gone
			return 0;
		}
		return -1;
	} else if (buf_used < sizeof(tai)) {
		// if we didn't get enough information
		return -1;
	}

	// check thread info
	if (tai.pbsd.pbi_flags & PROC_FLAG_THCWD) {
		int	i;

		// get list of threads
		buf_used = tai.ptinfo.pti_threadnum * sizeof(uint64_t);

		while (1) {
			if (buf_used > info->thr_size) {
				// if we need to allocate [more] space
				while (buf_used > info->thr_size) {
					info->thr_size += (sizeof(uint64_t) * 32);
				}

				if (info->threads == NULL) {
					info->threads = malloc(info->thr_size);
				} else {
					info->threads = reallocf(info->threads, info->thr_size);
				}
				if (info->threads == NULL) {
					return -1;
				}
			}

			buf_used = proc_pidinfo(pid, PROC_PIDLISTTHREADS, 0, info->threads, (int)info->thr_size);
			if (buf_used <= 0) {
				return -1;
			}

			if ((buf_used + sizeof(uint64_t)) >= info->thr_size) {
				// if not enough room in the buffer for an extra thread
				buf_used = (int)(info->thr_size + sizeof(uint64_t));
				continue;
			}

			info->thr_count = buf_used / sizeof(uint64_t);
			break;
		}

		// iterate through each thread
		for (i = 0; i < info->thr_count; i++) {
			uint64_t			thr	= info->threads[i];
			struct proc_threadwithpathinfo	tpi;

			buf_used = proc_pidinfo(pid, PROC_PIDTHREADPATHINFO, thr, &tpi, sizeof(tpi));
			if (buf_used <= 0) {
				if ((errno == ESRCH) || (errno == EINVAL)) {
					// if the process or thread is gone
					continue;
				}
			} else if (buf_used < sizeof(tai)) {
				// if we didn't get enough information
				return -1;
			}

			status = check_file(info, &tpi.pvip.vip_vi.vi_stat);
			if (status != 0) {
				// if error or match
				return status;
			}
		}
	}

	return 0;
}


/*
 * check_process_phase1
 *   check [process] process-wide current working and root directories
 *   check [process] open file descriptors
 *   check [process] per-thread current working and root directories
 *
 *   in  : pid
 *   out : -1 if error
 *          0 if no match
 *          1 if match
 */
static int
check_process_phase1(fdOpenInfoRef info, int pid)
{
	int	status;

	// check root and current working directory
	status = check_process_vnodes(info, pid);
	if (status != 0) {
		// if error or match
		return status;
	}

	// check open file descriptors
	status = check_process_fds(info, pid);
	if (status != 0) {
		// if error or match
		return status;
	}

	// check per-thread working directories
	status = check_process_threads(info, pid);
	if (status != 0) {
		// if error or match
		return status;
	}

	return 0;
}

/*
 * check_process_phase2
 *   check [process] text (memory)
 *
 *   in  : pid
 *   out : -1 if error
 *          0 if no match
 *          1 if match
 */
static int
check_process_phase2(fdOpenInfoRef info, int pid)
{
	int	status;

	// check process text (memory)
	status = check_process_text(info, pid);
	if (status != 0) {
		// if error or match
		return status;
	}

	return 0;
}

/*
 * proc_listpidspath
 *
 *   in  : type
 *       : typeinfo
 *       : path
 *       : pathflags
 *       : buffer
 *       : buffersize
 *   out : buffer filled with process IDs that have open file
 *         references that match the specified path or volume;
 *         return value is the bytes of the returned buffer
 *         that contains valid information.
 */
int
proc_listpidspath(uint32_t	type,
		  uint32_t	typeinfo,
		  const char	*path,
		  uint32_t	pathflags,
		  void		*buffer,
		  int		buffersize)
{
	int		buf_used;
	int		*buf_next	= (int *)buffer;
	int		i;
	fdOpenInfoRef	info;
	int		status		= -1;

	if (buffer == NULL) {
		// if this is a sizing request
		return proc_listpids(type, typeinfo, NULL, 0);
	}

	buffersize -= (buffersize % sizeof(int)); // make whole number of ints
	if (buffersize < sizeof(int)) {
		// if we can't even return a single PID
		errno = ENOMEM;
		return -1;
	}

	// init
	info = check_init(path, pathflags);
	if (info == NULL) {
		return -1;
	}

	// get list of processes
	buf_used = proc_listpids(type, typeinfo, NULL, 0);
	if (buf_used <= 0) {
		goto done;
	}

	while (1) {
		if (buf_used > info->pids_size) {
			// if we need to allocate [more] space
			while (buf_used > info->pids_size) {
				info->pids_size += (sizeof(int) * 32);
			}

			if (info->pids == NULL) {
				info->pids = malloc(info->pids_size);
			} else {
				info->pids = reallocf(info->pids, info->pids_size);
			}
			if (info->pids == NULL) {
				goto done;
			}
		}

		buf_used = proc_listpids(type, typeinfo, info->pids, (int)info->pids_size);
		if (buf_used <= 0) {
			goto done;
		}

		if ((buf_used + sizeof(int)) >= info->pids_size) {
			// if not enough room in the buffer for an extra pid
			buf_used = (int)(info->pids_size + sizeof(int));
			continue;
		}

		info->pids_count = buf_used / sizeof(int);
		break;
	}

	// iterate through each process
	buf_used = 0;
	for (i = info->pids_count - 1; i >= 0; i--) {
		int	pid;
		int	pstatus;

		pid = info->pids[i];
		if (pid == 0) {
			continue;
		}

		pstatus = check_process_phase1(info, pid);
		if (pstatus != 1) {
			// if not a match
			continue;
		}

		*buf_next++ = pid;
		buf_used += sizeof(int);

		if (buf_used >= buffersize) {
			// if we have filled the buffer
			break;
		}
	}

	if (buf_used >= buffersize) {
		// if we have filled the buffer
		status = buf_used;
		goto done;
	}

	// do a more expensive search if we still have buffer space
	for (i = info->pids_count - 1; i >= 0; i--) {
		int	pid;
		int	pstatus;

		pid = info->pids[i];
		if (pid == 0) {
			continue;
		}

		pstatus = check_process_phase2(info, pid);
		if (pstatus != 1) {
			// if not a match
			continue;
		}

		*buf_next++ = pid;
		buf_used += sizeof(int);

		if (buf_used >= buffersize) {
			// if we have filled the buffer
			break;
		}
	}

	status = buf_used;

    done :

	// cleanup
	check_free(info);

	return status;
}
