/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Guest test: /usr/lib/libDiskUnlock.dylib loads and DUUnlockVolume can be
 * called with an object argument without crashing. Link with
 * -L/usr/lib -lDiskUnlock (or the .dylib path).
 */
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>

void DUUnlockVolume(void *volumeName);

int main(void)
{
	DUUnlockVolume((void *)CFSTR("Macintosh HD"));
	DUUnlockVolume(NULL);
	printf("PASS\n");
	return 0;
}
