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
 * The PipeWire client is compiled against the Darwin SDK headers, while the
 * libspa headers include the host's <endian.h>. The host header defines the
 * __int64_t typedefs a second time with a different width, so provide only the
 * byte-order macros the libspa headers actually inspect.
 */

#ifndef DARLING_PIPEWIRE_ENDIAN_COMPAT_H
#define DARLING_PIPEWIRE_ENDIAN_COMPAT_H

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __BYTE_ORDER __LITTLE_ENDIAN

#endif
