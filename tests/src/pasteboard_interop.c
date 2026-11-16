// Copyright (C) 2026 Lubos Dolezel
//
// The Pasteboard Manager (PasteboardCreate and the other Pasteboard* calls) on its own: names, change tracking with
// PasteboardSynchronize, items and flavors, the errors for bad indices, items, flavors, stale references and missing
// ownership, flavor flags, promises with a promise keeper, and a change made by another process.
//
// Expected results were observed with the same program on macOS 26.7.3 and 10.14.4 (x86_64 and i386). Where the two
// differ the test accepts both and says so. macOS adds translated flavors (UTF-16 and legacy text for UTF-8 text), so
// flavor lists are only checked for what must be in them.
//
// Cross-process part: the program starts itself with "write <board>" to change a board from another process; on
// Darling that goes through the pboard service.
#include <ApplicationServices/ApplicationServices.h>
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
static CFDataRef bytes(const char *s) { return CFDataCreate(NULL, (const UInt8 *)s, (CFIndex)strlen(s)); }
static int dataIs(CFDataRef d, const char *s)
{
	return d && CFDataGetLength(d) == (CFIndex)strlen(s) && memcmp(CFDataGetBytePtr(d), s, strlen(s)) == 0;
}

static CFStringRef const kText = CFSTR("public.utf8-plain-text");
static CFStringRef const kHTML = CFSTR("public.html");

static int hasFlavor(PasteboardRef pb, PasteboardItemID item, CFStringRef type)
{
	CFArrayRef flavors = NULL;
	if (PasteboardCopyItemFlavors(pb, item, &flavors) != noErr) return 0;
	int found = CFArrayContainsValue(flavors, CFRangeMake(0, CFArrayGetCount(flavors)), type);
	CFRelease(flavors);
	return found;
}

static void testNames(void)
{
	PasteboardRef clip = NULL, find = NULL, general = NULL, a = NULL, b = NULL;
	CFStringRef name = NULL;
	check("create clipboard", PasteboardCreate(kPasteboardClipboard, &clip) == noErr && clip);
	check("clipboard name", PasteboardCopyName(clip, &name) == noErr && CFEqual(name, CFSTR("Apple CFPasteboard general")));
	check("create find", PasteboardCreate(kPasteboardFind, &find) == noErr);
	PasteboardCopyName(find, &name);
	check("find name", CFEqual(name, CFSTR("Apple CFPasteboard find")));
	check("the CoreFoundation name is the same board", PasteboardCreate(CFSTR("Apple CFPasteboard general"), &general) == noErr);
	PasteboardCopyName(general, &name);
	check("and reports the same name", CFEqual(name, CFSTR("Apple CFPasteboard general")));
	check("unique boards", PasteboardCreate(kPasteboardUniqueName, &a) == noErr && PasteboardCreate(NULL, &b) == noErr);
	CFStringRef an = NULL, bn = NULL;
	PasteboardCopyName(a, &an);
	PasteboardCopyName(b, &bn);
	check("unique names differ and are prefixed", !CFEqual(an, bn) && CFStringHasPrefix(an, CFSTR("CFPasteboardUnique-")));
	check("type id", PasteboardGetTypeID() != 0 && CFGetTypeID(a) == PasteboardGetTypeID());

	// the clipboard and the find board are different boards
	PasteboardRef pb = NULL;
	PasteboardCreate(CFSTR("test.pasteboard-interop.names"), &pb);
	PasteboardClear(pb);
	PasteboardPutItemFlavor(pb, (PasteboardItemID)1, kText, (CFDataRef)bytes("named"), 0);
	PasteboardRef again = NULL;
	PasteboardCreate(CFSTR("test.pasteboard-interop.names"), &again);
	ItemCount n = 0;
	CFDataRef d = NULL;
	PasteboardItemID id = 0;
	check("another reference to a named board sees it", PasteboardGetItemCount(again, &n) == noErr && n == 1
		&& PasteboardGetItemIdentifier(again, 1, &id) == noErr
		&& PasteboardCopyItemFlavorData(again, id, kText, &d) == noErr && dataIs(d, "named"));
	PasteboardRef other = NULL;
	PasteboardCreate(CFSTR("test.pasteboard-interop.other"), &other);
	check("another name is another board", PasteboardGetItemCount(other, &n) == noErr && n == 0);
}

static void testRoundTrip(void)
{
	PasteboardRef pb = NULL;
	PasteboardCreate(kPasteboardUniqueName, &pb);
	check("clear", PasteboardClear(pb) == noErr);
	check("put text", PasteboardPutItemFlavor(pb, (PasteboardItemID)1, kText, (CFDataRef)bytes("hello"), kPasteboardFlavorNoFlags) == noErr);
	check("put html with flags", PasteboardPutItemFlavor(pb, (PasteboardItemID)1, kHTML, (CFDataRef)bytes("<b>x</b>"), kPasteboardFlavorSenderOnly | kPasteboardFlavorNotSaved) == noErr);
	check("synchronize reports the owner", (PasteboardSynchronize(pb) & kPasteboardClientIsOwner) != 0);

	ItemCount n = 0;
	PasteboardItemID id = 0;
	CFDataRef d = NULL;
	PasteboardFlavorFlags flags = 99;
	check("one item", PasteboardGetItemCount(pb, &n) == noErr && n == 1);
	check("item identifier", PasteboardGetItemIdentifier(pb, 1, &id) == noErr);
	check("flavors include both", hasFlavor(pb, id, kText) && hasFlavor(pb, id, kHTML));
	check("text data", PasteboardCopyItemFlavorData(pb, id, kText, &d) == noErr && dataIs(d, "hello"));
	check("plain flags", PasteboardGetItemFlavorFlags(pb, id, kText, &flags) == noErr && flags == 0);
	check("html flags are kept", PasteboardGetItemFlavorFlags(pb, id, kHTML, &flags) == noErr && flags == (kPasteboardFlavorSenderOnly | kPasteboardFlavorNotSaved));

	// errors
	PasteboardItemID bad = 0;
	check("index 0", PasteboardGetItemIdentifier(pb, 0, &bad) == badPasteboardIndexErr);
	check("index past the end", PasteboardGetItemIdentifier(pb, 2, &bad) == badPasteboardIndexErr);
	CFArrayRef flavors = NULL;
	check("unknown item flavors", PasteboardCopyItemFlavors(pb, (PasteboardItemID)12345, &flavors) == badPasteboardItemErr);
	check("unknown item data", PasteboardCopyItemFlavorData(pb, (PasteboardItemID)12345, kText, &d) == badPasteboardItemErr);
	check("unknown flavor", PasteboardCopyItemFlavorData(pb, id, CFSTR("public.png"), &d) == badPasteboardFlavorErr);
	check("unknown flavor flags", PasteboardGetItemFlavorFlags(pb, id, CFSTR("public.png"), &flags) == badPasteboardFlavorErr);
	check("duplicate flavor", PasteboardPutItemFlavor(pb, id, kText, (CFDataRef)bytes("again"), 0) == duplicatePasteboardFlavorErr);

	// a second item with a caller-chosen identifier
	check("second item", PasteboardPutItemFlavor(pb, (PasteboardItemID)2, kText, (CFDataRef)bytes("two"), 0) == noErr);
	PasteboardSynchronize(pb);
	PasteboardItemID first = 0, second = 0;
	check("two items", PasteboardGetItemCount(pb, &n) == noErr && n == 2);
	PasteboardGetItemIdentifier(pb, 1, &first);
	PasteboardGetItemIdentifier(pb, 2, &second);
	check("identifiers are distinct and stable", first != second && first == id);
	PasteboardSynchronize(pb);
	PasteboardItemID again = 0;
	PasteboardGetItemIdentifier(pb, 1, &again);
	check("stable across synchronizations", again == first);
}

static void testSynchronize(void)
{
	PasteboardRef a = NULL, b = NULL;
	PasteboardCreate(CFSTR("test.pasteboard-interop.sync"), &a);
	PasteboardCreate(CFSTR("test.pasteboard-interop.sync"), &b);
	PasteboardSynchronize(a);
	PasteboardSynchronize(b);
	check("nothing changed: no flags", PasteboardSynchronize(a) == 0);
	check("b clears", PasteboardClear(b) == noErr);
	check("b is the owner, not modified", PasteboardSynchronize(b) == kPasteboardClientIsOwner);

	// a has not synchronized: reads are refused, putting is refused with the stale error (or, on newer macOS, the owner error)
	ItemCount n = 0;
	PasteboardItemID id = 0;
	CFDataRef d = NULL;
	CFArrayRef flavors = NULL;
	PasteboardFlavorFlags flags;
	// (a change made by a Clear in this process does not make a reference's reads fail; putting does)
	// (10.14 allows it, 26 returns badPasteboardSyncErr)
	OSStatus read = PasteboardGetItemCount(a, &n);
	check("reads after a clear in this process", read == noErr || read == badPasteboardSyncErr);
	OSStatus put = PasteboardPutItemFlavor(a, (PasteboardItemID)1, kText, (CFDataRef)bytes("x"), 0);
	check("stale: put (10.14 badPasteboardSyncErr, 26 notPasteboardOwnerErr)", put == badPasteboardSyncErr || put == notPasteboardOwnerErr);
	PasteboardSyncFlags seen = PasteboardSynchronize(a);
	check("a sees the change once (26: the failed put already synchronized)", seen == kPasteboardModified || seen == 0);
	// b's clear was made in this process: the process owns the contents, which a reports from now on
	check("then the process is the owner", PasteboardSynchronize(a) == kPasteboardClientIsOwner);
	check("synchronized: item count works", PasteboardGetItemCount(a, &n) == noErr && n == 0);
	check("so a may put into what b cleared", PasteboardPutItemFlavor(a, (PasteboardItemID)1, kText, (CFDataRef)bytes("x"), 0) == noErr);
	check("b is still the owner", PasteboardSynchronize(b) == kPasteboardClientIsOwner);
	check("a clears: a is the owner", PasteboardClear(a) == noErr && PasteboardSynchronize(a) == kPasteboardClientIsOwner);
}

static int helper(const char *board)
{
	char *argv[] = { (char *)self, "write", (char *)board, NULL };
	pid_t pid;
	if (posix_spawn(&pid, self, NULL, NULL, argv, environ) != 0) return -1;
	int status;
	waitpid(pid, &status, 0);
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

static void testOtherProcess(void)
{
	char name[100];
	snprintf(name, sizeof(name), "test.pasteboard-interop.process.%d", (int)getpid());
	CFStringRef n = str(name);
	PasteboardRef pb = NULL;
	PasteboardCreate(n, &pb);
	PasteboardSynchronize(pb);
	check("another process writes", helper(name) == 0);
	// a change by another process makes reads fail until the reference is synchronized
	ItemCount early = 0;
	PasteboardItemID earlyItem = 0;
	CFArrayRef earlyFlavors = NULL;
	CFDataRef earlyData = NULL;
	PasteboardFlavorFlags earlyFlags;
	check("stale: item count", PasteboardGetItemCount(pb, &early) == badPasteboardSyncErr);
	check("stale: item identifier", PasteboardGetItemIdentifier(pb, 1, &earlyItem) == badPasteboardSyncErr);
	check("stale: flavors", PasteboardCopyItemFlavors(pb, (PasteboardItemID)1, &earlyFlavors) == badPasteboardSyncErr);
	check("stale: data", PasteboardCopyItemFlavorData(pb, (PasteboardItemID)1, kText, &earlyData) == badPasteboardSyncErr);
	check("stale: flags", PasteboardGetItemFlavorFlags(pb, (PasteboardItemID)1, kText, &earlyFlags) == badPasteboardSyncErr);
	check("the change is reported", PasteboardSynchronize(pb) == kPasteboardModified);
	check("then nothing", PasteboardSynchronize(pb) == 0);
	ItemCount count = 0;
	PasteboardItemID id = 0;
	CFDataRef d = NULL;
	check("and readable", PasteboardGetItemCount(pb, &count) == noErr && count == 1
		&& PasteboardGetItemIdentifier(pb, 1, &id) == noErr && PasteboardCopyItemFlavorData(pb, id, kText, &d) == noErr && dataIs(d, "from-other-process"));
	check("not the owner of another process's contents", PasteboardPutItemFlavor(pb, id, kHTML, (CFDataRef)bytes("x"), 0) == notPasteboardOwnerErr);
}

// promise keeper
static int keeperCalls;
static OSStatus keeper(PasteboardRef pb, PasteboardItemID item, CFStringRef type, void *context)
{
	keeperCalls++;
	CFDataRef d = bytes((const char *)context);
	OSStatus err = PasteboardPutItemFlavor(pb, item, type, d, 0);
	CFRelease(d);
	return err;
}

static void testPromises(void)
{
	PasteboardRef pb = NULL;
	PasteboardCreate(kPasteboardUniqueName, &pb);
	PasteboardClear(pb);
	check("promise without a keeper", PasteboardPutItemFlavor(pb, (PasteboardItemID)1, kText, kPasteboardPromisedData, 0) == noPasteboardPromiseKeeperErr);
	check("set keeper", PasteboardSetPromiseKeeper(pb, keeper, "kept") == noErr);
	check("promise text", PasteboardPutItemFlavor(pb, (PasteboardItemID)1, kText, kPasteboardPromisedData, 0) == noErr);
	check("promise html", PasteboardPutItemFlavor(pb, (PasteboardItemID)1, kHTML, kPasteboardPromisedData, 0) == noErr);
	PasteboardSynchronize(pb);
	PasteboardFlavorFlags flags = 0;
	check("flagged as promised", PasteboardGetItemFlavorFlags(pb, (PasteboardItemID)1, kText, &flags) == noErr && (flags & kPasteboardFlavorPromised));
	check("not asked yet", keeperCalls == 0);
	CFDataRef d = NULL;
	check("reading asks the keeper", PasteboardCopyItemFlavorData(pb, (PasteboardItemID)1, kText, &d) == noErr && dataIs(d, "kept") && keeperCalls == 1);
	check("no longer promised", PasteboardGetItemFlavorFlags(pb, (PasteboardItemID)1, kText, &flags) == noErr && !(flags & kPasteboardFlavorPromised));
	check("read again: not asked again", PasteboardCopyItemFlavorData(pb, (PasteboardItemID)1, kText, &d) == noErr && keeperCalls == 1);
	check("resolve promises", PasteboardResolvePromises(pb) == noErr && keeperCalls == 2);
	check("html resolved", PasteboardCopyItemFlavorData(pb, (PasteboardItemID)1, kHTML, &d) == noErr && dataIs(d, "kept") && keeperCalls == 2);
}

int main(int argc, char **argv)
{
	self = argv[0];
	if (argc == 3 && !strcmp(argv[1], "write")) {
		CFStringRef n = str(argv[2]);
		PasteboardRef pb = NULL;
		if (PasteboardCreate(n, &pb) != noErr) return 2;
		if (PasteboardClear(pb) != noErr) return 3;
		CFDataRef d = bytes("from-other-process");
		return PasteboardPutItemFlavor(pb, (PasteboardItemID)1, kText, d, 0) == noErr ? 0 : 4;
	}
	testNames();
	testRoundTrip();
	testSynchronize();
	testOtherProcess();
	testPromises();
	printf(failures ? "FAILED\n" : "ALL PASSED\n");
	return failures ? 1 : 0;
}
