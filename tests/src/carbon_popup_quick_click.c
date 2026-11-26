// Copyright (C) 2026 Lubos Dolezel
//
// A pop-up menu (PopUpMenuSelect, the menu of a pop-up button or of a bevel button) that is opened with a quick
// click, a press and release that does not move the pointer, stays open until the next click, as the menus of the
// menu bar do when their title is clicked; a press that moves outside the menu and is released there closes it
// without a choice.
//
// Needs a window system (X11 or Wayland) and a host GL driver. The input is posted with CGEventPost() from a
// thread of the program.
#include <Carbon/Carbon.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static volatile int g_phase;            // 1, 2: a pop-up menu is open (the call has not returned)
static volatile int g_downPosted;       // the press that opens the menu was posted
static volatile int g_stillOpen[3];     // the menu was still open after the first gesture of the phase

static void post(CGEventType type, double x, double y)
{
	CGEventRef event = CGEventCreateMouseEvent(NULL, type, CGPointMake(x, y), kCGMouseButtonLeft);

	CGEventPost(kCGSessionEventTap, event);
	CFRelease(event);
}

static void waitForPhase(int phase)
{
	for (int i = 0; i < 100 && g_phase != phase; i++)
		usleep(50000);
	usleep(800000);
}

// The press that opens a pop-up menu is handled by the program before it asks for the menu: the events that
// came with it are taken until the queue is quiet
static void takePress(int posted)
{
	while (g_downPosted != posted)
		usleep(20000);
	for (;;)
	{
		EventRef event = NULL;

		if (ReceiveNextEvent(0, NULL, 0.4, true, &event) != noErr)
			return;
		ReleaseEvent(event);
	}
}

static void* injector(void* arg)
{
	// 1: a quick click on the "control" above the menu (the press opens the menu, the release comes at once),
	// then a click on an item of the menu
	post(kCGEventMouseMoved, 300, 180);
	usleep(200000);
	post(kCGEventLeftMouseDown, 300, 180);
	g_downPosted = 1;
	waitForPhase(1);
	post(kCGEventLeftMouseUp, 300, 180);
	usleep(600000);
	g_stillOpen[1] = g_phase == 1;
	if (g_phase == 1)
	{
		post(kCGEventMouseMoved, 320, 212);
		usleep(200000);
		post(kCGEventLeftMouseDown, 320, 212);
		usleep(50000);
		post(kCGEventLeftMouseUp, 320, 212);
	}
	// 2: a press that moves away from the menu and is released there closes it
	g_downPosted = 0;
	post(kCGEventMouseMoved, 300, 180);
	usleep(200000);
	post(kCGEventLeftMouseDown, 300, 180);
	g_downPosted = 2;
	waitForPhase(2);
	post(kCGEventLeftMouseDragged, 500, 500);
	usleep(200000);
	post(kCGEventLeftMouseDragged, 700, 600);
	usleep(200000);
	post(kCGEventLeftMouseUp, 700, 600);
	usleep(600000);
	g_stillOpen[2] = g_phase == 2;
	return NULL;
}

static void onAlarm(int signal)
{
	printf("FAIL: timed out (phase %d)\n", g_phase);
	fflush(stdout);
	_exit(1);
}

int main(void)
{
	ProcessSerialNumber self = { 0, kCurrentProcess };
	MenuRef menu;
	pthread_t thread;

	signal(SIGALRM, onAlarm);
	alarm(60);
	TransformProcessType(&self, kProcessTransformToForegroundApplication);
	CreateNewMenu(300, 0, &menu);
	AppendMenuItemTextWithCFString(menu, CFSTR("First"), 0, 'frst', NULL);
	AppendMenuItemTextWithCFString(menu, CFSTR("Second"), 0, 'scnd', NULL);
	AppendMenuItemTextWithCFString(menu, CFSTR("Third"), 0, 'thrd', NULL);

	pthread_create(&thread, NULL, injector, NULL);

	takePress(1);
	g_phase = 1;
	UInt32 choice = PopUpMenuSelect(menu, 200, 300, 0);
	g_phase = 0;
	printf("quick click: still open %d, choice %08x\n", g_stillOpen[1], (unsigned) choice);
	CHECK(g_stillOpen[1]);
	CHECK((choice & 0xffff) != 0);

	usleep(300000);
	takePress(2);
	g_phase = 2;
	choice = PopUpMenuSelect(menu, 200, 300, 0);
	g_phase = 0;
	printf("drag away: choice %08x\n", (unsigned) choice);
	CHECK(choice == 0);

	pthread_join(thread, NULL);
	printf(g_failures ? "FAILED: %d\n" : "PASS carbon_popup_quick_click\n", g_failures);
	return g_failures != 0;
}
