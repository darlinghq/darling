// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <CoreServices/MacTypes.h>
using CFStringRef = const void*;
extern "C" CFStringRef CFStringCreateWithCharacters(const void*, const UniChar*, long);
extern "C" Boolean CFStringGetFileSystemRepresentation(CFStringRef, char*, long);
extern "C" void CFRelease(const void*);
