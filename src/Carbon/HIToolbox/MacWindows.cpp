#include "MacWindows.h"
#include <X11/Xlib.h>
#include <iostream>
#include <cstdlib>

static Display* g_display = nullptr;
static void closeDisplay() __attribute__((destructor));

Display* Darling::Carbon::getDisplay()
{
	if (g_display)
		return g_display;

	g_display = XOpenDisplay(nullptr);

	if (!g_display)
	{
		std::cerr << "Darling Carbon: Cannot open a connection to the X server!\n";
		if (!getenv("DISPLAY"))
			std::cerr << "The application you are trying to run requires an X server and cannot be run only in the console.\n";
		abort();
	}

	return g_display;
}

void closeDisplay()
{
	if (g_display)
	{
		XCloseDisplay(g_display);
		g_display = nullptr;
	}
}

OSStatus CreateNewWindow(WindowClass cls, WindowAttributes attr, const Rect* rect, WindowRef* newWindow)
{
	Window win;
	Display* dpy;
	int blackColor, whiteColor;

	*newWindow = 0;

	if (rect->right < rect->left || rect->bottom < rect->top)
		return paramErr;

	dpy = Darling::Carbon::getDisplay();
	blackColor = BlackPixel(dpy, DefaultScreen(dpy));
	whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

	win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), rect->left,
			rect->top, rect->right - rect->left,
			rect->bottom - rect->top, 0, blackColor, whiteColor);

	if (!win)
		return paramErr;

	*newWindow = win;
	return noErr;
}

void DisposeWindow(WindowRef wnd)
{
	if (wnd)
		XDestroyWindow(wnd);
}

void ShowWindow(WindowRef wnd)
{
	if (wnd)
		XMapWindow(Darling::Carbon::getDisplay(), Window(wnd));
}

void HideWindow(WindowRef wnd)
{
	if (wnd)
		XUnmapWindow(Darling::Carbon::getDisplay(), Window(wnd));
}

void ShowHide(WindowRef wnd, Boolean showFlag)
{
	if (showFlag)
		ShowWindow(wnd);
	else
		HideWindow(wnd);
}

Boolean IsWindowVisible(WindowRef wnd)
{
	
}

