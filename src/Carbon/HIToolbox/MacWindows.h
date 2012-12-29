#ifndef MACWINDOWS_H
#define MACWINDOWS_H
//#include <X11/Xlib.h>
#include "../../CoreServices/MacErrors.h"
#include "../../CoreServices/MacTypes.h"
#include <stdint.h>

#pragma pack(2)

extern "C" {

typedef int WindowRef; // mapped to X11 Window
typedef WindowRef HIWindowRef;

typedef uint32_t WindowClass;
typedef OptionBits WindowAttributes;

struct Rect
{
	short top, left, bottom, right;
};

OSStatus CreateNewWindow(WindowClass cls, WindowAttributes attr, const Rect* rect, WindowRef* newWindow);
void DisposeWindow(WindowRef wnd);

void ShowWindow(WindowRef wnd);
void HideWindow(WindowRef wnd);
void ShowHide(WindowRef wnd, Boolean showFlag);
Boolean IsWindowVisible(WindowRef wnd);

}

namespace Darling {
namespace Carbon {

Display* getDisplay();

}} // end namespaces

#pragma pack()

#endif

