// Copyright (C) 2026 Lubos Dolezel
//
// A global hot key in a pure Carbon process (no AppKit): the window system reports the key, and
// the handler of the application target that RegisterEventHotKey was given runs inside
// RunApplicationEventLoop, for the press and the release, with the id of the registration.
// Needs a display where hot keys work (X11 on Darling), and a real key press after READY:
//   xdotool key ctrl+shift+a
// Without HOTKEY_E2E=<seconds> the program only registers and unregisters the hot key. Where the
// window system cannot grab keys (Wayland) the registration fails and the program prints SKIP.
#include <Carbon/Carbon.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void RunApplicationEventLoop(void);
void QuitApplicationEventLoop(void);

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

static int g_pressed, g_released, g_onMainThread;
static EventHotKeyID g_lastID;

static OSStatus handler(EventHandlerCallRef next, EventRef event, void* data)
{
	EventHotKeyID id;

	if (GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(id), NULL, &id) != noErr)
		return eventNotHandledErr;
	g_lastID = id;
	if (GetEventKind(event) == kEventHotKeyPressed)
		g_pressed++;
	else
		g_released++;
	if (g_pressed >= 1 && g_released >= 1)
		QuitApplicationEventLoop();
	return noErr;
}

static void giveUp(EventLoopTimerRef timer, void* data)
{
	QuitApplicationEventLoop();
}

int main(void)
{
	EventTypeSpec specs[2] = { { kEventClassKeyboard, kEventHotKeyPressed }, { kEventClassKeyboard, kEventHotKeyReleased } };
	EventHandlerRef h = NULL;
	EventHotKeyRef ref = NULL;
	EventHotKeyID id = { 'strm', 5 };

	CHECK(InstallEventHandler(GetApplicationEventTarget(), handler, 2, specs, NULL, &h) == noErr);
	OSStatus status = RegisterEventHotKey(kVK_ANSI_A, controlKey | shiftKey, id, GetApplicationEventTarget(), 0, &ref);
	if (status != noErr)
	{
		printf("SKIP: the window system cannot grab keys (RegisterEventHotKey returned %d)\n", (int) status);
		return 0;
	}

	const char* e2e = getenv("HOTKEY_E2E");
	if (e2e)
	{
		EventLoopTimerRef guard = NULL;

		InstallEventLoopTimer(GetMainEventLoop(), atof(e2e), 0, giveUp, NULL, &guard);
		printf("READY\n");
		fflush(stdout);
		RunApplicationEventLoop();
		RemoveEventLoopTimer(guard);
		CHECK(g_pressed == 1 && g_released == 1);
		CHECK(g_lastID.signature == 'strm' && g_lastID.id == 5);

		// A hot key that is gone no longer reaches its handler
		CHECK(UnregisterEventHotKey(ref) == noErr);
		g_pressed = g_released = 0;
		InstallEventLoopTimer(GetMainEventLoop(), atof(e2e) / 3, 0, giveUp, NULL, &guard);
		printf("READY2\n");
		fflush(stdout);
		RunApplicationEventLoop();
		CHECK(g_pressed == 0 && g_released == 0);
	}
	else
		CHECK(UnregisterEventHotKey(ref) == noErr);

	// No AppKit in this process
	for (uint32_t i = 0; i < _dyld_image_count(); i++)
	{
		const char* name = _dyld_get_image_name(i);

		CHECK(!(name && strstr(name, "AppKit.framework")));
	}
	RemoveEventHandler(h);
	printf(failures ? "FAIL: %d checks failed\n" : "PASS\n", failures);
	return failures != 0;
}
