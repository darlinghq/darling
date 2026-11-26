// Copyright (C) 2026 Lubos Dolezel
//
// CVBuffer attachment functions: the last release of an attachment value runs its finalizer, which may
// call attachment functions again (on this or another buffer). That must not deadlock, and a value that
// the caller does not own (the result of CVBufferGetAttachment, passed back in with the other mode)
// must survive the move. Also exercised through the equivalent CoreMedia calls on a CVPixelBuffer.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;
static CVBufferRef g_buffer;
static int g_finalized;
static int g_reentered;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out (attachment lock held while a value was released)\n");
	fflush(stdout);
	_exit(2);
}

// A token is a one-entry dictionary whose value callbacks run code when the token is destroyed, which is how a
// plain Core Foundation client gets a finalizer.
static void tokenRelease(CFAllocatorRef allocator, const void* value)
{
	(void) allocator;
	(void) value;
	g_finalized++;
	// Re-enter the attachment functions of the same buffer from the finalizer.
	CVAttachmentMode mode;
	(void) CVBufferGetAttachment(g_buffer, CFSTR("reentrant.probe"), &mode);
	CVBufferSetAttachment(g_buffer, CFSTR("reentrant.other"), CFSTR("x"), kCVAttachmentMode_ShouldNotPropagate);
	g_reentered++;
}

static CFTypeRef makeToken(void)
{
	static const CFDictionaryValueCallBacks callbacks = { 0, NULL, tokenRelease, NULL, NULL };
	const void* key = CFSTR("token");
	const void* value = (const void*) 1;
	return CFDictionaryCreate(NULL, &key, &value, 1, &kCFTypeDictionaryKeyCallBacks, &callbacks);
}

static CFStringRef k = CFSTR("reentrant.probe");

// Each case attaches a token that only the buffer owns, then drops it in a different way.
static void expectDroppedAndReentered(const char* what, int expectedFinalized)
{
	(void) what;
	CHECK(g_finalized == expectedFinalized);
	CHECK(g_reentered == expectedFinalized);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(20);

	CVPixelBufferRef pixels = NULL;
	CHECK(CVPixelBufferCreate(NULL, 4, 4, kCVPixelFormatType_32BGRA, NULL, &pixels) == kCVReturnSuccess);
	g_buffer = (CVBufferRef) pixels;

	CFTypeRef t = makeToken();
	CVBufferSetAttachment(g_buffer, k, t, kCVAttachmentMode_ShouldPropagate);
	CFRelease(t);
	CVBufferRemoveAttachment(g_buffer, k);
	expectDroppedAndReentered("remove", 1);

	t = makeToken();
	CVBufferSetAttachment(g_buffer, k, t, kCVAttachmentMode_ShouldPropagate);
	CFRelease(t);
	CVBufferRemoveAllAttachments(g_buffer);
	expectDroppedAndReentered("remove all", 2);

	t = makeToken();
	CVBufferSetAttachment(g_buffer, k, t, kCVAttachmentMode_ShouldPropagate);
	CFRelease(t);
	CVBufferSetAttachment(g_buffer, k, CFSTR("replacement"), kCVAttachmentMode_ShouldPropagate);
	expectDroppedAndReentered("replace", 3);

	t = makeToken();
	CVBufferSetAttachment(g_buffer, k, t, kCVAttachmentMode_ShouldNotPropagate);
	CFRelease(t);
	CVBufferSetAttachment(g_buffer, k, CFSTR("replacement"), kCVAttachmentMode_ShouldPropagate);
	expectDroppedAndReentered("replace in the other mode", 4);

	t = makeToken();
	CVBufferSetAttachment(g_buffer, k, t, kCVAttachmentMode_ShouldPropagate);
	CFRelease(t);
	CFDictionaryRef replacement = CFDictionaryCreate(NULL, (const void**) &k, (const void*[]) { CFSTR("again") }, 1,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CVBufferSetAttachments(g_buffer, replacement, kCVAttachmentMode_ShouldPropagate);
	CFRelease(replacement);
	expectDroppedAndReentered("replace through a dictionary", 5);

	// The same through CoreMedia, which delegates on a CVBuffer.
	t = makeToken();
	CMSetAttachment(g_buffer, k, t, kCMAttachmentMode_ShouldPropagate);
	CFRelease(t);
	CMRemoveAttachment(g_buffer, k);
	expectDroppedAndReentered("CMRemoveAttachment", 6);

	t = makeToken();
	CMSetAttachment(g_buffer, k, t, kCMAttachmentMode_ShouldPropagate);
	CFRelease(t);
	CMRemoveAllAttachments(g_buffer);
	expectDroppedAndReentered("CMRemoveAllAttachments", 7);

	// A value the caller does not own survives a move to the other mode.
	CVBufferRemoveAllAttachments(g_buffer);
	t = makeToken();
	CVBufferSetAttachment(g_buffer, k, t, kCVAttachmentMode_ShouldPropagate);
	CFRelease(t);
	const int before = g_finalized;
	CVAttachmentMode mode = 99;
	CFTypeRef borrowed = CVBufferGetAttachment(g_buffer, k, &mode);
	CHECK(borrowed != NULL && mode == kCVAttachmentMode_ShouldPropagate);
	CVBufferSetAttachment(g_buffer, k, borrowed, kCVAttachmentMode_ShouldNotPropagate);
	CHECK(g_finalized == before);
	CHECK(CVBufferGetAttachment(g_buffer, k, &mode) == borrowed && mode == kCVAttachmentMode_ShouldNotPropagate);
	CHECK(CFGetRetainCount(borrowed) == 1);
	CVBufferRemoveAllAttachments(g_buffer);
	CHECK(g_finalized == before + 1);

	CFRelease(pixels);
	printf(g_failures ? "FAILED\n" : "PASS\n");
	return g_failures ? 1 : 0;
}
