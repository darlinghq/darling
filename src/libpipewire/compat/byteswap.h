/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

/*
 * Counterpart to endian.h in this directory: libspa includes <byteswap.h> on
 * Linux, but the host header would pull in incompatible glibc typedefs.
 */

#ifndef DARLING_PIPEWIRE_BYTESWAP_COMPAT_H
#define DARLING_PIPEWIRE_BYTESWAP_COMPAT_H

#define bswap_16(value) __builtin_bswap16(value)
#define bswap_32(value) __builtin_bswap32(value)
#define bswap_64(value) __builtin_bswap64(value)

#endif
