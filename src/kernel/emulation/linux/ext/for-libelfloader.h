#ifndef _DARLING_EMULATION_FOR_LIBELFLOADER_H_
#define _DARLING_EMULATION_FOR_LIBELFLOADER_H_

#include <darling/emulation/base.h>

// these are non-vchrooted calls for libelfloader

VISIBLE
long _open_for_libelfloader(const char* path, int flags, unsigned int mode);

VISIBLE
long _access_for_libelfloader(const char* path, int mode);

#endif // _DARLING_EMULATION_FOR_LIBELFLOADER_H_
