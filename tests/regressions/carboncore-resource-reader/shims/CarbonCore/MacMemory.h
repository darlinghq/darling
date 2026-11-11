// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <CarbonCore/Resources.h>
extern "C" OSErr PtrToHand(const void* data, Handle* out, long length);
extern "C" void DisposeHandle(Handle h);
extern "C" long GetHandleSize(Handle h);
