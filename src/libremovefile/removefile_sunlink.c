/* srm */
/* Copyright (c) 2000 Matthew D. Gauthier
 * Portions copyright (c) 2007 Apple Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the contributors shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <time.h>
#include <unistd.h>

#if HAVE_LINUX_EXT2_FS_H
#include <linux/ext2_fs.h>
#endif

#if HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif

#if HAVE_SYS_PARAM_H && HAVE_SYS_MOUNT_H
#include <sys/param.h>
#include <sys/mount.h>
#endif

#if __APPLE__
#include <sys/disk.h>
#endif

#if HAVE_CHFLAGS
/* define unsupported flags as 0 */
# if !defined UF_IMMUTABLE
#  define UF_IMMUTABLE 0
# endif
# if !defined UF_APPEND
#  define UF_APPEND 0
# endif
# if !defined UF_NOUNLINK
#  define UF_NOUNLINK 0
# endif
# if !defined SF_IMMUTABLE
#  define SF_IMMUTABLE 0
# endif
# if !defined SF_APPEND
#  define SF_APPEND 0
# endif
# if !defined SF_NOUNLINK
#  define SF_NOUNLINK 0
# endif
#endif

#include "removefile.h"
#include "removefile_priv.h"

static int
init_write_buffer(struct stat *statbuf, struct statfs *fs_stats, removefile_state_t state) {
  u_int32_t tmp_buffsize;

  state->file_size = statbuf->st_size;
  state->buffsize = statbuf->st_blksize;

#if HAVE_SYS_PARAM_H
  /* try to determine an optimal write buffer size */
  state->buffsize = (u_int32_t)(statbuf->st_size / statbuf->st_blksize) * statbuf->st_blksize;
  if ((statbuf->st_size % statbuf->st_blksize) != 0) {
    /* add full size of last block */
    state->buffsize += statbuf->st_blksize;
  } else if (state->buffsize < statbuf->st_blksize) {
    /* no smaller than one device block */
    state->buffsize = statbuf->st_blksize;
  }
  tmp_buffsize = MAXBSIZE;
  if (state->buffsize > tmp_buffsize) {
    /* no larger than the largest file system buffer size */
    state->buffsize = tmp_buffsize;
  }
#endif

  /* Allocated buffer must be at least 2 bytes larger than logical buffsize.
     This lets us align repeating 3-byte patterns across multiple buffer
     writes by using a variable offset (0..2) from the start of the buffer. */

  tmp_buffsize = state->buffsize + 4;

  if (state->buffer) {
    if (tmp_buffsize > state->allocated_buffsize) {
      free(state->buffer);
      state->buffer = NULL;
    } else {
      return 0; /* use existing buffer */
    }
  }
  if ((state->buffer = (unsigned char *)malloc(tmp_buffsize)) == NULL) {
    errno = ENOMEM;
    return -1;
  }
  state->allocated_buffsize = tmp_buffsize;
  return 0;
}

static void
flush(int fd) {
  /* force buffered writes to be flushed to disk */
#if defined F_FULLFSYNC
  /* F_FULLFSYNC is equivalent to fsync plus device flush to media */
  if (fcntl(fd, F_FULLFSYNC, NULL) != 0) {
    /* we're not on a fs that supports this; fall back to plain fsync */
    fsync(fd);
  }
#elif HAVE_FDATASYNC
  fdatasync(fd);
#else
  fsync(fd);
#endif
}

static unsigned char *align_buffer(unsigned char *buf, off_t pos) {
  /* return a pointer to the start of the buffer which should be written,
     offset from the given buffer by 0, 1, or 2 bytes, so that the 3-byte
     pattern which the buffer contains is aligned with the previous write. */
  return (unsigned char *)((uintptr_t)buf + (unsigned int)(pos % 3));
}

#if 0 /* UNUSED */
void verification_failure(off_t count) {
  if (sizeof(off_t) == 4)
    printf("warning: failed to verify write at offset %d\n", count);
  else if (sizeof(off_t) == 8)
    printf("warning: failed to verify write at offset %lld\n", count);
  else
    printf("warning: previous write failed to verify!\n");
  fflush(stdout);
}
#endif /* UNUSED */

static void
overwrite(int stage, removefile_state_t state) {
  ssize_t i;
  off_t count = 0;
  unsigned char *buffptr = state->buffer;

	// break out of the function early if cancel is detected
	if (__removefile_state_test_cancel(state)) return;

  lseek(state->file, 0, SEEK_SET);
  while (count < state->file_size - state->buffsize) {
    if (stage == 1 /* W_RANDOM */) {
      __removefile_randomize_buffer(state->buffer, state->buffsize, state);
    } else if (stage == 2 /* W_TRIPLE */) {
      buffptr = align_buffer(state->buffer, count);
    }
    i = write(state->file, buffptr, state->buffsize);
    if (i > 0)
      count += i;

    // break out of the loop early if cancel is detected
    if (__removefile_state_test_cancel(state)) return;
  }
  if (stage == 1 /* W_RANDOM */) {
     __removefile_randomize_buffer(state->buffer, (size_t)(state->file_size - count), state);
  } else if (stage == 2 /* W_TRIPLE */) {
    buffptr = align_buffer(state->buffer, count);
  }
  i = write(state->file, buffptr, (size_t)(state->file_size - count));
  /*
   * Only flush the data if we're doing more than one pass of writes.
   */
  if ((state->unlink_flags & (REMOVEFILE_SECURE_7_PASS | REMOVEFILE_SECURE_35_PASS | REMOVEFILE_SECURE_3_PASS)) != 0)
    flush(state->file);
  lseek(state->file, 0, SEEK_SET);
}

static void
overwrite_random(int num_passes, removefile_state_t state) {
  int i;

  for (i = 0; i < num_passes; i++) {
    overwrite(1 /* W_RANDOM */, state);
  }
}

static void
overwrite_byte(int byte, removefile_state_t state) {
  memset(state->buffer, byte, state->buffsize);
  overwrite(0 /* W_SINGLE */, state);
}

static void 
overwrite_bytes(unsigned int byte1, unsigned int byte2, unsigned int byte3, removefile_state_t state) {
  u_int32_t val[3], *p = (u_int32_t *)state->buffer;
  unsigned int i, mod12buffsize = state->allocated_buffsize - (state->allocated_buffsize % 12);
  
  val[0] = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte1;
  val[1] = (byte2 << 24) | (byte3 << 16) | (byte1 << 8) | byte2;
  val[2] = (byte3 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;

  /* fill buffer 12 bytes at a time, optimized for 4-byte alignment */
  for (i = 0; i < mod12buffsize; i += 12) {
    *p++ = val[0];
    *p++ = val[1];
    *p++ = val[2];
  }
  while (i < state->allocated_buffsize) {
    state->buffer[i] = ((unsigned char *)&val[0])[i % 3];
    i++;
  }
  overwrite(2 /* W_TRIPLE */, state);
}

static void
overwrite_file(removefile_state_t state) {
  if (state->unlink_flags & REMOVEFILE_SECURE_35_PASS) {
    /* Gutmann 35-pass overwrite */
    overwrite_random(4, state);
    overwrite_byte(0x55, state);
    overwrite_byte(0xAA, state);
    overwrite_bytes(0x92, 0x49, 0x24, state);
    overwrite_bytes(0x49, 0x24, 0x92, state);
    overwrite_bytes(0x24, 0x92, 0x49, state);
    overwrite_byte(0x00, state);
    overwrite_byte(0x11, state);
    overwrite_byte(0x22, state);
    overwrite_byte(0x33, state);
    overwrite_byte(0x44, state);
    overwrite_byte(0x55, state);
    overwrite_byte(0x66, state);
    overwrite_byte(0x77, state);
    overwrite_byte(0x88, state);
    overwrite_byte(0x99, state);
    overwrite_byte(0xAA, state);
    overwrite_byte(0xBB, state);
    overwrite_byte(0xCC, state);
    overwrite_byte(0xDD, state);
    overwrite_byte(0xEE, state);
    overwrite_byte(0xFF, state);
    overwrite_bytes(0x92, 0x49, 0x24, state);
    overwrite_bytes(0x49, 0x24, 0x92, state);
    overwrite_bytes(0x24, 0x92, 0x49, state);
    overwrite_bytes(0x6D, 0xB6, 0xDB, state);
    overwrite_bytes(0xB6, 0xDB, 0x6D, state);
    overwrite_bytes(0xDB, 0x6D, 0xB6, state);
    overwrite_random(4, state);
  } else if (state->unlink_flags & REMOVEFILE_SECURE_7_PASS) {
    /* DoD-compliant 7-pass overwrite */
    overwrite_byte(0xF6, state);
    overwrite_byte(0x00, state);
    overwrite_byte(0xFF, state);
    overwrite_random(1, state);
    overwrite_byte(0x00, state);
    overwrite_byte(0xFF, state);
    overwrite_random(1, state);
  } else if (state->unlink_flags & REMOVEFILE_SECURE_3_PASS) {
    /* DOE M2051-2 or DOD 5220.22-M */
    overwrite_random(2, state);
    overwrite_byte(0xAA, state);
  } else if (state->unlink_flags & REMOVEFILE_SECURE_1_PASS) {
    overwrite_random(1, state);
  } else if (state->unlink_flags & REMOVEFILE_SECURE_1_PASS_ZERO) {
    overwrite_byte(0, state);
  }
}

int
__removefile_sunlink(const char *path, removefile_state_t state) {
  struct stat statbuf;
  struct statfs fs_stats;
#if HAVE_LINUX_EXT2_FS_H
  int flags = 0;
#endif
  int fmode = O_WRONLY;
  struct flock flock;

  if (lstat(path, &statbuf) == -1) 
    return -1;
  if (!S_ISREG(statbuf.st_mode))
    return __removefile_rename_unlink(path, state);
  if (statbuf.st_nlink > 1) {
    return __removefile_rename_unlink(path, state);
  }

  if ( (state->file = open(path, fmode)) == -1) /* BSD doesn't support O_SYNC */
    return -1;
  if (fcntl(state->file, F_WRLCK, &flock) == -1) {
    close(state->file);
    state->file = -1;
    return -1;
  }

  if (fstatfs(state->file, &fs_stats) == -1 && errno != ENOSYS) {
    close(state->file);
    state->file = -1;
    return -1;
  }

#if HAVE_LINUX_EXT2_FS_H
  if (fs_stats.f_type == EXT2_SUPER_MAGIC) 
    if (ioctl(state->file, EXT2_IOC_GETFLAGS, &flags) == -1) {
      close(state->file);
      state->file = -1;
      return -1;
    } 

  if ( (flags & EXT2_UNRM_FL) || (flags & EXT2_IMMUTABLE_FL) ||
      (flags & EXT2_APPEND_FL) )  
    {
      close(state->file);
      state->file = -1;
      errno = EPERM;
      return -1;
    }

#endif /* HAVE_LINUX_EXT2_FS_H */

/* chflags(2) turns out to be a different system call in every BSD
   derivative. The important thing is to make sure we'll be able to
   unlink it after we're through messing around. Unlinking it first
   would remove the need for any of these checks, but would leave the
   user with no way to overwrite the file if the process was
   interrupted during the overwriting. So, instead we assume that the
   open() above will fail on immutable and append-only files and try
   and catch only platforms supporting NOUNLINK here.

   FreeBSD - supports NOUNLINK (from 4.4 on?)
   MacOS X - doesn't support NOUNLINK (as of 10.3.5)
   OpenBSD - doesn't support NOUNLINK (as of 3.1)
   Tru64   - unknown
   
   Note: unsupported flags are defined as 0 at the top of this file,
   so a specific platform check is not required here.
*/

#if HAVE_CHFLAGS
  if ((statbuf.st_flags & UF_IMMUTABLE) || 
      (statbuf.st_flags & UF_APPEND) ||
      (statbuf.st_flags & UF_NOUNLINK) || 
      (statbuf.st_flags & SF_IMMUTABLE) ||
      (statbuf.st_flags & SF_APPEND) ||
      (statbuf.st_flags & SF_NOUNLINK)) 
    {
      close(state->file);
      state->file = -1;
      errno = EPERM;
      return -1;
    }
#endif /* HAVE_CHFLAGS */

  if (init_write_buffer(&statbuf, &fs_stats, state) == -1) {
    close(state->file);
    state->file = -1;
    return -1;
  }
#if defined F_NOCACHE
  /* before performing file I/O, set F_NOCACHE to prevent caching */
  (void)fcntl(state->file, F_NOCACHE, 1);
#endif
  overwrite_file(state);
#if HAVE_LINUX_EXT2_FS_H
  ioctl(state->file, EXT2_IOC_SETFLAGS, EXT2_SECRM_FL);
#endif

  close(state->file);
  state->file = -1;
#if __APPLE__
  /* Also overwrite the file's resource fork, if present. */
  {
    static const char *RSRCFORKSPEC = "/..namedfork/rsrc";
    off_t rsrc_fork_size;
    size_t rsrc_path_size = strlen(path) + strlen(RSRCFORKSPEC) + 1;
    char *rsrc_path = (char *)alloca(rsrc_path_size);
    if (rsrc_path == NULL) {
      errno = ENOMEM;
      return -1;
    }
    if (snprintf(rsrc_path, MAXPATHLEN,
        "%s%s", path, RSRCFORKSPEC ) > MAXPATHLEN - 1) {
      errno = ENAMETOOLONG;
      return -1;
    }

    if (lstat(rsrc_path, &statbuf) != 0) {
      int err = errno;
      if (err == ENOENT || err == ENOTDIR) {
        rsrc_fork_size = 0;
      } else {
        return -1;
      }
    } else {
      rsrc_fork_size = statbuf.st_size;
    }

    if (rsrc_fork_size > 0) {

      if ((state->file = open(rsrc_path, O_WRONLY)) == -1) {
        return -1;
      }
      if (fcntl(state->file, F_WRLCK, &flock) == -1) {
        close(state->file);
	state->file = -1;
        return -1;
      }
  
      if (init_write_buffer(&statbuf, &fs_stats, state) == -1) {
        close(state->file);
	state->file = -1;
        return -1;
      }

    #if defined F_NOCACHE
      /* before performing file I/O, set F_NOCACHE to prevent caching */
      (void)fcntl(state->file, F_NOCACHE, 1);
    #endif

      overwrite_file(state);

      close(state->file);
      state->file = -1;
    }
  }
#endif /* __APPLE__ */

	if (__removefile_state_test_cancel(state)) {
		errno = ECANCELED;
		return -1;
	}

  return __removefile_rename_unlink(path, state);
}
