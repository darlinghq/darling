// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <sys/types.h>
#include <stddef.h>
#include <errno.h>
inline ssize_t getxattr(const char*, const char*, void*, size_t, unsigned int, int) { errno=ENODATA; return -1; }
