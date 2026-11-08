/*
This file is part of Darling.

Copyright (C) 2026 Darling Developers

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

#ifndef _MLDR_IMAGE_PROTECTION_H_
#define _MLDR_IMAGE_PROTECTION_H_

#include <stddef.h>
#include <stdint.h>

// the largest file mldr_snapshot_image() copies (2 GiB, as DC_F_SNAPSHOT_IMAGE)
#define MLDR_SNAPSHOT_IMAGE_MAX (2ull * 1024 * 1024 * 1024)

/**
 * Copies the whole regular file `fd` refers to into a new anonymous file
 * (memfd) and seals the copy: it can no longer be written, shrunk or grown,
 * and its seals cannot be changed. Mapping the copy instead of the file means
 * that the bytes darlingserver checked are the bytes that get mapped, even if
 * the file changes afterwards.
 *
 * The offset of `fd` does not change. Returns a new close-on-exec descriptor
 * whose offset is 0, or a negative errno value: EINVAL for an empty file or
 * one that is not a regular file, EFBIG for a file larger than
 * `MLDR_SNAPSHOT_IMAGE_MAX`, EIO when the file became shorter while it was
 * copied, and the errors of fstat(), memfd_create(), pread(), write() and
 * fcntl(F_ADD_SEALS) otherwise.
 */
int mldr_snapshot_image(int fd);

/**
 * Makes a read-only mapping immutable: the pages in [addr, addr + len) are
 * copied into anonymous memory that replaces the mapping at the same address
 * with protection `prot`, and the range is then sealed with mseal(2), so it
 * can no longer be unmapped, remapped or have its protection changed. Copying
 * first matters for file mappings, because sealing does not stop the file
 * itself from changing underneath.
 *
 * `addr` must be aligned to the runtime page size and `len` must not be zero;
 * `len` is rounded up to whole pages. `prot` must include PROT_READ and not
 * PROT_WRITE. Returns 0 or a negative errno value: EINVAL for bad arguments,
 * ENOSYS when the kernel has no mseal() (Linux before 6.10, and 32-bit
 * processes, for which Linux does not implement it), and the errors of
 * mmap(), mprotect(), mremap() and mseal() otherwise. After ENOSYS from a
 * 64-bit process the copy stays in place, unsealed.
 */
int mldr_seal_range(void* addr, size_t len, int prot);

#endif // _MLDR_IMAGE_PROTECTION_H_
