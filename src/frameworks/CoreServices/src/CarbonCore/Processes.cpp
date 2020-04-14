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

#include <CoreServices/Processes.h>
#include <CoreServices/MacErrors.h>
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
#include "UserBreak.h"

#define STUB() // TODO
#ifndef PATH_MAX
#	define PATH_MAX	4096
#endif

// CFStringRef kCFBundleExecutableKey = CFSTR("kCFBundleExecutableKey");
// CFStringRef kCFBundleNameKey = CFSTR("kCFBundleNameKey");
// CFStringRef kCFBundleIdentifierKey = CFSTR("kCFBundleIdentifierKey");

OSStatus CopyProcessName(const ProcessSerialNumber* psn, CFStringRef* name)
{
	char buf[1024];
	char path[100];
	int fd, len;

	sprintf(path, "/proc/%u/comm", (unsigned int)psn->lowLongOfPSN);

	fd = ::open(path, O_RDONLY);
	if (fd == -1)
	{
		*name = nullptr;
		return procNotFound;
	}

	len = ::read(fd, buf, sizeof(buf)-1);
	buf[len] = '\0';

	::close(fd);
	
	*name = CFStringCreateWithCString(NULL, buf, kCFStringEncodingUTF8);
	return noErr;
}

void ExitToShell()
{
	exit(0);
}

OSErr GetCurrentProcess(ProcessSerialNumber* psn)
{
	psn->highLongOfPSN = 0;
	psn->lowLongOfPSN = getpid();
	return noErr;
}

OSErr GetFrontProcess(ProcessSerialNumber* psn)
{
	STUB();
	return GetCurrentProcess(psn);
}

OSErr GetNextProcess(ProcessSerialNumber* psn)
{
	DIR* dir = opendir("/proc");
	struct dirent* ent;
	int nextAfter = psn->lowLongOfPSN;
	bool found = false, next = false;
	
	if (nextAfter == kCurrentProcess)
		nextAfter = getpid();
	else if (nextAfter == kNoProcess)
		next = true;
	
	while ((ent = readdir(dir)) != nullptr)
	{
		if (!isdigit(ent->d_name[0]))
			continue;
		if (next)
		{
			psn->lowLongOfPSN = atoi(ent->d_name);
			psn->highLongOfPSN = 0;
			found = true;
			break;
		}
		else if (atoi(ent->d_name) == nextAfter)
			next = true;
	}
	
	closedir(dir);
	
	return found ? noErr : procNotFound;
}

OSStatus GetProcessBundleLocation(const ProcessSerialNumber* psn, FSRef* location)
{
	STUB();
	memset(location, 0, sizeof(*location));
	return noErr;
}

OSStatus GetProcessForPID(pid_t pid, ProcessSerialNumber* psn)
{
	psn->highLongOfPSN = 0;
	psn->lowLongOfPSN = pid;
	return noErr;
}

OSErr GetProcessInformation(const ProcessSerialNumber* psn, ProcessInfoRec* pi)
{
	STUB();
	unsigned long len = pi->processInfoLength;
	
	memset(pi, 0, len);
	pi->processInfoLength = len;
	
	return noErr;
}

OSStatus GetProcessPID(const ProcessSerialNumber* psn, pid_t* pid)
{
	*pid = psn->lowLongOfPSN;
	return noErr;
}

Boolean IsProcessVisible(const ProcessSerialNumber* psn)
{
	STUB();
	return true;
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
	char procpath[PATH_MAX], lpath[100];
	int len;
	
	status = CopyProcessName(PSN, &name);
	if (status != noErr)
		return nullptr;

	sprintf(lpath, "/proc/%u/exe", (unsigned int)PSN->lowLongOfPSN);
	len = ::readlink(lpath, procpath, sizeof(procpath)-1);
	if (len < 0)
		return nullptr;

	procpath[len] = '\0';
	
	dict = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	pid = CFNumberCreate(nullptr, kCFNumberLongType, &PSN->lowLongOfPSN);
	path = CFStringCreateWithCString(nullptr, procpath, kCFStringEncodingUTF8);
	
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
	*result = psn1->lowLongOfPSN == psn2->highLongOfPSN;
	return noErr;
}

OSErr SetFrontProcess(const ProcessSerialNumber* psn)
{
	STUB();
	return noErr;
}

OSErr SetFrontProcessWithOptions(const ProcessSerialNumber* psn, OptionBits opts)
{
	STUB();
	return noErr;
}

OSErr ShowHideProcess(const ProcessSerialNumber* psn, Boolean visible)
{
	STUB();
	return noErr;
}

OSStatus TransformProcessType(const ProcessSerialNumber* psn, UInt32 type)
{
	STUB();
	return noErr;
}

OSErr WakeUpProcess(const ProcessSerialNumber* psn)
{
	STUB();
	return noErr;
}

void DebugStr(ConstStr255Param msg)
{
	std::cerr.write((const char*) &msg[1], msg[0]);
	std::cerr << std::endl;
	doUserBreak();
}
