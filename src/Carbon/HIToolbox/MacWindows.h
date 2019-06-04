#ifndef MACWINDOWS_H
#define MACWINDOWS_H

#include <X11/Xlib.h>
#include "CoreServices/MacErrors.h"
#include "CoreServices/MacTypes.h"
#include <stdint.h>

#pragma pack(2)

#ifdef __cplusplus
extern "C" {
#endif

typedef int WindowRef; // mapped to X11 Window
typedef WindowRef HIWindowRef;
typedef int WindowGroupRef;
typedef void* CGrafPtr;

typedef uint32_t WindowClass;
typedef OptionBits WindowAttributes;
typedef OptionBits WindowGroupAttributes;
typedef uint32_t WindowRegionCode;

typedef Rect HIRect;
typedef void* TransitionWindowOptions;
typedef uint32_t WindowTransitionAction;
typedef uint32_t WindowTransitionEffect;

typedef struct RGBColor {
	uint16_t components[3];
} RGBColor;

OSStatus CreateNewWindow(WindowClass cls, WindowAttributes attr, const Rect* rect, WindowRef* newWindow);
void DisposeWindow(WindowRef wnd);

void ShowWindow(WindowRef wnd);
void HideWindow(WindowRef wnd);
void ShowHide(WindowRef wnd, Boolean showFlag);
Boolean IsWindowVisible(WindowRef wnd);


OSStatus SelectWindow(WindowRef a);
OSStatus SetWindowContentColor(WindowRef a, const RGBColor * b);
OSStatus SetWindowGroup(WindowRef a, WindowGroupRef b);
OSStatus SetWindowGroupLevel(WindowGroupRef a, SInt32 b);
OSStatus SetWindowTitleWithCFString(WindowRef a, CFStringRef b);
OSStatus TransitionWindowWithOptions(WindowRef a, WindowTransitionEffect b, WindowTransitionAction c, const HIRect * d, Boolean e, TransitionWindowOptions * f);
OSStatus CreateWindowGroup(WindowGroupAttributes a, WindowGroupRef *b);
OSStatus GetWindowBounds(WindowRef a, WindowRegionCode b, Rect * c);
OSStatus GetWindowEventTarget(WindowRef a);
OSStatus GetWindowGroupLevel(WindowGroupRef a, SInt32 * b);
WindowGroupRef GetWindowGroupOfClass(WindowClass a);
CGrafPtr GetWindowPort(WindowRef a);
Rect* GetWindowPortBounds(WindowRef window, Rect * bounds);
Boolean IsValidWindowPtr(WindowRef a);
OSStatus ReleaseWindow(WindowRef a);


#ifdef __cplusplus
}
#endif

#pragma pack()

#endif

