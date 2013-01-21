/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "CGDirectDisplay.h"
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <cstdlib>
#include <cmath>

static Display* g_display = nullptr;
static Display* getDisplay();
static void closeDisplay() __attribute__((destructor));

static int g_lastMouseX = -1, g_lastMouseY = -1;

CFStringRef kCGDisplayWidth = CFSTR("Width");
CFStringRef kCGDisplayHeight = CFSTR("Height");
CFStringRef kCGDisplayMode = CFSTR("Mode");
CFStringRef kCGDisplayBitsPerPixel = CFSTR("BitsPerPixel");
CFStringRef kCGDisplayBitsPerSample = CFSTR("BitsPerSample");
CFStringRef kCGDisplaySamplesPerPixel = CFSTR("SamplesPerPixel");
CFStringRef kCGDisplayRefreshRate = CFSTR("RefreshRate");
CFStringRef kCGDisplayModeUsableForDesktopGUI = CFSTR("UsableForDesktopGUI");
CFStringRef kCGDisplayIOFlags = CFSTR("IOFlags");
CFStringRef kCGDisplayBytesPerRow = CFSTR("kCGDisplayBytesPerRow");
static CFStringRef kCGDarlingResolutionIndex = CFSTR("kCGDarlingResolutionIndex");

Display* getDisplay()
{
        if (g_display)
                return g_display;

        g_display = XOpenDisplay(nullptr);

		// Returning a CG error would be better than this
        if (!g_display)
        {
                std::cerr << "Darling CG: Cannot open a connection to the X server!\n";
                if (!getenv("DISPLAY"))
                        std::cerr << "The application you are trying to run requires an X server and cannot be run only in the console.\n";
                abort();
        }

		int evbase, errbase;
		if (!XRRQueryExtension(g_display, &evbase, &errbase))
		{
			std::cerr << "Darling CG: XRandR not available\n";
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

bool checkRandRVersion(int minMaj, int minMin)
{
	static int maj = 0, min = 0;
	if (!maj && !min)
		XRRQueryVersion(getDisplay(), &maj, &min);
	if (minMaj < maj)
		return false;
	if (minMin < min)
		return false;
	return true;
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

static void CFDictAddInt(CFMutableDictionaryRef dict, CFStringRef key, int number)
{
	CFNumberRef num;
	num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &number);
	CFDictionaryAddValue(dict, key, num);
	CFRelease(num);
}

CFArrayRef CGDisplayCopyAllDisplayModes(CGDirectDisplayID id)
{
	CFMutableArrayRef rv;
	Display* dpy = getDisplay();
	XRRScreenConfiguration* rrc = XRRGetScreenInfo(dpy, RootWindow(dpy, id));
	XRRScreenSize *rrsizes;
	int nSizes;

	rrsizes = XRRConfigSizes(rrc, &nSizes);

	if (nSizes <= 0)
	{
		XRRFreeScreenConfigInfo(rrc);
		return nullptr;
	}
	
	rv = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	for (int i = 0; i < nSizes; i++)
	{
		CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
				&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		short* rates;
		int nRates;

		CFDictAddInt(dict, kCGDisplayWidth, rrsizes[i].width);
		CFDictAddInt(dict, kCGDisplayHeight, rrsizes[i].height);
		CFDictAddInt(dict, kCGDisplayBitsPerPixel, 24);
		CFDictAddInt(dict, kCGDisplayIOFlags, 0);
		CFDictAddInt(dict, kCGDarlingResolutionIndex, i);
		
		CFDictionaryAddValue(dict, kCGDisplayModeUsableForDesktopGUI,
				(rrsizes[i].width > 800 && rrsizes[i].height > 600) ? kCFBooleanTrue : kCFBooleanFalse);

		rates = XRRConfigRates (rrc, i, &nRates);
		for (int j = 0; j < nRates; j++)
		{
			CFMutableDictionaryRef dictR = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, dict);
			CFDictAddInt(dictR, kCGDisplayRefreshRate, rates[j]);
			CFDictAddInt(dictR, kCGDisplayMode, (i << 16) | rates[j]);

			CFArrayAppendValue(rv, dictR);
			CFRelease(dictR);
		}

		CFRelease(dict);
	}

	XRRFreeScreenConfigInfo(rrc);
	return rv;
}

CGError CGDisplaySwitchToMode(CGDirectDisplayID id, CFDictionaryRef mode)
{
	XRRScreenConfiguration* rrc;
	Display* dpy = getDisplay();
	CFNumberRef sizeIndex, refreshRate;
	int sizeIndexN, refreshRateN;
	Status status;

	sizeIndex = (CFNumberRef) CFDictionaryGetValue(mode, kCGDarlingResolutionIndex);
	if (!sizeIndex)
		return kCGErrorIllegalArgument;

	refreshRate = (CFNumberRef) CFDictionaryGetValue(mode, kCGDisplayRefreshRate);
	if (!refreshRate)
		return kCGErrorIllegalArgument;

	CFNumberGetValue(sizeIndex, kCFNumberIntType, &sizeIndexN);
	CFNumberGetValue(refreshRate, kCFNumberIntType, &refreshRateN);
	
	rrc = XRRGetScreenInfo(dpy, RootWindow(dpy, id));
	// TODO: remove fixed RR_Rotate_0
	status = XRRSetScreenConfigAndRate(dpy, rrc, RootWindow(dpy, id), sizeIndexN, RR_Rotate_0, refreshRateN, CurrentTime);

	return (status != Success) ? kCGErrorFailure : kCGErrorSuccess;
}

CFDictionaryRef CGDisplayCurrentMode(CGDirectDisplayID id)
{
	Display* dpy = getDisplay();
	XRRScreenConfiguration* rrc = XRRGetScreenInfo(dpy, RootWindow(dpy, id));
	XRRScreenSize *rrsizes;
	int nSizes;
	SizeID curSize;
	Rotation curRotation;

	rrsizes = XRRConfigSizes(rrc, &nSizes);

	if (nSizes <= 0)
	{
		XRRFreeScreenConfigInfo(rrc);
		return nullptr;
	}

	curSize = XRRConfigCurrentConfiguration(rrc, &curRotation);
	
	CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
				&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFDictAddInt(dict, kCGDisplayWidth, rrsizes[curSize].width);
	CFDictAddInt(dict, kCGDisplayHeight, rrsizes[curSize].height);
	CFDictAddInt(dict, kCGDisplayBitsPerPixel, 24);
	CFDictAddInt(dict, kCGDisplayIOFlags, 0);
	CFDictAddInt(dict, kCGDarlingResolutionIndex, curSize);
		
	CFDictionaryAddValue(dict, kCGDisplayModeUsableForDesktopGUI,
			(rrsizes[curSize].width > 800 && rrsizes[curSize].height > 600) ? kCFBooleanTrue : kCFBooleanFalse);

	short rate = XRRConfigCurrentRate(rrc);
	CFDictAddInt(dict, kCGDisplayRefreshRate, rate);
	CFDictAddInt(dict, kCGDisplayMode, (curSize << 16) | rate);

	XRRFreeScreenConfigInfo(rrc);

	return dict;
}

CGDisplayModeRef CGDisplayCopyDisplayMode(CGDirectDisplayID id)
{
	return CGDisplayCurrentMode(id);
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

static size_t CGDisplayModeGetInt(CGDisplayModeRef mode, CFStringRef key)
{
	int n;
	CFNumberRef height = (CFNumberRef) CFDictionaryGetValue(mode, key);
    if (!height)
        return 0;

    CFNumberGetValue(height, kCFNumberIntType, &n);
	return n;
}

size_t CGDisplayModeGetHeight(CGDisplayModeRef mode)
{
	return CGDisplayModeGetInt(mode, kCGDisplayHeight);
}

double CGDisplayModeGetRefreshRate(CGDisplayModeRef mode)
{
	return CGDisplayModeGetInt(mode, kCGDisplayRefreshRate);
}

CFTypeID CGDisplayModeGetTypeID()
{
	return CFDictionaryGetTypeID();
}

size_t CGDisplayModeGetWidth(CGDisplayModeRef mode)
{
	return CGDisplayModeGetInt(mode, kCGDisplayWidth);
}

bool CGDisplayModeIsUsableForDesktopGUI(CGDisplayModeRef mode)
{
	CFBooleanRef _bool = (CFBooleanRef) CFDictionaryGetValue(mode, kCGDisplayModeUsableForDesktopGUI);
	return !!CFBooleanGetValue(_bool);
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
	return false;
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
	Display* dpy = getDisplay();
	XRRScreenConfiguration* rrc = XRRGetScreenInfo(dpy, RootWindow(dpy, id));
	Rotation rot;

	XRRConfigCurrentConfiguration(rrc, &rot);
	XRRFreeScreenConfigInfo(rrc);

	switch (rot)
	{
	case RR_Rotate_0:
		return 0.0;
	case RR_Rotate_90:
		return 90.0;
	case RR_Rotate_180:
		return 180.0;
	case RR_Rotate_270:
		return 270.0;
	default:
		return 0.0;
	}
}

CGSize CGDisplayScreenSize(CGDirectDisplayID id)
{
	Display* dpy = getDisplay();
	XRRScreenResources* rrc = XRRGetScreenResources(dpy, RootWindow(dpy, id));
	CGSize size;
	
	// No idea if this is even nearly correct, idea taken from Qt
	for (int i = 0; i < rrc->noutput; i++)
	{
		XRROutputInfo *output = XRRGetOutputInfo(dpy, rrc, rrc->outputs[i]);
		if (output->crtc)
			size = CGSizeMake(output->mm_width, output->mm_height);
		XRRFreeOutputInfo(output);
	}

	XRRFreeScreenResources(rrc);
	return size;
}

uint32_t CGDisplaySerialNumber(CGDirectDisplayID id)
{
	return 0;
}

CGError CGDisplaySetDisplayMode(CGDirectDisplayID id, CGDisplayModeRef mode, CFDictionaryRef)
{
	return CGDisplaySwitchToMode(id, mode);
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

CGError CGSetDisplayTransferByFormula(CGDirectDisplayID id, CGGammaValue redMin, CGGammaValue redMax, CGGammaValue redGamma,
		CGGammaValue greenMin, CGGammaValue greenMax, CGGammaValue greenGamma, CGGammaValue blueMin, CGGammaValue blueMax, CGGammaValue blueGamma)
{
	if (!checkRandRVersion(1, 2))
		return kCGErrorNotImplemented;
	
	int SAMPLES = CGDisplayGammaTableCapacity(id);
	CGGammaValue r[SAMPLES], g[SAMPLES], b[SAMPLES];

	auto CalculateSample = [](float index, float min, float max, float value) -> float
	{
		return min + ((max-min) * powf(index, value));
	};

	for (int i = 0; i < SAMPLES; i++)
	{
		float index = 1.0f/SAMPLES * i;
		r[i] = CalculateSample(index, redMin, redMax, redGamma);
		g[i] = CalculateSample(index, greenMin, greenMax, greenGamma);
		b[i] = CalculateSample(index, blueMin, blueMax, blueGamma);
	}

	return CGSetDisplayTransferByTable(id, SAMPLES, r, g, b);
}

CGError CGGetDisplayTransferByFormula(CGDirectDisplayID id, CGGammaValue* redMin, CGGammaValue* redMax, CGGammaValue* redGamma,
		CGGammaValue* greenMin, CGGammaValue* greenMax, CGGammaValue* greenGamma, CGGammaValue* blueMin, CGGammaValue* blueMax, CGGammaValue* blueGamma)
{
	if (!checkRandRVersion(1, 2))
		return kCGErrorNotImplemented;
	
	return kCGErrorNotImplemented;
}

uint32_t CGDisplayGammaTableCapacity(CGDirectDisplayID id)
{
	if (!checkRandRVersion(1, 2))
		return kCGErrorNotImplemented;
	
	Display* dpy = getDisplay();
	XRRScreenResources* rrc = XRRGetScreenResources(dpy, RootWindow(dpy, id));
	int rv;

	rv = XRRGetCrtcGammaSize(dpy, rrc->crtcs[0]);

	XRRFreeScreenResources(rrc);
	return rv;
}

CGError CGSetDisplayTransferByTable(CGDirectDisplayID id, uint32_t tableSize, const CGGammaValue* redTable, const CGGammaValue* greenTable, const CGGammaValue* blueTable)
{
	if (!checkRandRVersion(1, 2))
		return kCGErrorNotImplemented;
	
	XRRCrtcGamma* gamma;
	Display* dpy = getDisplay();
	XRRScreenResources* rrc;

	gamma = XRRAllocGamma(tableSize);
	if (!gamma)
		return kCGErrorCannotComplete;

	rrc = XRRGetScreenResources(dpy, RootWindow(dpy, id));

	for (uint32_t i = 0; i < tableSize; i++)
	{
		gamma->red[i] = short(redTable[i]*65535.0f);
		gamma->green[i] = short(greenTable[i]*65535.0f);
		gamma->blue[i] = short(blueTable[i]*65535.0f);
	}

	for (int i = 0; i < rrc->ncrtc; i++)
		XRRSetCrtcGamma(dpy, rrc->crtcs[i], gamma);

	XRRFreeScreenResources(rrc);
	XRRFreeGamma(gamma);
	return kCGErrorNotImplemented;
}

CGError CGGetDisplayTransferByTable(CGDirectDisplayID id, uint32_t capacity, CGGammaValue* redTable, CGGammaValue* greenTable, CGGammaValue* blueTable, uint32_t* sampleCount)
{
	if (!checkRandRVersion(1, 2))
		return kCGErrorNotImplemented;
	return kCGErrorNotImplemented;
}

