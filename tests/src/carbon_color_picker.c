// Copyright (C) 2026 Lubos Dolezel
//
// The Color Picker of the Carbon API: NPickColor() shows a dialog that asks for a colour and returns when the
// user is done with it. The program shows the dialog twice with a colour to start with, and a harness
// (carbon_dialog_user.py) acts as the user: it accepts the first dialog and cancels the second. What is
// checked is what the documentation promises:
//   - an accepted dialog has newColorChosen set, and the colour is the one the dialog was given when the user
//     changed nothing (the 16 bit RGB colour, within what a conversion to the display's colour space may change);
//   - a cancelled dialog has newColorChosen cleared and leaves the colour of the caller as it was;
//   - NPickColor() is given nothing it cannot work with: a missing structure is paramErr.
//
//   carbon_color_picker
//
// Lines the harness reads:
//   DIALOG_READY n    the dialog n (1: to be accepted, 2: to be cancelled) is shown
//   PASS carbon_color_picker / FAIL ...
//
// Needs a window system (X11 or Wayland) and a host GL driver; 32-bit only (the Color Picker is not in the
// 64-bit API).
#include <Carbon/Carbon.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __LP64__
int main(void)
{
	printf("PASS carbon_color_picker (no 64-bit API)\n");
	return 0;
}
#else

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(1);
}

static int close_to(unsigned a, unsigned b)
{
	return (a > b ? a - b : b - a) <= 300;
}



static NColorPickerInfo g_info;
static int g_number;
static int g_done;

// NPickColor() blocks, so the program calls it from the event loop, as a program does from a button's handler
static void pick(EventLoopTimerRef timer, void* userData)
{
	printf("DIALOG_READY %d\n", g_number);
	fflush(stdout);
	g_info.newColorChosen = 0xAA;
	CHECK(NPickColor(&g_info) == noErr);
	g_done = 1;
	QuitApplicationEventLoop();
}

static void runPicker(int number)
{
	RGBColor start = {0x3333, 0x6666, 0xCCCC};
	EventLoopTimerRef timer;

	g_number = number;
	g_done = 0;
	memset(&g_info, 0, sizeof(g_info));
	g_info.theColor.color.rgb = start;
	g_info.placeWhere = kCenterOnMainScreen;
	memcpy(g_info.prompt, "\pPick a colour", 14);
	InstallEventLoopTimer(GetMainEventLoop(), 0.5, 0, NewEventLoopTimerUPP(pick), NULL, &timer);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(timer);
	CHECK(g_done);
	if (number == 1)
	{
		CHECK(g_info.newColorChosen == true);
		CHECK(close_to(g_info.theColor.color.rgb.red, start.red));
		CHECK(close_to(g_info.theColor.color.rgb.green, start.green));
		CHECK(close_to(g_info.theColor.color.rgb.blue, start.blue));
		printf("accepted %04x %04x %04x\n", g_info.theColor.color.rgb.red, g_info.theColor.color.rgb.green, g_info.theColor.color.rgb.blue);
	}
	else
	{
		CHECK(g_info.newColorChosen == false);
		CHECK(g_info.theColor.color.rgb.red == start.red && g_info.theColor.color.rgb.green == start.green &&
			g_info.theColor.color.rgb.blue == start.blue);
	}
}

int main(void)
{
	Rect bounds = {100, 100, 300, 400};
	WindowRef window;

	signal(SIGALRM, on_alarm);
	alarm(100);
	CHECK(NPickColor(NULL) == paramErr);
	CHECK(CreateNewWindow(kDocumentWindowClass, kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute, &bounds, &window) == noErr);
	ShowWindow(window);
	runPicker(1);
	runPicker(2);
	if (g_failures)
		printf("FAIL carbon_color_picker: %d check(s)\n", g_failures);
	else
		printf("PASS carbon_color_picker\n");
	return g_failures != 0;
}

#endif
