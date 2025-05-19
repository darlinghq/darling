#include <HIToolbox/MacWindows.h>
#include <stdlib.h>
#include <stdio.h>

#define NO_X11 

#ifndef NO_X11
#include <X11/Xlib.h>
#endif

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


#ifndef NO_X11
static Display* g_display = nullptr;
#endif
static void closeDisplay() __attribute__((destructor));

namespace Darling
{

namespace Carbon
{

#ifndef NO_X11
Display* getDisplay()
{
	if (g_display)
		return g_display;

	g_display = XOpenDisplay(nullptr);

	if (!g_display)
	{
		fprintf(stderr, "Darling Carbon: Cannot open a connection to the X server!\n");
		if (!getenv("DISPLAY"))
			fprintf(stderr, "The application you are trying to run requires an X server and cannot be run only in the console.\n");
		abort();
	}

	return g_display;
}
#endif

void closeDisplay()
{
#ifndef NO_X11
	if (g_display)
	{
		XCloseDisplay(g_display);
		g_display = nullptr;
	}
#endif
}

}
}

OSStatus CreateNewWindow(WindowClass cls, WindowAttributes attr, const Rect* rect, WindowRef* newWindow)
{
#ifndef NO_X11
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
#endif
	return noErr;
}

void DisposeWindow(WindowRef wnd)
{
#ifndef NO_X11
	if (wnd)
		XDestroyWindow(Darling::Carbon::getDisplay(), wnd);
#endif
}

void ShowWindow(WindowRef wnd)
{
#ifndef NO_X11
	if (wnd)
		XMapWindow(Darling::Carbon::getDisplay(), Window(wnd));
#endif
}

void HideWindow(WindowRef wnd)
{
#ifndef NO_X11
	if (wnd)
		XUnmapWindow(Darling::Carbon::getDisplay(), Window(wnd));
#endif
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
	return 0;
}


OSStatus SelectWindow(WindowRef a)
{
	return 0;
}

OSStatus SetWindowContentColor(WindowRef a, const RGBColor * b)
{
	return 0;
}

OSStatus SetWindowGroup(WindowRef a, WindowGroupRef b)
{
	return 0;
}

OSStatus SetWindowGroupLevel(WindowGroupRef a, SInt32 b)
{
	return 0;
}

OSStatus SetWindowTitleWithCFString(WindowRef a, CFStringRef b)
{
	return 0;
}

OSStatus TransitionWindowWithOptions(WindowRef a, WindowTransitionEffect b, WindowTransitionAction c, const HIRect * d, Boolean e, TransitionWindowOptions * f)
{
	return 0;
}

OSStatus CreateWindowGroup(WindowGroupAttributes a, WindowGroupRef *b)
{
	return 0;
}

OSStatus GetWindowBounds(WindowRef a, WindowRegionCode b, Rect * c)
{
	return 0;
}

OSStatus GetWindowEventTarget(WindowRef a)
{
	return 0;
}

OSStatus GetWindowGroupLevel(WindowGroupRef a, SInt32 * b)
{
	return 0;
}

WindowGroupRef GetWindowGroupOfClass(WindowClass a)
{
	return (WindowGroupRef)0;
}

CGrafPtr GetWindowPort(WindowRef a)
{
	return (CGrafPtr)0;
}

Rect* GetWindowPortBounds(WindowRef window, Rect * bounds)
{
	return NULL;
}

Boolean IsValidWindowPtr(WindowRef a)
{
	return 0;
}

OSStatus ReleaseWindow(WindowRef a)
{
	return 0;
}
