// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <CoreServices/MacTypes.h>
extern "C" OSErr PtrToHand(const void*, Handle*, long);
extern "C" void DisposeHandle(Handle);
extern "C" long GetHandleSize(Handle);
extern "C" void SetHandleSize(Handle, long);
