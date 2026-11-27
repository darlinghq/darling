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

#ifndef DARLING_PIPEWIRE_EXPORT_H_
#define DARLING_PIPEWIRE_EXPORT_H_

/* The PipeWire client is one shared dylib below the media frameworks. Only
 * the declarations in this include directory are part of its interface. */
#if defined(__GNUC__) || defined(__clang__)
#define DARLING_PW_EXPORT __attribute__((visibility("default")))
#else
#define DARLING_PW_EXPORT
#endif

#endif
