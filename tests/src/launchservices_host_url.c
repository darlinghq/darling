// Copyright (C) 2026 Lubos Dolezel
//
// Web and mail URLs (http, https, mailto) are handed to the host desktop
// through the XDG desktop portal. No application in the prefix handles them,
// so LSGetApplicationForURL and LSCopyDefaultApplicationURLForURL report
// kLSApplicationNotFoundErr; URLs that may not be handed to the host are a
// paramErr; and without a portal, opening fails with
// kLSApplicationNotFoundErr.
//
// The session bus address is pointed at a socket that does not exist, so
// the test never opens anything on the real desktop.
#include <CoreServices/CoreServices.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(1);
}

static CFURLRef url(const char* string)
{
	CFStringRef s = CFStringCreateWithCString(NULL, string, kCFStringEncodingUTF8);
	CFURLRef result = CFURLCreateWithString(NULL, s, NULL);
	CFRelease(s);
	if (result == NULL)
	{
		printf("FAIL: CFURLCreateWithString(%s)\n", string);
		g_failures++;
	}
	return result;
}

static void checkNoApplication(const char* string)
{
	CFURLRef u = url(string);
	if (u == NULL)
		return;

	CFURLRef app = (CFURLRef) 1;
	FSRef ref;
	CHECK(LSGetApplicationForURL(u, kLSRolesAll, &ref, &app) == kLSApplicationNotFoundErr);
	CFErrorRef err = NULL;
	CHECK(LSCopyDefaultApplicationURLForURL(u, kLSRolesAll, &err) == NULL);
	CHECK(err != NULL && CFErrorGetCode(err) == kLSApplicationNotFoundErr);
	if (err)
		CFRelease(err);
	CFRelease(u);
}

static OSStatus openURL(const char* string)
{
	CFURLRef u = url(string);
	if (u == NULL)
		return 1;
	OSStatus status = LSOpenCFURLRef(u, NULL);
	CFRelease(u);
	return status;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	setenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/nonexistent/darling-test-bus", 1);

	checkNoApplication("https://example.org/index.html");
	checkNoApplication("http://example.org/");
	checkNoApplication("HTTPS://example.org/");
	checkNoApplication("mailto:someone@example.org");

	// Refused by the URL policy before the bus is touched.
	CHECK(openURL("https://example.org/$(id)") == paramErr);
	CHECK(openURL("mailto:someone@example.org?attach=/etc/passwd") == paramErr);
	CHECK(openURL("http:foo") == paramErr);

	// No portal reachable.
	OSStatus status = openURL("https://example.org/");
	printf("open without a portal: %d\n", (int) status);
	CHECK(status == kLSApplicationNotFoundErr);

	// Other schemes are not handed to the host.
	CHECK(openURL("ftp://example.org/") == kLSApplicationNotFoundErr);

	if (g_failures == 0)
		printf("PASS\n");
	finish(g_failures == 0 ? 0 : 1);
}
