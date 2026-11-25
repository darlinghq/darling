// Copyright (C) 2026 Lubos Dolezel
//
// The Carbon Control Manager and HIView: the controls of a window made from a nib (a bevel button with
// a menu, a toggling bevel button, a push button, a slider, a scroll bar, static text), their values, titles,
// IDs, command IDs and hierarchy, how they look in the window (read back with CGWindowListCreateImage()),
// the events of controls (draw, hit, command), and the tracking of the mouse: clicks, a drag on a slider and on a
// scroll bar with action functions, and a pop-up menu of a bevel button, all with input that the program posts with
// CGEventPost() from a thread of its own.
//
// Needs a window system (X11 or Wayland). The nib is written by the test, in a bundle of its own, from a hand-written
// description. The behaviours the documentation does not give are pinned by the test_quirk_* checks (see the QUIRK:
// comments in Controls.c).
#include <Carbon/Carbon.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern unsigned int _HIWindowGetCGSWindowID(WindowRef window);

static int g_failures, g_checks;
#define CHECK(cond) do { ++g_checks; if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fflush(stdout); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long long _a = (long long) (a), _b = (long long) (b); ++g_checks; if (_a != _b) { printf("FAIL line %d: %s = %lld, expected %lld\n", __LINE__, #a, _a, _b); fflush(stdout); g_failures++; } } while (0)

enum { kSig = 'Test', kPush = 1, kCheck = 2, kSlider = 3, kScroll = 4, kText = 5, kMenuButton = 6, kToggle = 7 };

static const char kObjects[] =
	"<?xml version=\"1.0\" standalone=\"yes\"?>\n"
	"<object class=\"NSIBObjectData\">\n"
	"  <string name=\"targetFramework\">IBCarbonFramework</string>\n"
	"  <object name=\"rootObject\" class=\"NSCustomObject\" id=\"1\"><string name=\"customClass\">NSApplication</string></object>\n"
	"  <array count=\"1\" name=\"allObjects\">\n"
	"    <object class=\"IBCarbonWindow\" id=\"100\">\n"
	"      <string name=\"windowRect\">100 100 400 500 </string>\n"
	"      <string name=\"title\">Controls</string>\n"
	"      <object name=\"rootControl\" class=\"IBCarbonRootControl\" id=\"101\">\n"
	"        <string name=\"bounds\">0 0 300 400 </string>\n"
	"        <array count=\"7\" name=\"subviews\">\n"
	"          <object class=\"IBCarbonPushButton\" id=\"110\">\n"
	"            <string name=\"bounds\">10 10 32 110 </string>\n"
	"            <ostype name=\"controlSignature\">Test</ostype><int name=\"controlID\">1</int>\n"
	"            <string name=\"title\">Push</string><ostype name=\"command\">push</ostype>\n"
	"          </object>\n"
	"          <object class=\"IBCarbonCheckBox\" id=\"111\">\n"
	"            <string name=\"bounds\">10 130 30 250 </string>\n"
	"            <ostype name=\"controlSignature\">Test</ostype><int name=\"controlID\">2</int>\n"
	"            <string name=\"title\">Check</string>\n"
	"          </object>\n"
	"          <object class=\"IBCarbonSlider\" id=\"112\">\n"
	"            <string name=\"bounds\">50 10 70 200 </string>\n"
	"            <ostype name=\"controlSignature\">Test</ostype><int name=\"controlID\">3</int>\n"
	"            <int name=\"initialValue\">30</int><boolean name=\"isLive\">TRUE</boolean>\n"
	"          </object>\n"
	"          <object class=\"IBCarbonScrollBar\" id=\"113\">\n"
	"            <string name=\"bounds\">10 300 210 315 </string>\n"
	"            <ostype name=\"controlSignature\">Test</ostype><int name=\"controlID\">4</int>\n"
	"            <int name=\"minimum\">0</int><int name=\"maximum\">100</int><int name=\"viewSize\">10</int><boolean name=\"isLive\">TRUE</boolean>\n"
	"          </object>\n"
	"          <object class=\"IBCarbonStaticText\" id=\"114\">\n"
	"            <string name=\"bounds\">80 10 100 150 </string>\n"
	"            <ostype name=\"controlSignature\">Test</ostype><int name=\"controlID\">5</int>\n"
	"            <string name=\"title\">Hello</string>\n"
	"          </object>\n"
	"          <object class=\"IBCarbonBevelButton\" id=\"115\">\n"
	"            <string name=\"bounds\">110 10 134 110 </string>\n"
	"            <ostype name=\"controlSignature\">Test</ostype><int name=\"controlID\">6</int>\n"
	"            <string name=\"title\">Width</string><int name=\"menuPlacement\">4</int>\n"
	"            <object name=\"menu\" class=\"IBCarbonMenu\" id=\"116\">\n"
	"              <string name=\"title\">Width</string>\n"
	"              <array count=\"3\" name=\"items\">\n"
	"                <object class=\"IBCarbonMenuItem\" id=\"117\"><string name=\"title\">One</string><ostype name=\"command\">mni1</ostype></object>\n"
	"                <object class=\"IBCarbonMenuItem\" id=\"118\"><string name=\"title\">Two</string><ostype name=\"command\">mni2</ostype></object>\n"
	"                <object class=\"IBCarbonMenuItem\" id=\"119\"><string name=\"title\">Three</string><ostype name=\"command\">mni3</ostype></object>\n"
	"              </array>\n"
	"            </object>\n"
	"            <int name=\"selectedItem\">1</int>\n"
	"          </object>\n"
	"          <object class=\"IBCarbonBevelButton\" id=\"120\">\n"
	"            <string name=\"bounds\">150 10 174 80 </string>\n"
	"            <ostype name=\"controlSignature\">Test</ostype><int name=\"controlID\">7</int>\n"
	"            <string name=\"title\">Tog</string><unsigned_int name=\"behavior\">256</unsigned_int><ostype name=\"command\">togl</ostype>\n"
	"          </object>\n"
	"        </array>\n"
	"      </object>\n"
	"      <boolean name=\"hasCloseBox\">TRUE</boolean>\n"
	"    </object>\n"
	"  </array>\n"
	"  <dictionary count=\"1\" name=\"nameTable\">\n"
	"    <string>MainWindow</string>\n"
	"    <reference idRef=\"100\"/>\n"
	"  </dictionary>\n"
	"  <unsigned_int name=\"nextObjectID\">200</unsigned_int>\n"
	"</object>\n";

static void writeFile(const char* path, const char* contents)
{
	FILE* file = fopen(path, "w");

	if (!file || fwrite(contents, 1, strlen(contents), file) != strlen(contents))
		printf("FAIL: cannot write %s\n", path);
	if (file)
		fclose(file);
}

static CFBundleRef makeBundle(const char* root, const char* nibName, const char* objects)
{
	char path[1024];
	const char* dirs[] = { "", "/Contents", "/Contents/Resources", "/Contents/Resources/en.lproj" };

	for (int i = 0; i < 4; i++)
	{
		snprintf(path, sizeof(path), "%s/Fixture.app%s", root, dirs[i]);
		mkdir(path, 0755);
	}
	snprintf(path, sizeof(path), "%s/Fixture.app/Contents/Info.plist", root);
	writeFile(path, "<?xml version=\"1.0\"?><plist version=\"1.0\"><dict><key>CFBundleIdentifier</key><string>test.controlsfixture</string>"
		"<key>CFBundleDevelopmentRegion</key><string>en</string><key>CFBundleName</key><string>Fixture</string></dict></plist>");
	snprintf(path, sizeof(path), "%s/Fixture.app/Contents/Resources/en.lproj/%s.nib", root, nibName);
	mkdir(path, 0755);
	snprintf(path, sizeof(path), "%s/Fixture.app/Contents/Resources/en.lproj/%s.nib/objects.xib", root, nibName);
	writeFile(path, objects);

	snprintf(path, sizeof(path), "%s/Fixture.app", root);
	CFStringRef string = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, string, kCFURLPOSIXPathStyle, true);
	CFBundleRef bundle = CFBundleCreate(NULL, url);

	CFRelease(url);
	CFRelease(string);
	return bundle;
}

static void quitTimer(EventLoopTimerRef t, void* u) { QuitApplicationEventLoop(); }

static void spin(int ms)
{
	EventLoopTimerRef timer;

	InstallEventLoopTimer(GetMainEventLoop(), ms / 1000.0, 0, NewEventLoopTimerUPP(quitTimer), NULL, &timer);
	RunApplicationEventLoop();
	RemoveEventLoopTimer(timer);
}

// ---- pixels

static unsigned char* readPixels(WindowRef w, size_t* width, size_t* height)
{
	CGImageRef image = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow, _HIWindowGetCGSWindowID(w),
		kCGWindowImageBoundsIgnoreFraming);

	if (!image)
		return NULL;
	*width = CGImageGetWidth(image);
	*height = CGImageGetHeight(image);

	unsigned char* pixels = calloc(*width * *height, 4);
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGContextRef ctx = CGBitmapContextCreate(pixels, *width, *height, 8, *width * 4, space, kCGImageAlphaPremultipliedLast);

	CGContextSetBlendMode(ctx, kCGBlendModeCopy);
	CGContextDrawImage(ctx, CGRectMake(0, 0, *width, *height), image);
	CGContextRelease(ctx);
	CGColorSpaceRelease(space);
	CGImageRelease(image);
	return pixels;
}

// The number of pixels in a rectangle (the top left origin) that are not white
static int inkIn(const unsigned char* pixels, size_t width, size_t height, int left, int top, int right, int bottom)
{
	int count = 0;

	for (int y = top; y < bottom && y < (int) height; y++)
		for (int x = left; x < right && x < (int) width; x++)
		{
			const unsigned char* p = pixels + ((size_t) y * width + x) * 4;

			if (p[0] < 235 || p[1] < 235 || p[2] < 235)
				count++;
		}
	return count;
}

static int differing(const unsigned char* a, const unsigned char* b, size_t width, int left, int top, int right, int bottom)
{
	int count = 0;

	for (int y = top; y < bottom; y++)
		for (int x = left; x < right; x++)
			if (memcmp(a + ((size_t) y * width + x) * 4, b + ((size_t) y * width + x) * 4, 4))
				count++;
	return count;
}

// ---- events

static volatile int g_menuEnded;
static int g_hits[16], g_commandCount, g_draws, g_actionCalls, g_lastPart, g_actionParts[16];
static UInt32 g_commands[16], g_commandAttributes[16];
static int g_clicks[16], g_menuCommands;

#ifdef __LP64__
#define SETV(c, v) HIViewSetValue((c), (v))
#else
#define SETV(c, v) SetControl32BitValue((c), (v))
#endif

#ifdef __LP64__
#define GETV(c) HIViewGetValue(c)
#else
#define GETV(c) GetControl32BitValue(c)
#endif

static OSStatus controlHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	long which = (long) userData;

	switch (GetEventKind(event))
	{
		case kEventControlHit:
		{
			ControlPartCode part = 0;

			GetEventParameter(event, kEventParamControlPart, typeControlPartCode, NULL, sizeof(part), NULL, &part);
			g_hits[which]++;
			g_lastPart = part;
			break;
		}
		case kEventControlClick:
			g_clicks[which]++;
			break;
		case kEventControlDraw:
			g_draws++;
			break;
	}
	return eventNotHandledErr;
}

static OSStatus menuHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	g_menuEnded = 1;
	return eventNotHandledErr;
}

static OSStatus commandHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	HICommand command;

	if (GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command) != noErr)
		return eventNotHandledErr;
	if (g_commandCount < 16)
	{
		g_commands[g_commandCount] = command.commandID;
		g_commandAttributes[g_commandCount++] = command.attributes;
	}
	return noErr;
}

static void actionProc(ControlRef control, ControlPartCode part)
{
	g_actionCalls++;
	if (g_actionCalls < 16)
		g_actionParts[g_actionCalls] = part;
	// Like an application's scroll action: the value follows the arrows and the pages
	switch (part)
	{
		case kControlUpButtonPart: SETV(control, GetControl32BitValue(control) - 1); break;
		case kControlDownButtonPart: SETV(control, GetControl32BitValue(control) + 1); break;
		case kControlPageUpPart: SETV(control, GetControl32BitValue(control) - 10); break;
		case kControlPageDownPart: SETV(control, GetControl32BitValue(control) + 10); break;
		default: break;
	}
}

// ---- input

static void post(CGEventRef event)
{
	CGEventPost(kCGSessionEventTap, event);
	CFRelease(event);
}

static WindowRef g_window;

// The events a program posts have no window under the pointer: the window is named in them
static void mouse(CGEventType type, double x, double y)
{
	CGEventRef event = CGEventCreateMouseEvent(NULL, type, CGPointMake(x, y), kCGMouseButtonLeft);
	int64_t window = _HIWindowGetCGSWindowID(g_window);

	CGEventSetIntegerValueField(event, kCGMouseEventWindowUnderMousePointer, window);
	CGEventSetIntegerValueField(event, kCGMouseEventWindowUnderMousePointerThatCanHandleThisEvent, window);
	post(event);
}

static void click(double x, double y)
{
	mouse(kCGEventLeftMouseDown, x, y);
	usleep(150000);
	mouse(kCGEventLeftMouseUp, x, y);
	usleep(300000);
}

static void dragTo(double x0, double y0, double x1, double y1, int steps)
{
	mouse(kCGEventLeftMouseDown, x0, y0);
	usleep(200000);
	for (int i = 1; i <= steps; i++)
	{
		mouse(kCGEventLeftMouseDragged, x0 + (x1 - x0) * i / steps, y0 + (y1 - y0) * i / steps);
		usleep(60000);
	}
	usleep(150000);
	mouse(kCGEventLeftMouseUp, x1, y1);
	usleep(300000);
}

static ControlRef g_controls[8];
static volatile int g_done;
static Rect g_content;

static ControlRef byID(int id)
{
	ControlID cid = { kSig, id };
	ControlRef c = NULL;

	GetControlByID(g_window, &cid, &c);
	return c;
}

static void centerOf(ControlRef c, double* x, double* y)
{
	Rect b;

	GetWindowBounds(g_window, kWindowContentRgn, &g_content);
	GetControlBounds(c, &b);
	*x = g_content.left + (b.left + b.right) / 2.0;
	*y = g_content.top + (b.top + b.bottom) / 2.0;
}

static volatile int g_phase;
static int g_toggleAfterFirst, g_sliderBefore, g_sliderAfterDrag, g_sliderAfterGroove;
static int g_arrowCalls, g_arrowValue, g_pageCalls, g_pagePart, g_indicatorBefore, g_indicatorAfter, g_indicatorPart;
static SInt16 g_menuChosen;
static int g_pageValue;

static void* injector(void* arg)
{
	double x, y;
	Rect b;

	usleep(400000);
	GetWindowBounds(g_window, kWindowContentRgn, &g_content);
	// 1. A click on the push button: the control gets the click, a hit and its command
	centerOf(g_controls[kPush], &x, &y);
	click(x, y);
	// 2. A press on it that is dragged out of it before the release is no hit
	mouse(kCGEventLeftMouseDown, x, y);
	usleep(200000);
	mouse(kCGEventLeftMouseDragged, x + 150, y + 60);
	usleep(200000);
	mouse(kCGEventLeftMouseUp, x + 150, y + 60);
	usleep(300000);
	g_phase = 1;
	// 3. The toggling bevel button: two clicks
	centerOf(g_controls[kToggle], &x, &y);
	click(x, y);
	g_toggleAfterFirst = GETV(g_controls[kToggle]);
	click(x, y);
	g_phase = 2;
	// 4. The check box toggles
	centerOf(g_controls[kCheck], &x, &y);
	click(x - 40, y);
	g_phase = 3;
	// 5. The slider: a drag of the thumb to the right, then a click in the groove at the left end
	GetControlBounds(g_controls[kSlider], &b);
	{
		double sy = g_content.top + (b.top + b.bottom) / 2.0;
		double left = g_content.left + b.left, width = b.right - b.left;

		g_sliderBefore = GETV(g_controls[kSlider]);
		dragTo(left + width * 0.30, sy, left + width * 0.85, sy, 8);
		g_sliderAfterDrag = GETV(g_controls[kSlider]);
		click(left + 3, sy);
		g_sliderAfterGroove = GETV(g_controls[kSlider]);
	}
	g_phase = 4;
	// 6. The scroll bar: the down arrow held, then a click in the page area, then a drag of the indicator
	GetControlBounds(g_controls[kScroll], &b);
	{
		double sx = g_content.left + (b.left + b.right) / 2.0;

		g_actionCalls = 0;
		mouse(kCGEventLeftMouseDown, sx, g_content.top + b.bottom - 6);
		usleep(900000);
		mouse(kCGEventLeftMouseUp, sx, g_content.top + b.bottom - 6);
		usleep(300000);
		g_arrowCalls = g_actionCalls;
		g_arrowValue = GETV(g_controls[kScroll]);
		SETV(g_controls[kScroll], 0);
		usleep(200000);
		g_actionCalls = 0;
		click(sx, g_content.top + b.top + 100);
		g_pageCalls = g_actionCalls;
		g_pageValue = GETV(g_controls[kScroll]);
		g_pagePart = g_actionParts[1];
		SETV(g_controls[kScroll], 50);
		usleep(300000);
		g_actionCalls = 0;
		g_indicatorBefore = GETV(g_controls[kScroll]);
		HIThemeTrackDrawInfo info;
		HIRect thumb;

		memset(&info, 0, sizeof(info));
		info.kind = kThemeScrollBarMedium;
		info.bounds = CGRectMake(0, 0, b.right - b.left, b.bottom - b.top);
		info.max = 100;
		info.value = 50;
		info.attributes = kThemeTrackShowThumb;
		info.enableState = kThemeTrackActive;
		info.trackInfo.scrollbar.viewsize = 10;
		HIThemeGetTrackPartBounds(&info, kAppearancePartIndicator, &thumb);
		dragTo(sx, g_content.top + b.top + thumb.origin.y + thumb.size.height / 2, sx, g_content.top + b.top + thumb.origin.y + thumb.size.height / 2 + 40, 6);
		g_indicatorAfter = GETV(g_controls[kScroll]);
		g_indicatorPart = g_actionParts[1];
	}
	g_phase = 5;
	// 7. The bevel button's menu: a press opens it, the pointer moves down onto an item, the release chooses it
	GetControlBounds(g_controls[kMenuButton], &b);
	GetWindowBounds(g_window, kWindowContentRgn, &g_content);
	{
		// The menu opens at the right of the button (menuPlacement 4), its first item level with the button's top
		double mx = g_content.left + b.left + 30, my = g_content.top + (b.top + b.bottom) / 2;

		g_commandCount = 0;
		dragTo(mx, my, g_content.left + b.right + 25, g_content.top + b.top + 30, 8);
		// Opening the menu can take a while: its tracking ends with the release
		for (int i = 0; i < 100 && !g_menuEnded; i++)
			usleep(100000);
		g_menuChosen = 0;
		GetControlData(g_controls[kMenuButton], kControlEntireControl, kControlBevelButtonMenuValueTag, sizeof(SInt16), &g_menuChosen, NULL);
		g_menuCommands = g_commandCount;
	}
	g_phase = 6;
	g_done = 1;
	QuitApplicationEventLoop();
	return NULL;
}

static void on_alarm(int sig)
{
	printf("FAIL: timed out in phase %d (hits push %d toggle %d check %d, clicks push %d, slider %d/%d/%d, scroll arrow %d/%d page %d/%d ind %d->%d, menu %d, commands %d)\n", (int) g_phase,
		g_hits[kPush], g_hits[kToggle], g_hits[kCheck], g_clicks[kPush], g_sliderBefore, g_sliderAfterDrag, g_sliderAfterGroove,
		g_arrowCalls, g_arrowValue, g_pageCalls, g_pageValue, g_indicatorBefore, g_indicatorAfter, g_menuCommands, g_commandCount);
	fflush(stdout);
	_exit(1);
}

#ifdef __LP64__
#define GETMIN(c) HIViewGetMinimum(c)
#define GETMAX(c) HIViewGetMaximum(c)
#define SETMIN(c, v) HIViewSetMinimum((c), (v))
#define SETMAX(c, v) HIViewSetMaximum((c), (v))
#else
#define GETMIN(c) GetControl32BitMinimum(c)
#define GETMAX(c) GetControl32BitMaximum(c)
#define SETMIN(c, v) SetControl32BitMinimum((c), (v))
#define SETMAX(c, v) SetControl32BitMaximum((c), (v))
#endif

static void checkContract(void)
{
	ControlKind kind;
	ControlID id;
	ControlRef c;
	CFStringRef title = NULL;
	UInt32 command = 0;
	Rect bounds;

	// The controls of the nib: IDs, kinds, values, titles
	for (int i = 1; i <= 7; i++)
		CHECK((g_controls[i] = byID(i)) != NULL);
	CHECK_EQ(GetControlKind(g_controls[kPush], &kind), noErr);
	CHECK_EQ(kind.signature, 'appl');
	CHECK_EQ(kind.kind, 'push');
	GetControlKind(g_controls[kSlider], &kind);
	CHECK_EQ(kind.kind, 'sldr');
	GetControlKind(g_controls[kScroll], &kind);
	CHECK_EQ(kind.kind, 'sbar');
	GetControlKind(g_controls[kText], &kind);
	CHECK_EQ(kind.kind, 'stxt');
	GetControlKind(g_controls[kMenuButton], &kind);
	CHECK_EQ(kind.kind, 'bevl');
	GetControlKind(g_controls[kCheck], &kind);
	CHECK_EQ(kind.kind, 'cbox');
	CHECK_EQ(GETV(g_controls[kSlider]), 30);
	CHECK_EQ(GETMIN(g_controls[kScroll]), 0);
	CHECK_EQ(GETMAX(g_controls[kScroll]), 100);
	CHECK_EQ(CopyControlTitleAsCFString(g_controls[kPush], &title), noErr);
	CHECK(title && CFStringCompare(title, CFSTR("Push"), 0) == kCFCompareEqualTo);
	if (title)
		CFRelease(title);
	CHECK_EQ(GetControlCommandID(g_controls[kPush], &command), noErr);
	CHECK_EQ(command, 'push');
	CHECK_EQ(GetControlCommandID(g_controls[kCheck], &command), noErr);
	CHECK_EQ(command, 0);
	CHECK(GetControlOwner(g_controls[kPush]) == g_window);

	// IDs that are not there
	id = (ControlID) { kSig, 99 };
	c = (ControlRef) 1;
	CHECK_EQ(GetControlByID(g_window, &id, &c), errUnknownControl);
	CHECK(c == NULL);

	// Bounds are in the window's coordinates (the top left of the content is 0, 0)
	GetControlBounds(g_controls[kPush], &bounds);
	CHECK_EQ(bounds.left, 10);
	CHECK_EQ(bounds.top, 10);
	CHECK_EQ(bounds.right, 110);
	CHECK_EQ(bounds.bottom, 32);

	// The hierarchy: the controls are subviews of the root (or of its content view)
	HIViewRef root = HIViewGetRoot(g_window);
#ifdef __LP64__
	CHECK(root != NULL);
	HIViewRef content = NULL;
	CHECK_EQ(HIViewFindByID(root, kHIViewWindowContentID, &content), noErr);
	CHECK(content != NULL && HIViewGetSuperview(content) == root);
	CHECK(HIViewGetSuperview(g_controls[kPush]) == content);
	CHECK_EQ(HIViewFindByID(root, (HIViewID) { kSig, 3 }, &c), noErr);
	CHECK(c == g_controls[kSlider]);
#else
	CHECK(root != NULL);
	CHECK(HIViewGetSuperview(g_controls[kPush]) == root);
#endif
	CHECK(HIViewGetWindow(g_controls[kPush]) == g_window);

	// Frames and conversion
	HIRect frame, hb;
	CHECK_EQ(HIViewGetFrame(g_controls[kPush], &frame), noErr);
	CHECK(frame.origin.x == 10 && frame.origin.y == 10 && frame.size.width == 100 && frame.size.height == 22);
	CHECK_EQ(HIViewGetBounds(g_controls[kPush], &hb), noErr);
	CHECK(hb.origin.x == 0 && hb.origin.y == 0 && hb.size.width == 100 && hb.size.height == 22);
	HIPoint p = { 5, 6 };
	CHECK_EQ(HIViewConvertPoint(&p, g_controls[kPush], NULL), noErr);
	CHECK(p.x == 15 && p.y == 16);

	// Values: the range limits the value; shrinking the range moves it (test_quirk_value_clamped_to_range)
	SETV(g_controls[kSlider], 150);
	CHECK_EQ(GETV(g_controls[kSlider]), 100);
	SETV(g_controls[kSlider], -5);
	CHECK_EQ(GETV(g_controls[kSlider]), 0);
	SETV(g_controls[kSlider], 30);
	SETMAX(g_controls[kSlider], 20);
	CHECK_EQ(GETV(g_controls[kSlider]), 20);
	SETMAX(g_controls[kSlider], 100);
	SETV(g_controls[kSlider], 30);

	// Visibility, enabling, activation
	CHECK(HIViewIsVisible(g_controls[kText]));
	HIViewSetVisible(g_controls[kText], false);
	CHECK(!HIViewIsVisible(g_controls[kText]));
	HIViewSetVisible(g_controls[kText], true);
	CHECK(IsControlEnabled(g_controls[kPush]));
	DisableControl(g_controls[kPush]);
	CHECK(!IsControlEnabled(g_controls[kPush]));
	EnableControl(g_controls[kPush]);
	CHECK(IsControlEnabled(g_controls[kPush]));

	// Data of a control: the kind, an unknown tag, a wrong size
	CHECK_EQ(GetControlData(g_controls[kPush], kControlEntireControl, 'zzzz', 0, NULL, NULL), errDataNotSupported);
	{
		SInt16 value = 0;
		Size actual = 0;

		CHECK_EQ(GetControlData(g_controls[kMenuButton], kControlEntireControl, kControlBevelButtonMenuValueTag, sizeof(value), &value, &actual), noErr);
		CHECK_EQ(value, 1);
		CHECK_EQ(actual, sizeof(SInt16));
		value = 3;
		CHECK_EQ(SetControlData(g_controls[kMenuButton], kControlEntireControl, kControlBevelButtonMenuValueTag, sizeof(value), &value), noErr);
		value = 0;
		GetControlData(g_controls[kMenuButton], kControlEntireControl, kControlBevelButtonMenuValueTag, sizeof(value), &value, NULL);
		CHECK_EQ(value, 3);
		value = 1;
		SetControlData(g_controls[kMenuButton], kControlEntireControl, kControlBevelButtonMenuValueTag, sizeof(value), &value);
		CHECK_EQ(SetControlData(g_controls[kMenuButton], kControlEntireControl, kControlBevelButtonMenuValueTag, 1, &value), errDataSizeMismatch);
	}

	// The control that is at a point, and its part
	{
		Point where = { 20, 20 };
		ControlPartCode part = 0;

		c = FindControlUnderMouse(where, g_window, &part);
		CHECK(c == g_controls[kPush]);
		CHECK_EQ(part, kControlButtonPart);
		where = (Point) { 280, 200 };
		c = FindControlUnderMouse(where, g_window, &part);
		CHECK(c == NULL);
		// The parts of the scroll bar (value 0 of 0 to 100: the indicator is at the top)
		where = (Point) { 100, 307 };
		c = FindControlUnderMouse(where, g_window, &part);
		CHECK(c == g_controls[kScroll]);
		CHECK_EQ(part, kControlPageDownPart);
		where = (Point) { 205, 307 };
		FindControlUnderMouse(where, g_window, &part);
		CHECK_EQ(part, kControlDownButtonPart);
		where = (Point) { 60, 100 };
		c = FindControlUnderMouse(where, g_window, &part);
		CHECK(c == g_controls[kSlider]);
	}

	// Properties and refcon-like data
	{
		int data = 7, back = 0;
		ByteCount actual = 0;

		CHECK_EQ(SetControlProperty(g_controls[kPush], 'Test', 'prop', sizeof(data), &data), noErr);
		CHECK_EQ(GetControlProperty(g_controls[kPush], 'Test', 'prop', sizeof(back), &actual, &back), noErr);
		CHECK_EQ(back, 7);
		CHECK_EQ(RemoveControlProperty(g_controls[kPush], 'Test', 'prop'), noErr);
		CHECK_EQ(GetControlProperty(g_controls[kPush], 'Test', 'prop', sizeof(back), &actual, &back), errUnknownControl);
	}
}

static void checkSimulatedHit(void)
{
	ControlPartCode part = 0;

	g_commandCount = 0;
	g_hits[kPush] = 0;
	CHECK_EQ(HIViewSimulateClick(g_controls[kPush], kControlButtonPart, 0, &part), noErr);
	CHECK_EQ(part, kControlButtonPart);
	CHECK_EQ(g_hits[kPush], 1);
	CHECK_EQ(g_commandCount, 1);
	CHECK_EQ(g_commands[0], 'push');
	CHECK((g_commandAttributes[0] & kHICommandFromControl) != 0);

	// A disabled control gets no hit
	DisableControl(g_controls[kPush]);
	HIViewSimulateClick(g_controls[kPush], kControlButtonPart, 0, &part);
	CHECK_EQ(g_hits[kPush], 1);
	EnableControl(g_controls[kPush]);
}

// How the controls look: they draw into the window's pixels
static void checkDrawing(void)
{
	size_t width, height;
	unsigned char* before = readPixels(g_window, &width, &height);

	CHECK(before != NULL);
	if (!before)
		return;
	CHECK(width >= 400 && height >= 300);
	CHECK(inkIn(before, width, height, 10, 10, 110, 32) > 150);      // the push button
	CHECK(inkIn(before, width, height, 10, 80, 150, 100) > 10);      // the text
	CHECK(inkIn(before, width, height, 10, 50, 200, 70) > 100);      // the slider
	CHECK(inkIn(before, width, height, 300, 10, 315, 210) > 300);    // the scroll bar
	CHECK(inkIn(before, width, height, 10, 110, 110, 134) > 100);    // the bevel button
	CHECK(inkIn(before, width, height, 200, 200, 280, 280) == 0);    // where there is none
	g_draws = 0;

	// A change of the value shows without anything else being done
	SETV(g_controls[kSlider], 90);
	spin(300);
	unsigned char* after = readPixels(g_window, &width, &height);

	CHECK(after && differing(before, after, width, 10, 50, 200, 70) > 20);
	CHECK(after && differing(before, after, width, 10, 10, 110, 32) == 0);
	SETV(g_controls[kSlider], 30);

	// Hiding a control takes it from the window
	HIViewSetVisible(g_controls[kPush], false);
	spin(300);
	free(after);
	after = readPixels(g_window, &width, &height);
	CHECK(after && inkIn(after, width, height, 10, 10, 110, 32) == 0);
	HIViewSetVisible(g_controls[kPush], true);
	spin(300);
	free(after);
	after = readPixels(g_window, &width, &height);
	CHECK(after && inkIn(after, width, height, 10, 10, 110, 32) > 150);
	free(after);
	free(before);

	// Invalidating a view draws it again
	g_draws = 0;
	HIViewSetNeedsDisplay(g_controls[kCheck], true);
#ifdef __LP64__
	spin(300);
	CHECK(g_draws >= 1);
#endif
}


// QUIRK: a window that gets no update events (kWindowNoUpdatesAttribute) still shows its controls: the toolbox draws them.
// Not documented. Evidence: the standard behaviour that applications with such palettes rely on, tier 3 (inference)
// until confirmed on the Mac by this test: see test_quirk_no_updates_window_draws_controls.
static void test_quirk_no_updates_window_draws_controls(void)
{
	WindowRef w;
	ControlRef button = NULL, root = NULL;
	Rect bounds = { 300, 700, 400, 900 }, buttonRect = { 20, 20, 42, 120 };
	WindowAttributes attributes = kWindowNoUpdatesAttribute | kWindowStandardHandlerAttribute;

#ifdef __LP64__
	attributes |= kWindowCompositingAttribute;
#endif
	if (CreateNewWindow(kFloatingWindowClass, attributes, &bounds, &w) != noErr)
	{
		CHECK(0);
		return;
	}
#ifndef __LP64__
	CHECK_EQ(GetRootControl(w, &root), errNoRootControl);
	CHECK_EQ(CreateRootControl(w, &root), noErr);
	CHECK_EQ(CreateRootControl(w, &root), errRootAlreadyExists);
#endif
	CHECK_EQ(CreatePushButtonControl(w, &buttonRect, CFSTR("OK"), &button), noErr);
	ShowWindow(w);
	spin(600);

	size_t width, height;
	unsigned char* pixels = readPixels(w, &width, &height);

	CHECK(pixels != NULL);
	if (pixels)
	{
		CHECK(inkIn(pixels, width, height, 20, 20, 120, 42) > 100);
		free(pixels);
	}
	DisposeWindow(w);
}

int main(int argc, char** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	signal(SIGALRM, on_alarm);
	alarm(40);

	char root[] = "/tmp/carbon_controls.XXXXXX";

	if (!mkdtemp(root))
		return 1;

	CFBundleRef bundle = makeBundle(root, "Controls", kObjects);
	IBNibRef nib = NULL;

	CHECK(bundle != NULL);
	CHECK_EQ(CreateNibReferenceWithCFBundle(bundle, CFSTR("Controls"), &nib), noErr);
	CHECK_EQ(CreateWindowFromNib(nib, CFSTR("Missing"), &g_window), kIBCarbonRuntimeCantFindObject);
	CHECK_EQ(CreateWindowFromNib(nib, CFSTR("MainWindow"), &g_window), noErr);
	if (!g_window)
	{
		printf("FAIL: no window\n");
		return 1;
	}

	static const EventTypeSpec commandTypes[] = { { kEventClassCommand, kEventCommandProcess } };

	InstallApplicationEventHandler(NewEventHandlerUPP(commandHandler), 1, commandTypes, NULL, NULL);
	static const EventTypeSpec menuTypes[] = { { kEventClassMenu, kEventMenuEndTracking } };

	InstallApplicationEventHandler(NewEventHandlerUPP(menuHandler), 1, menuTypes, NULL, NULL);
	checkContract();

	// Handlers on the controls
	static const EventTypeSpec types[] = { { kEventClassControl, kEventControlHit }, { kEventClassControl, kEventControlClick }, { kEventClassControl, kEventControlDraw } };

	for (int i = 1; i <= 7; i++)
		InstallControlEventHandler(g_controls[i], NewEventHandlerUPP(controlHandler), 3, types, (void*) (long) i, NULL);
#ifndef __LP64__
	SetControlAction(g_controls[kScroll], actionProc);
#endif

	ShowWindow(g_window);
	spin(500);
	GetWindowBounds(g_window, kWindowContentRgn, &g_content);
	checkSimulatedHit();
	checkDrawing();
	test_quirk_no_updates_window_draws_controls();

	// The mouse
	pthread_t thread;

	g_done = 0;
	pthread_create(&thread, NULL, injector, NULL);
	g_hits[kPush] = 0;
	g_commandCount = 0;
	RunApplicationEventLoop();
	pthread_join(thread, NULL);

	// 1, 2: the push button
	CHECK_EQ(g_hits[kPush], 1);
	CHECK_EQ(g_clicks[kPush], 2);
	// 3: the toggling bevel button
	CHECK_EQ(g_toggleAfterFirst, 1);
	CHECK_EQ(GETV(g_controls[kToggle]), 0);
	CHECK_EQ(g_hits[kToggle], 2);
	// 4: the check box
	CHECK_EQ(GETV(g_controls[kCheck]), 1);
	CHECK_EQ(g_hits[kCheck], 1);
	// 5: the slider follows the pointer, and the groove puts the thumb at the pointer
	CHECK_EQ(g_sliderBefore, 30);
	CHECK(g_sliderAfterDrag >= 70 && g_sliderAfterDrag <= 100);
	CHECK(g_sliderAfterGroove <= 15);
	// 6: the scroll bar
	// (in 64-bit the standard actions of the bar move the value; the action function is set with SetControlAction in 32-bit)
	CHECK(g_arrowValue >= 3);
	CHECK(g_pageValue >= 5);
	CHECK(g_indicatorAfter > g_indicatorBefore);
#ifndef __LP64__
	CHECK(g_arrowCalls >= 3);
	CHECK(g_pageCalls >= 1);
	CHECK_EQ(g_pagePart, kControlPageDownPart);
	CHECK_EQ(g_indicatorPart, kControlIndicatorPart);
#endif
	// 7: the menu of the bevel button: the item chosen has its command, and is the button's menu value
	CHECK_EQ(g_menuCommands, 1);
	if (g_menuCommands == 1)
	{
		CHECK(g_menuChosen >= 1 && g_menuChosen <= 3);
		CHECK_EQ(g_commands[0], 'mni0' + g_menuChosen);
	}

	// A control that is disposed of with its window leaves the window
	DisposeWindow(g_window);

	printf("%s checks=%d failures=%d\n", g_failures ? "FAIL" : "PASS", g_checks, g_failures);
	return g_failures != 0;
}
