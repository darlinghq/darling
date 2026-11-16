/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <HIServices/CPS.h>
#include <HIServices/Processes.h>

// The CPS calls are the lower-level counterparts of the Process Manager
// calls; they are implemented on top of them, so whatever the Process Manager
// does for a serial number (or later learns to do) applies to both entry
// points.

static_assert(sizeof(CPSProcessSerNum) == sizeof(ProcessSerialNumber), "layout must match ProcessSerialNumber");

// Observed on macOS 10.14 and 26 in a session without a window server
// connection: a NULL serial number makes all three calls fail with 1003.
static const int kCPSInvalidArgument = 1003;

static inline ProcessSerialNumber toPSN(const CPSProcessSerNum* psn)
{
	ProcessSerialNumber out;
	out.highLongOfPSN = psn->hi;
	out.lowLongOfPSN = psn->lo;
	return out;
}

extern "C" int CPSGetCurrentProcess(CPSProcessSerNum* psn)
{
	if (!psn)
		return kCPSInvalidArgument;

	ProcessSerialNumber current;
	OSErr err = GetCurrentProcess(&current);
	if (err != noErr)
		return err;

	psn->hi = current.highLongOfPSN;
	psn->lo = current.lowLongOfPSN;
	return 0;
}

// The four trailing words are opaque flags and do not change the outcome.
extern "C" int CPSEnableForegroundOperation(CPSProcessSerNum* psn, uint32_t, uint32_t, uint32_t, uint32_t)
{
	if (!psn)
		return kCPSInvalidArgument;

	ProcessSerialNumber target = toPSN(psn);
	return TransformProcessType(&target, kProcessTransformToForegroundApplication);
}

extern "C" int CPSSetFrontProcess(CPSProcessSerNum* psn)
{
	if (!psn)
		return kCPSInvalidArgument;

	ProcessSerialNumber target = toPSN(psn);
	return SetFrontProcess(&target);
}
