/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <HIServices/Processes.h>
#include <LaunchServices/LaunchServicesPriv.h>
#include <atomic>
#include <CarbonCore/MacErrors.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <cstring>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFBundle.h>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <CoreGraphics/CoreGraphicsPrivate.h>

#define STUB() // TODO
#ifndef PATH_MAX
#	define PATH_MAX	4096
#endif

// TODO: 99% of these should be rewritten to call LaunchServices instead

// CFStringRef kCFBundleExecutableKey = CFSTR("kCFBundleExecutableKey");
// CFStringRef kCFBundleNameKey = CFSTR("kCFBundleNameKey");
// CFStringRef kCFBundleIdentifierKey = CFSTR("kCFBundleIdentifierKey");

// The Process Manager sits on top of LaunchServices: a process serial number is
// the process' ASN split into (high, low) parts, and everything else is looked
// up with the LaunchServices application information calls.

// Resolves a serial number (including kCurrentProcess) to a live process.
static OSErr resolvePID(const ProcessSerialNumber* psn, pid_t* pid)
{
	if (!psn)
		return paramErr;

	if (psn->highLongOfPSN == 0 && psn->lowLongOfPSN == kCurrentProcess)
	{
		*pid = getpid();
		return noErr;
	}
	if (psn->highLongOfPSN != 0 || psn->lowLongOfPSN == kNoProcess
		|| psn->lowLongOfPSN > 0x7fffffffu)
	{
		return procNotFound;
	}

	if (kill((pid_t) psn->lowLongOfPSN, 0) != 0 && errno != EPERM)
		return procNotFound;

	*pid = (pid_t) psn->lowLongOfPSN;
	return noErr;
}

static CFTypeRef copyInfoItem(pid_t pid, CFStringRef key)
{
	CFTypeRef asn = _LSASNCreateWithPid(nullptr, pid);
	CFTypeRef value = _LSCopyApplicationInformationItem(kLSDefaultSessionID, asn, key);
	CFRelease(asn);
	return value;
}

OSStatus CopyProcessName(const ProcessSerialNumber* psn, CFStringRef* name)
{
	pid_t pid;
	OSErr err;

	if (!name)
		return paramErr;
	*name = nullptr;

	err = resolvePID(psn, &pid);
	if (err != noErr)
		return err;

	*name = (CFStringRef) copyInfoItem(pid, _kLSDisplayNameKey);
	return *name ? noErr : procNotFound;
}

void ExitToShell()
{
	exit(0);
}

OSErr GetCurrentProcess(ProcessSerialNumber* psn)
{
	if (!psn)
		return paramErr;

	CFTypeRef asn = _LSASNCreateWithPid(nullptr, getpid());
	UInt32 high, low;
	bool ok = _LSASNExtractHighAndLowParts(asn, &high, &low);
	CFRelease(asn);
	if (!ok)
		return procNotFound;

	psn->highLongOfPSN = high;
	psn->lowLongOfPSN = low;
	return noErr;
}

// The frontmost process as last requested through SetFrontProcess: the fallback for when the
// window system cannot say which process has the input focus.
static std::atomic<pid_t> g_frontPID(0);

static bool processExists(pid_t pid)
{
	return pid > 0 && (kill(pid, 0) == 0 || errno == EPERM);
}

OSErr GetFrontProcess(ProcessSerialNumber* psn)
{
	if (!psn)
		return paramErr;

	// CoreGraphics (the window system layer) knows the process that really has the focus where
	// the display server tells (X11) or where processes report it (Wayland).
	pid_t front = 0;
	if (_CGSGetFrontProcess(&front) == kCGErrorSuccess && processExists(front))
	{
		if (GetProcessForPID(front, psn) == noErr)
			return noErr;
	}

	// Without an answer: the process SetFrontProcess activated last, else this one.
	front = g_frontPID.load();
	if (processExists(front))
		return GetProcessForPID(front, psn);
	return GetCurrentProcess(psn);
}

OSErr GetNextProcess(ProcessSerialNumber* psn)
{
	if (!psn)
		return paramErr;

	pid_t after = 0;
	if (psn->highLongOfPSN == 0 && psn->lowLongOfPSN == kNoProcess)
		after = 0;
	else
	{
		OSErr err = resolvePID(psn, &after);
		if (err != noErr)
			return err;
	}

	CFArrayRef list = _LSCopyApplicationArray(kLSDefaultSessionID);
	if (!list)
		return procNotFound;

	OSErr result = procNotFound;
	for (CFIndex i = 0; i < CFArrayGetCount(list); i++)
	{
		UInt32 high, low;
		if (_LSASNExtractHighAndLowParts(CFArrayGetValueAtIndex(list, i), &high, &low) && low > (UInt32) after)
		{
			psn->highLongOfPSN = high;
			psn->lowLongOfPSN = low;
			result = noErr;
			break;
		}
	}
	CFRelease(list);
	return result;
}

OSStatus GetProcessBundleLocation(const ProcessSerialNumber* psn, FSRef* location)
{
	STUB();
	memset(location, 0, sizeof(*location));
	return noErr;
}

OSStatus GetProcessForPID(pid_t pid, ProcessSerialNumber* psn)
{
	if (!psn)
		return paramErr;
	if (pid <= 0 || (kill(pid, 0) != 0 && errno != EPERM))
		return procNotFound;

	CFTypeRef asn = _LSASNCreateWithPid(nullptr, pid);
	UInt32 high, low;
	bool ok = _LSASNExtractHighAndLowParts(asn, &high, &low);
	CFRelease(asn);
	if (!ok)
		return procNotFound;

	psn->highLongOfPSN = high;
	psn->lowLongOfPSN = low;
	return noErr;
}

OSErr GetProcessInformation(const ProcessSerialNumber* psn, ProcessInfoRec* pi)
{
	pid_t pid;
	OSErr err;

	if (!pi || pi->processInfoLength != sizeof(ProcessInfoRec))
		return paramErr;

	err = resolvePID(psn, &pid);
	if (err != noErr)
		return err;

	StringPtr nameBuf = pi->processName;
	FSRef* appRef = pi->processAppRef;
	memset(pi, 0, sizeof(*pi));
	pi->processInfoLength = sizeof(*pi);
	pi->processName = nameBuf;
	pi->processAppRef = appRef;

	err = GetProcessForPID(pid, &pi->processNumber);
	if (err != noErr)
		return err;

	// Processes without a creator or file type report '????'.
	pi->processType = '????';
	pi->processSignature = '????';

	if (nameBuf)
	{
		CFStringRef name = (CFStringRef) copyInfoItem(pid, _kLSDisplayNameKey);
		nameBuf[0] = 0;
		if (name)
		{
			CFIndex used = 0;
			CFStringGetBytes(name, CFRangeMake(0, CFStringGetLength(name)), kCFStringEncodingMacRoman, '?', false, nameBuf + 1, 255, &used);
			nameBuf[0] = (unsigned char) used;
			CFRelease(name);
		}
	}
	return noErr;
}

OSStatus GetProcessPID(const ProcessSerialNumber* psn, pid_t* pid)
{
	if (!pid)
		return paramErr;
	return resolvePID(psn, pid);
}

// Whether the application of the process has not been hidden (ShowHideProcess()); the hidden state is the one
// the application publishes through CoreGraphics. A process that does not exist is not visible.
Boolean IsProcessVisible(const ProcessSerialNumber* psn)
{
	pid_t pid;

	if (!psn || resolvePID(psn, &pid) != noErr)
		return false;
	return !_CGSProcessIsHidden(pid);
}

OSErr KillProcess(const ProcessSerialNumber* psn)
{
	if (kill(psn->lowLongOfPSN, SIGTERM) == -1)
		return makeOSStatus(errno);
	
	return noErr;
}

OSErr LaunchApplication(LaunchParamBlockRec* launch)
{
	STUB();
	return unimpErr;
}

CFDictionaryRef ProcessInformationCopyDictionary(const ProcessSerialNumber *PSN, UInt32 infoToReturn)
{
	STUB();
	
	CFStringRef name, path;
	CFMutableDictionaryRef dict;
	OSStatus status;
	CFNumberRef pid;
	
	status = CopyProcessName(PSN, &name);
	if (status != noErr)
		return nullptr;

	pid_t realPID;
	if (resolvePID(PSN, &realPID) != noErr)
	{
		CFRelease(name);
		return nullptr;
	}
	path = (CFStringRef) copyInfoItem(realPID, _kLSExecutablePathKey);
	if (!path)
	{
		CFRelease(name);
		return nullptr;
	}
	
	dict = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	pid = CFNumberCreate(nullptr, kCFNumberLongType, &PSN->lowLongOfPSN);
	
	CFDictionaryAddValue(dict, CFSTR("PSN"), pid);
	CFDictionaryAddValue(dict, CFSTR("pid"), pid);
	CFDictionaryAddValue(dict, CFSTR("LSBackgroundOnly"), kCFBooleanFalse);
	CFDictionaryAddValue(dict, CFSTR("IsHiddenAttr"), kCFBooleanFalse);
	CFDictionaryAddValue(dict, CFSTR("RequiresCarbon"), kCFBooleanFalse);
	CFDictionaryAddValue(dict, kCFBundleNameKey, name);
	CFDictionaryAddValue(dict, kCFBundleExecutableKey, path);
	
	CFRelease(pid);
	CFRelease(name);
	CFRelease(path);
	
	return dict;
}

OSErr SameProcess(const ProcessSerialNumber* psn1, const ProcessSerialNumber* psn2, Boolean* result)
{
	if (!psn1 || !psn2 || !result)
		return paramErr;

	// An unresolvable serial number is simply not the same process.
	pid_t pid1, pid2;
	*result = resolvePID(psn1, &pid1) == noErr && resolvePID(psn2, &pid2) == noErr && pid1 == pid2;
	return noErr;
}

OSErr SetFrontProcess(const ProcessSerialNumber* psn)
{
	return SetFrontProcessWithOptions(psn, 0);
}

OSErr SetFrontProcessWithOptions(const ProcessSerialNumber* psn, OptionBits opts)
{
	pid_t pid;
	OSErr err;

	if (opts & ~(OptionBits) (kSetFrontProcessFrontWindowOnly | kSetFrontProcessCausedByUser))
		return paramErr;

	// The system process is accepted and left alone.
	if (psn && psn->highLongOfPSN == 0 && psn->lowLongOfPSN == kSystemProcess)
		return noErr;

	err = resolvePID(psn, &pid);
	if (err != noErr)
		return err;

	// QUIRK: an application of the window system that is prohibited from being in the foreground
	// (a bundle with LSBackgroundOnly, or a program outside any bundle) cannot be made the front
	// process: the call fails with -606 and nothing changes.
	// Not documented. Evidence: black-box observation on macOS 10.14.4 and 26.7.3 (tier 2),
	// mac-probes/front-process (the target an LSBackgroundOnly application; the same call to an
	// LSUIElement application succeeds and makes it the front process). A process that never
	// connected to the window system (a command-line tool) is not rejected.
	// Test: tests/front-process (cases errors: the background-only target answers -606).
	static const OSErr kProcessCannotBeFront = -606;

	// CoreGraphics (the window system layer) keeps the front process, the menu bar owner and the
	// activation policy each application published (what kind of application a process is, so
	// whether it is refused). It changes the front process and, for a regular application, the
	// menu bar owner when the call returns, then asks the window system to move the input focus to
	// the process' windows. The focus is a request: the window manager or compositor decides, so
	// only a bad argument or a prohibited process is an error.
	CGError cgError = _CGSRequestFrontApplication(
		pid, (opts & kSetFrontProcessFrontWindowOnly) ? kCGSRequestFrontProcessFrontWindowOnly : 0);
	if (cgError == kCGErrorInvalidOperation)
		return kProcessCannotBeFront;
	if (cgError == kCGErrorIllegalArgument)
		return paramErr;

	g_frontPID.store(pid);
	return noErr;
}

// Asks the application of the process to hide or show its windows. The request is carried out by the
// application, so success means the request was made. A process that is not an application of the window
// system is procNotFound.
OSErr ShowHideProcess(const ProcessSerialNumber* psn, Boolean visible)
{
	pid_t pid;
	OSErr err;

	if (!psn)
		return paramErr;
	err = resolvePID(psn, &pid);
	if (err != noErr)
		return err;

	CGError error = _CGSSendProcessCommand(pid, visible ? kCGSProcessCommandUnhide : kCGSProcessCommandHide);

	return error == kCGErrorSuccess ? noErr : procNotFound;
}

OSStatus TransformProcessType(const ProcessSerialNumber* psn, UInt32 type)
{
	CFStringRef newType;
	pid_t pid;
	OSErr err;

	switch (type)
	{
		case kProcessTransformToForegroundApplication:
			newType = _kLSApplicationForegroundTypeKey;
			break;
		case kProcessTransformToBackgroundApplication:
			newType = _kLSApplicationBackgroundOnlyTypeKey;
			break;
		case kProcessTransformToUIElementApplication:
			newType = _kLSApplicationUIElementTypeKey;
			break;
		default:
			return paramErr;
	}

	err = resolvePID(psn, &pid);
	if (err != noErr)
		return err;

	// QUIRK: transforming a process to the type it already has fails with
	// paramErr. Not documented. Evidence: black-box observation on macOS
	// 10.14.4 (i386, x86_64) and 26.7.3 (tier 2): a command-line process
	// (initially background-only) got paramErr for Background, noErr for
	// UIElement, noErr for Foreground, paramErr for Foreground again and noErr
	// for Background. Test: tests/src/process_manager.c (transform to the
	// current type).
	CFStringRef current = (CFStringRef) copyInfoItem(pid, _kLSApplicationTypeKey);
	bool same = current && CFEqual(current, newType);
	if (current)
		CFRelease(current);
	if (same)
		return paramErr;

	CFTypeRef asn = _LSASNCreateWithPid(nullptr, pid);
	OSStatus status = _LSSetApplicationInformationItem(kLSDefaultSessionID, asn, _kLSApplicationTypeKey, newType, nullptr);
	CFRelease(asn);

	// Becoming a foreground application concerns the window system too: it goes through
	// CPSEnableForegroundOperation (as on macOS), which publishes the process as a regular
	// application of the window system. Only the calling process can do that for itself.
	if (status == noErr && type == kProcessTransformToForegroundApplication && pid == getpid())
	{
		CPSProcessSerNum self = { 0, (uint32_t) pid };
		CPSEnableForegroundOperation(&self, 0, 0, 0, 0);
	}
	return status;
}

OSErr WakeUpProcess(const ProcessSerialNumber* psn)
{
	STUB();
	return noErr;
}

