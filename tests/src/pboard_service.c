// Copyright (C) 2026 Lubos Dolezel
//
// The per-user pasteboard service: boards written by one process are seen by another, the find board is shared,
// unique boards are not, a promise made in one process is kept when another reads it, a new generation by another
// process is noticed, and the service does not need a display. Uses CoreFoundation's private pasteboard interface
// (header CoreFoundation/CFPasteboard_Private.h). The test fails if the service cannot be reached: without it the
// boards would live in each process and nothing could be shared.
//
// Roles: no argument starts the test; "write NAME TEXT" and "read NAME TEXT" and "readpromise NAME TEXT" are the
// helper processes the test starts from itself.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPasteboard_Private.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;
static int failures;
static const char *self;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

static CFStringRef str(const char *s) { return CFStringCreateWithCString(NULL, s, kCFStringEncodingUTF8); }
static CFStringRef const kText = CFSTR("public.utf8-plain-text");
static CFStringRef const kPNG = CFSTR("public.png");

static int helper(const char *role, const char *name, const char *text)
{
	char *argv[] = { (char *)self, (char *)role, (char *)name, (char *)text, NULL };
	pid_t pid;
	if (posix_spawn(&pid, self, NULL, NULL, argv, environ) != 0) return -1;
	int status;
	waitpid(pid, &status, 0);
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static int doWrite(const char *name, const char *text)
{
	CFStringRef n = str(name);
	CFPasteboardRef pb = CFPasteboardCreate(NULL, n);
	if (!pb) return 2;
	CFPasteboardBeginGeneration(pb);
	CFPasteboardItemID item;
	CFPasteboardAddItem(pb, &item);
	CFDataRef data = CFDataCreate(NULL, (const UInt8 *)text, (CFIndex)strlen(text));
	int ok = CFPasteboardSetData(pb, item, kText, data, 0) == kCFPasteboardSuccess;
	return ok ? 0 : 3;
}

static int doRead(const char *name, const char *text)
{
	CFStringRef n = str(name);
	CFPasteboardRef pb = CFPasteboardCreate(NULL, n);
	CFIndex count = 0;
	CFPasteboardItemID item;
	CFDataRef data = NULL;
	if (!pb || CFPasteboardGetItemCount(pb, &count) != kCFPasteboardSuccess || count != 1) return 2;
	if (CFPasteboardGetItemAtIndex(pb, 0, &item) != kCFPasteboardSuccess) return 3;
	if (CFPasteboardCopyData(pb, item, kText, &data) != kCFPasteboardSuccess || !data) return 4;
	return CFDataGetLength(data) == (CFIndex)strlen(text) && memcmp(CFDataGetBytePtr(data), text, strlen(text)) == 0 ? 0 : 5;
}

// the promise keeper of the main process
static int kept;
static CFPasteboardStatus provide(CFPasteboardRef pb, CFPasteboardItemID item, CFStringRef flavor, void *info)
{
	kept++;
	CFDataRef data = CFDataCreate(NULL, (const UInt8 *)"promised-png", 12);
	CFPasteboardStatus s = CFPasteboardSetData(pb, item, flavor, data, 0);
	CFRelease(data);
	return s;
}
static int ownerChanged;
static void changed(CFPasteboardRef pb, void *info) { ownerChanged++; }

static int doReadPromise(const char *name, const char *text)
{
	CFStringRef n = str(name);
	CFPasteboardRef pb = CFPasteboardCreate(NULL, n);
	CFPasteboardItemID item;
	CFDataRef data = NULL;
	if (!pb || CFPasteboardGetItemAtIndex(pb, 0, &item) != kCFPasteboardSuccess) return 2;
	if (!CFPasteboardIsDataPromised(pb, item, kPNG)) return 3;
	if (CFPasteboardCopyData(pb, item, kPNG, &data) != kCFPasteboardSuccess || !data) return 4;
	return CFDataGetLength(data) == 12 && memcmp(CFDataGetBytePtr(data), "promised-png", 12) == 0 ? 0 : 5;
}

int main(int argc, char **argv)
{
	self = argv[0];
	if (argc == 4) {
		if (!strcmp(argv[1], "write")) return doWrite(argv[2], argv[3]);
		if (!strcmp(argv[1], "read")) return doRead(argv[2], argv[3]);
		if (!strcmp(argv[1], "readpromise")) return doReadPromise(argv[2], argv[3]);
		if (!strcmp(argv[1], "clear")) {
			CFStringRef n = str(argv[2]);
			CFPasteboardRef pb = CFPasteboardCreate(NULL, n);
			return pb && CFPasteboardBeginGeneration(pb) ? 0 : 2;
		}
		return 64;
	}

	// a name that no earlier run used
	char name[100], findText[64];
	snprintf(name, sizeof(name), "test.pboard-service.%d", (int)getpid());
	CFStringRef n = str(name);

	// written by a child, read here
	check("child writes a named board", helper("write", name, "from-child") == 0);
	CFPasteboardRef pb = CFPasteboardCreate(NULL, n);
	CFIndex count = 0;
	CFPasteboardItemID item = 0;
	CFDataRef data = NULL;
	check("the parent sees the child's item", pb && CFPasteboardGetItemCount(pb, &count) == kCFPasteboardSuccess && count == 1);
	check("and its data", pb && CFPasteboardGetItemAtIndex(pb, 0, &item) == kCFPasteboardSuccess
		&& CFPasteboardCopyData(pb, item, kText, &data) == kCFPasteboardSuccess && data && CFDataGetLength(data) == 10
		&& memcmp(CFDataGetBytePtr(data), "from-child", 10) == 0);

	// the parent may not add to what the child owns, and a change count moves when the child replaces the contents
	CFIndex before = CFPasteboardGetChangeCount(pb);
	check("not the owner of the child's generation", CFPasteboardSetData(pb, item, CFSTR("public.html"), NULL, 0) == kCFPasteboardNotOwner);
	check("child replaces the board", helper("write", name, "second") == 0);
	check("change count moved", CFPasteboardGetChangeCount(pb) == before + 1);

	// written here, read by a child
	CFPasteboardBeginGeneration(pb);
	CFPasteboardAddItem(pb, &item);
	CFDataRef mine = CFDataCreate(NULL, (const UInt8 *)"from-parent", 11);
	check("parent writes", CFPasteboardSetData(pb, item, kText, mine, 0) == kCFPasteboardSuccess);
	check("child reads the parent's data", helper("read", name, "from-parent") == 0);

	// promise kept across processes
	CFPasteboardBeginGeneration(pb);
	CFPasteboardAddItem(pb, &item);
	CFPasteboardProviderCallbacks provider = { 0, provide, NULL, changed, NULL };
	check("parent promises a flavor", CFPasteboardPromiseData(pb, item, kPNG, 0, &provider, NULL) == kCFPasteboardSuccess);
	check("child's read resolves the promise in the parent", helper("readpromise", name, "") == 0 && kept == 1);
	check("child takes the board over and the parent is told", helper("clear", name, "") == 0 && (usleep(200000), ownerChanged == 1));

	// the find board is one board for everybody; a unique board is nobody else's
	snprintf(findText, sizeof(findText), "find-%d", (int)getpid());
	CFStringRef find = CFPasteboardCopyName(CFPasteboardCreate(NULL, kCFPasteboardFind));
	check("find board name", CFEqual(find, CFSTR("Apple CFPasteboard find")));
	check("child writes the find board", helper("write", "Apple CFPasteboard find", findText) == 0);
	check("parent reads the find board", helper("read", "Apple CFPasteboard find", findText) == 0);
	CFPasteboardRef unique = CFPasteboardCreateUnique(NULL);
	CFStringRef uname = CFPasteboardCopyName(unique);
	char uniqueName[100];
	CFStringGetCString(uname, uniqueName, sizeof(uniqueName), kCFStringEncodingUTF8);
	CFPasteboardBeginGeneration(unique);
	check("a unique board holds nothing for a child", helper("read", uniqueName, "x") != 0);

	printf(failures ? "FAILED\n" : "ALL PASSED\n");
	return failures ? 1 : 0;
}
