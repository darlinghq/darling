#ifndef PROCESSES_H
#define PROCESSES_H
#include "MacTypes.h"
#include "FileManager.h"
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFDictionary.h>
#include <sys/types.h>

struct ProcessInfoRec
{
	unsigned long processInfoLength;
	// fields omitted
};

struct LaunchParamBlockRec
{
	// fields omitted
};

enum { kNoProcess = 0, kSystemProcess = 1, kCurrentProcess = 2 };
enum { procNotFound = -600 };

extern "C" {

OSStatus CopyProcessName(const ProcessSerialNumber* psn, CFStringRef* name);
void ExitToShell();
OSErr GetCurrentProcess(ProcessSerialNumber* psn);
OSErr GetFrontProcess(ProcessSerialNumber* psn);
OSErr GetNextProcess(ProcessSerialNumber* psn);
OSStatus GetProcessBundleLocation(const ProcessSerialNumber* psn, FSRef* location);
OSStatus GetProcessForPID(pid_t pid, ProcessSerialNumber* psn);
OSErr GetProcessInformation(const ProcessSerialNumber* psn, ProcessInfoRec* pi);
OSStatus GetProcessPID(const ProcessSerialNumber* psn, pid_t* pid);
Boolean IsProcessVisible(const ProcessSerialNumber* psn);
OSErr KillProcess(const ProcessSerialNumber* psn);
OSErr LaunchApplication(LaunchParamBlockRec* launch);
CFDictionaryRef ProcessInformationCopyDictionary(const ProcessSerialNumber *PSN, UInt32 infoToReturn);
OSErr SameProcess(const ProcessSerialNumber* psn1, const ProcessSerialNumber* psn2, Boolean* result);
OSErr SetFrontProcess(const ProcessSerialNumber* psn);
OSErr SetFrontProcessWithOptions(const ProcessSerialNumber* psn, OptionBits opts);
OSErr ShowHideProcess(const ProcessSerialNumber* psn, Boolean visible);
OSStatus TransformProcessType(const ProcessSerialNumber* psn, UInt32 type);
OSErr WakeUpProcess(const ProcessSerialNumber* psn);

// extern CFStringRef kCFBundleExecutableKey;
// extern CFStringRef kCFBundleNameKey;
// extern CFStringRef kCFBundleIdentifierKey;

}

#endif
