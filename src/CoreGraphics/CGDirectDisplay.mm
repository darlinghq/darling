#include <iostream>
#include "CGDirectDisplay.h"
#include <X11/Xlib.h>
#include <cstdlib>

static Display* g_display = nullptr;
static Display* getDisplay();
static void closeDisplay() __attribute__((destructor));

static int g_lastMouseX = -1, g_lastMouseY = -1;

Display* getDisplay()
{
        if (g_display)
                return g_display;

        g_display = XOpenDisplay(nullptr);

        if (!g_display)
        {
                std::cerr << "Darling CG: Cannot open a connection to the X server!\n";
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

CGError CGCaptureAllDisplays()
{
	return kCGErrorSuccess;
}

CGError CGCaptureAllDisplaysWithOptions(CGCaptureOptions opts)
{
	return kCGErrorSuccess;
}

bool CGDisplayIsCaptured(CGDirectDisplayID id)
{
	return true;
}

CGError CGDisplayRelease(CGDirectDisplayID id)
{
	return kCGErrorSuccess;
}

CGError CGReleaseAllDisplays()
{
	return kCGErrorSuccess;
}

CGError CGDisplayMoveCursorToPoint(CGDirectDisplayID id, CGPoint pt)
{
	Display* dpy = getDisplay();
	Window root_window;
	root_window = XRootWindow(dpy, 0);
	
	XSelectInput(dpy, root_window, KeyReleaseMask);
	XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, pt.x, pt.y);
	
	g_lastMouseX = pt.x;
	g_lastMouseY = pt.y;
	
	XFlush(dpy);
	return kCGErrorSuccess;
}

void CGGetLastMouseDelta(int32_t* x, int32_t* y)
{
	Display* dpy = getDisplay();
	Window root_window;
	Window window_returned;
	int root_x, root_y;
	int win_x, win_y;
	unsigned int mask_return;
	
	root_window = XRootWindow(dpy, 0);
	XQueryPointer(dpy, root_window, &window_returned,
                &window_returned, &root_x, &root_y, &win_x, &win_y,
                &mask_return);
	
	if (g_lastMouseX == -1)
		*x = 0;
	else
		*x = root_x - g_lastMouseX;
		
	if (g_lastMouseY == -1)
		*y = 0;
	else
		*y = root_y - g_lastMouseY;
	
	g_lastMouseX = root_x;
	g_lastMouseY = root_y;
}

CGError CGGetActiveDisplayList(uint32_t max, CGDirectDisplayID* list, uint32_t* countOut)
{
	Display* dpy = getDisplay();
	int count = XScreenCount(dpy);
	
	for (int i = 0; i < count && i < max; i++)
		list[i] = i;
	
	*countOut = std::min<uint32_t>(count, max);
	return kCGErrorSuccess;
}

CGError CGGetOnlineDisplayList(uint32_t max, CGDirectDisplayID* list, uint32_t* countOut)
{
	return CGGetActiveDisplayList(max, list, countOut);
}

size_t CGDisplayPixelsWide(CGDirectDisplayID id)
{
	Screen* screen = XScreenOfDisplay(getDisplay(), id);
	if (!screen)
		return 0;
	else
		return XWidthOfScreen(screen);
}

size_t CGDisplayPixelsHigh(CGDirectDisplayID id)
{
	Screen* screen = XScreenOfDisplay(getDisplay(), id);
	if (!screen)
		return 0;
	else
		return XHeightOfScreen(screen);
}

CFArrayRef CGDisplayCopyAllDisplayModes(CGDirectDisplayID id)
{
	// TODO: Use XRandR API
	return nullptr;
}

CGError CGDisplaySwitchToMode(CGDirectDisplayID id, CFDictionaryRef mode)
{
	// TODO: Use XRandR API
	return kCGErrorNotImplemented;
}

CFDictionaryRef CGDisplayCurrentMode(CGDirectDisplayID id)
{
	return nullptr;
}

CGDisplayModeRef CGDisplayCopyDisplayMode(CGDirectDisplayID id)
{
	return nullptr;
}

CGDisplayModeRef CGDisplayModeRetain(CGDisplayModeRef mode)
{
	return (CGDisplayModeRef) CFRetain(mode);
}

void CGDisplayModeRelease(CGDisplayModeRef mode)
{
	CFRelease(mode);
}

CFStringRef CGDisplayModeCopyPixelEncoding(CGDisplayModeRef mode)
{
	return nullptr;
}

size_t CGDisplayModeGetHeight(CGDisplayModeRef mode)
{
	return 0;
}

double CGDisplayModeGetRefreshRate(CGDisplayModeRef mode)
{
	return 0;
}

CFTypeID CGDisplayModeGetTypeID()
{
	return CFNullGetTypeID();
}

size_t CGDisplayModeGetWidth(CGDisplayModeRef mode)
{
	return 0;
}

bool CGDisplayModeIsUsableForDesktopGUI(CGDisplayModeRef mode)
{
	return true;
}

bool CGDisplayIsActive(CGDirectDisplayID id)
{
	return true;
}

bool CGDisplayIsAlwaysInMirrorSet(CGDirectDisplayID id)
{
	return false;
}

bool CGDisplayIsAsleep(CGDirectDisplayID id)
{
	return true;
}

bool CGDisplayIsBuiltin(CGDirectDisplayID id)
{
	return false;
}

bool CGDisplayIsInHWMirrorSet(CGDirectDisplayID id)
{
	return false;
}

bool CGDisplayIsInMirrorSet(CGDirectDisplayID id)
{
	return false;
}

bool CGDisplayIsMain(CGDirectDisplayID id)
{
	return true;
}

bool CGDisplayIsOnline(CGDirectDisplayID id)
{
	return true;
}

bool CGDisplayIsStereo(CGDirectDisplayID id)
{
	return false;
}

CGDirectDisplayID CGDisplayMirrorsDisplay(CGDirectDisplayID id)
{
	return id;
}

uint32_t CGDisplayModelNumber(CGDirectDisplayID)
{
	return 0;
}

CGDirectDisplayID CGDisplayPrimaryDisplay(CGDirectDisplayID id)
{
	return id;
}

double CGDisplayRotation(CGDirectDisplayID id)
{
	return 0.0;
}

CGSize CGDisplayScreenSize(CGDirectDisplayID id)
{
	return CGSize{0, 0};
}

uint32_t CGDisplaySerialNumber(CGDirectDisplayID id)
{
	return 0;
}

CGError CGDisplaySetDisplayMode(CGDirectDisplayID id, CGDisplayModeRef mode, CFDictionaryRef unused)
{
	return kCGErrorNotImplemented;
}

uint32_t CGDisplayUnitNumber(CGDirectDisplayID id)
{
	return 0;
}

bool CGDisplayUsesOpenGLAcceleration(CGDirectDisplayID id)
{
	return true;
}

uint32_t CGDisplayVendorNumber(CGDirectDisplayID id)
{
	return 0;
}

