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


// QDFlushPortBuffer announces the flushed part of a port to the local
// notification center, so whatever shows the port on screen can pick it up.
// Darling-specific: this is how the layers above QuickDraw are told, and a
// real Mac does not post it. 32-bit only, like the port API.

#include <QD/QD.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>

#ifdef __LP64__

int main(void)
{
	printf("SKIP: the port API is 32-bit only\n");
	return 0;
}

#else

static int failures = 0;
static int notifications = 0;
static const void* lastObject;
static short lastRect[4];
static int lastHadRect;

static void check(const char* what, int ok)
{
	if (!ok) {
		printf("FAIL: %s\n", what);
		failures++;
	}
}

static void observe(CFNotificationCenterRef center, void* observer, CFNotificationName name,
	const void* object, CFDictionaryRef userInfo)
{
	notifications++;
	lastObject = object;
	lastHadRect = userInfo != NULL;
	if (userInfo != NULL) {
		CFStringRef keys[4] = {CFSTR("top"), CFSTR("left"), CFSTR("bottom"), CFSTR("right")};
		for (int i = 0; i < 4; i++) {
			CFNumberRef n = CFDictionaryGetValue(userInfo, keys[i]);
			lastRect[i] = -1;
			if (n != NULL)
				CFNumberGetValue(n, kCFNumberShortType, &lastRect[i]);
		}
	}
}

int main(void)
{
	Rect rect = {0, 0, 30, 50};
	GWorldPtr first = NULL, second = NULL;
	check("NewGWorld", NewGWorld(&first, 32, &rect, NULL, NULL, 0) == noErr && NewGWorld(&second, 32, &rect, NULL, NULL, 0) == noErr);

	// Observe the flushes of the first port only.
	static int token;
	CFNotificationCenterAddObserver(CFNotificationCenterGetLocalCenter(), &token, observe,
		CFSTR("com.darlinghq.QD.PortFlushed"), first, CFNotificationSuspensionBehaviorDeliverImmediately);

	QDFlushPortBuffer(first, NULL);
	check("flush of the whole port is announced without a rectangle", notifications == 1 && lastObject == first && !lastHadRect);

	QDFlushPortBuffer(second, NULL);
	check("a flush of another port is not announced to this observer", notifications == 1);

	RgnHandle part = NewRgn();
	SetRectRgn(part, 5, 6, 25, 20);
	QDFlushPortBuffer(first, part);
	check("flush of a region is announced with its bounds",
		notifications == 2 && lastHadRect && lastRect[0] == 6 && lastRect[1] == 5 && lastRect[2] == 20 && lastRect[3] == 25);

	SetEmptyRgn(part);
	QDFlushPortBuffer(first, part);
	check("an empty region has nothing to flush", notifications == 2);
	DisposeRgn(part);

	QDFlushPortBuffer(NULL, NULL);
	check("flushing no port does nothing", notifications == 2);

	CFNotificationCenterRemoveEveryObserver(CFNotificationCenterGetLocalCenter(), &token);
	DisposeGWorld(first);
	DisposeGWorld(second);

	if (failures == 0)
		printf("PASS\n");
	return failures != 0;
}

#endif
