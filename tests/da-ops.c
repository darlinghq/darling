// Copyright (C) 2026 Lubos Dolezel
// DiskArbitration unmount, eject and mount requests, approval callbacks and dissenters (see diskarbitration.md):
//  unmount <BSD name> <options>    DADiskUnmount; prints "status <hex>" and, if there is one, "string <text>"
//  eject <BSD name>                DADiskEject; prints the same
//  mount <BSD name>                DADiskMount at the default place; prints the same
//  busy-file <mount point>         unmounts the volume's leaf (no options) while a file in it is open
//  describe <BSD name>             prints "present" if the disk has a description, otherwise "absent"
//  no-daemon <BSD name>            for when diskarbitrationd cannot be reached: looks up "/" and the disk, asks
//                                  to unmount it, releases everything, and prints what it got (Darling only)
//  deny <seconds>                  for that long, dissents from every unmount and eject with kDAReturnBusy and
//                                  "in use by test", printing "approval asked for <BSD name>"
//  hang <seconds>                  for that long, answers every unmount and eject approval only after 30 s
// With "--uid <uid>" first, the program runs as that user (it must start as root); it exits with 3 if it cannot.
// The request modes exit with 0 once the completion callback has run (whatever its status), 1 if it did not run
// within 60 s. Link with -framework DiskArbitration -framework CoreFoundation. The program also runs on macOS.
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int done;

static void completed(DADiskRef disk, DADissenterRef dissenter, void* context)
{
	printf("status %x\n", dissenter ? (unsigned) DADissenterGetStatus(dissenter) : 0);
	CFStringRef string = dissenter ? DADissenterGetStatusString(dissenter) : NULL;
	if (string != NULL)
	{
		char text[256];
		if (CFStringGetCString(string, text, sizeof(text), kCFStringEncodingUTF8))
			printf("string %s\n", text);
	}
	fflush(stdout);
	done = 1;
}

static DADissenterRef deny(DADiskRef disk, void* context)
{
	printf("approval asked for %s\n", DADiskGetBSDName(disk));
	fflush(stdout);
	return DADissenterCreate(NULL, kDAReturnBusy, CFSTR("in use by test"));
}

static DADissenterRef hang(DADiskRef disk, void* context)
{
	printf("approval asked for %s, answering in 30 s\n", DADiskGetBSDName(disk));
	fflush(stdout);
	sleep(30);
	return NULL;
}

static int wait_done(void)
{
	for (int i = 0; i < 600 && !done; i++)
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, true);
	return done ? 0 : 1;
}

static DADiskRef leaf_for(DASessionRef session, const char* mp)
{
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) mp, strlen(mp), true);
	DADiskRef disk = DADiskCreateFromVolumePath(NULL, session, url);
	CFRelease(url);
	return disk;
}

int main(int argc, char** argv)
{
	if (argc > 2 && strcmp(argv[1], "--uid") == 0)
	{
		if (setuid((uid_t) atoi(argv[2])) != 0 || geteuid() != (uid_t) atoi(argv[2]))
		{
			printf("cannot change uid\n");
			return 3;
		}
		argc -= 2;
		argv += 2;
	}

	DASessionRef session = DASessionCreate(NULL);
	if (session == NULL || argc < 3)
	{
		fprintf(stderr, "usage: da-ops [--uid <uid>] unmount <name> <options> | eject <name> | mount <name> | busy-file <mp>"
			" | describe <name> | no-daemon <name> | deny <seconds> | hang <seconds>\n");
		return 2;
	}
	DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	const char* mode = argv[1];
	if (strcmp(mode, "deny") == 0 || strcmp(mode, "hang") == 0)
	{
		DADiskUnmountApprovalCallback callback = strcmp(mode, "deny") == 0 ? deny : hang;
		DARegisterDiskUnmountApprovalCallback(session, NULL, callback, NULL);
		DARegisterDiskEjectApprovalCallback(session, NULL, (DADiskEjectApprovalCallback) callback, NULL);
		printf("ready\n");
		fflush(stdout);
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, atof(argv[2]), false);
		return 0;
	}

	if (strcmp(mode, "no-daemon") == 0)
	{
		CFURLRef root = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) "/", 1, true);
		DADiskRef rootDisk = DADiskCreateFromVolumePath(NULL, session, root);
		printf("root volume: %s\n", rootDisk ? "a disk" : "none");
		DADiskRef disk = DADiskCreateFromBSDName(NULL, session, argv[2]);
		CFDictionaryRef description = disk ? DADiskCopyDescription(disk) : NULL;
		printf("description: %s\n", description ? "present" : "absent");
		if (disk != NULL)
		{
			DADiskUnmount(disk, kDADiskUnmountOptionDefault, completed, NULL);
			wait_done();
			CFRelease(disk);
		}
		if (description != NULL)
			CFRelease(description);
		if (rootDisk != NULL)
			CFRelease(rootDisk);
		CFRelease(root);
		DASessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		CFRelease(session);
		printf("released\n");
		return done ? 0 : 1;
	}

	if (strcmp(mode, "describe") == 0)
	{
		DADiskRef disk = DADiskCreateFromBSDName(NULL, session, argv[2]);
		CFDictionaryRef description = disk ? DADiskCopyDescription(disk) : NULL;
		printf("%s\n", description ? "present" : "absent");
		if (description != NULL)
			CFRelease(description);
		return 0;
	}

	DADiskRef disk = strcmp(mode, "busy-file") == 0 ? leaf_for(session, argv[2])
		: DADiskCreateFromBSDName(NULL, session, argv[2]);
	if (disk == NULL)
	{
		printf("no disk\n");
		return 1;
	}

	int fd = -1;
	if (strcmp(mode, "unmount") == 0 && argc == 4)
	{
		DADiskUnmount(disk, (DADiskUnmountOptions) strtoul(argv[3], NULL, 0), completed, NULL);
	}
	else if (strcmp(mode, "eject") == 0)
	{
		DADiskEject(disk, kDADiskEjectOptionDefault, completed, NULL);
	}
	else if (strcmp(mode, "mount") == 0)
	{
		DADiskMount(disk, NULL, kDADiskMountOptionDefault, completed, NULL);
	}
	else if (strcmp(mode, "busy-file") == 0)
	{
		char path[1024];
		snprintf(path, sizeof(path), "%s/hello.txt", argv[2]);
		fd = open(path, O_RDONLY);
		if (fd < 0)
		{
			printf("cannot open %s\n", path);
			return 1;
		}
		DADiskUnmount(disk, kDADiskUnmountOptionDefault, completed, NULL);
	}
	else
	{
		fprintf(stderr, "unknown mode %s\n", mode);
		return 2;
	}

	int result = wait_done();
	if (fd >= 0)
		close(fd);
	CFRelease(disk);
	DASessionUnscheduleFromRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	CFRelease(session);
	return result;
}
