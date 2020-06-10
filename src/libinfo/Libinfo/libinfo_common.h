/*
	Copyright (c) 2018 Apple Inc. All rights reserved.
*/

#ifndef __LIBINFO_COMMON_H__
#define __LIBINFO_COMMON_H__

#if (defined(__GNUC__) && (__GNUC__ >= 4))
#define LIBINFO_EXPORT __attribute__((visibility("default")))
#else
#define LIBINFO_EXPORT
#endif

#endif // __LIBINFO_COMMON_H__
