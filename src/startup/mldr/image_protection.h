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

#endif // _MLDR_IMAGE_PROTECTION_H_
