/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

// The Control Manager and HIView. The controls of a window are views in a hierarchy below the window's
// root view; a view has a frame in its superview's coordinates (the y axis points down), keeps a value,
// range, title and command ID, and has an event target of its own: the events of the Carbon Event Manager
// (kEventControlDraw, kEventControlClick, kEventControlHit, ...) go to the handlers an application installed on it
// and then to the standard handler of the control's class, installed first.
//
// The drawing of a window's views is done into the pixels of the window's content that the Window Manager keeps
// (MacWindows.c), with HITheme for the standard appearance. Invalidating a view asks the Window Manager for an
// update. The classic controls of a window that is not a compositing window (CreateRootControl()) are drawn when the
// application or the standard window handler asks (DrawControls()) and when their state changes.

#include <HIToolbox/HIView.h>
#include <HIToolbox/ControlDefinitions.h>
#include <HIToolbox/CarbonEvents.h>
#include <HIToolbox/HITheme.h>
#include <HIToolbox/Appearance.h>
#include <HIToolbox/Menus.h>
#include <CoreText/CoreText.h>
#include <CarbonCore/MacErrors.h>
#include "ControlsInternal.h"
#include "HIToolboxInternal.h"
#include "HIToolboxPrivate.h"
#include "MenusInternal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define kControlMagic 0x4354524c // 'CTRL'

enum {
	kClassRoot,
	kClassUserPane,
	kClassPushButton,
	kClassCheckBox,
	kClassRadioButton,
	kClassBevelButton,
	kClassSlider,
	kClassScrollBar,
	kClassStaticText,
	kClassGroupBox,
	kClassSeparator,
	kClassPopupButton,
	kClassProgressBar,
};

struct ControlProperty
{
	struct ControlProperty* next;
	OSType creator;
	OSType tag;
	ByteCount size;
	char data[];
};

struct OpaqueControlRef
{
	UInt32 magic;
	int cls;
	WindowRef window;
	struct OpaqueControlRef* parent;
	struct OpaqueControlRef* firstChild;   // the back of the children
	struct OpaqueControlRef* lastChild;    // the front
	struct OpaqueControlRef* prev;         // the sibling behind
	struct OpaqueControlRef* next;         // the sibling in front

	CGRect frame;          // in the bounds of the superview
	CGPoint boundsOrigin;
	CGFloat structureTop;   // root views: the height of what the window's frame has above the content (compositing windows)
	bool visible;
	bool enabled;
	bool activated;
	bool drawingEnabled;

	SInt32 value, min, max, viewSize;
	CFStringRef title;
	ControlID id;
	long refCon;
	UInt32 command;
	ControlPartCode hilite;
	ControlActionUPP action;
	ControlSize size;
	EventTargetRef target;
	struct ControlProperty* properties;

	// The data of the classes
	bool live;              // scroll bars and sliders track live
	UInt16 ticks;           // slider
	bool horizontal;        // slider
	bool autoToggle;        // check boxes and radio buttons
	bool showsArrows;       // scroll bars
	bool primary;           // group box
	bool indeterminate;     // progress bar
	// bevel buttons
	UInt16 bevelThickness, behavior, menuBehavior, menuPlacement;
	SInt16 menuID;
	MenuRef menu;
	bool menuOwned;
	SInt16 menuValue;
	ControlButtonContentInfo content;
	bool rounded;
	UInt32 features;        // user panes
	ControlFontStyleRec fontStyle;
	bool hasFontStyle;
	SInt16 popupTitleWidth;
};

// ------------------------------------------------------------------------------------------------
// Basics
// ------------------------------------------------------------------------------------------------

static bool isControl(ControlRef c)
{
	return c && c->magic == kControlMagic;
}

static bool isView(ControlRef c)
{
	return isControl(c);
}

static OSStatus controlHandler(EventHandlerCallRef call, EventRef event, void* userData);

static const EventTypeSpec kControlHandlerTypes[] = {
	{ kEventClassControl, kEventControlDraw },
	{ kEventClassControl, kEventControlHitTest },
	{ kEventClassControl, kEventControlClick },
	{ kEventClassControl, kEventControlTrack },
	{ kEventClassControl, kEventControlSimulateHit },
	{ kEventClassControl, kEventControlHit },
};

static CGFloat sizeFont(ControlSize size)
{
	switch (size)
	{
		case kControlSizeSmall: return 11;
		case kControlSizeMini: return 9;
		case kControlSizeLarge: return 15;
		default: return 13;
	}
}

static ControlRef newControl(int cls, CGRect frame)
{
	ControlRef c = calloc(1, sizeof(*c));

	if (!c)
		return NULL;
	c->magic = kControlMagic;
	c->cls = cls;
	c->frame = frame;
	c->visible = true;
	c->enabled = true;
	c->activated = true;
	c->drawingEnabled = true;
	// QUIRK: the size of a control that was not given one is kControlSizeAuto. Not documented. Evidence: black-box
	// observation on macOS 10.14.4 i386 (tier 2). Test: test_quirk_control_data_exact_size (the size tag)
	c->size = kControlSizeAuto;
	c->target = _HICreateEventTarget(NULL);
	if (!c->target)
	{
		free(c);
		return NULL;
	}
	InstallEventHandler(c->target, controlHandler, sizeof(kControlHandlerTypes) / sizeof(kControlHandlerTypes[0]),
		kControlHandlerTypes, c, NULL);
	return c;
}

static CGRect rectFromQD(const Rect* r)
{
	return CGRectMake(r->left, r->top, r->right - r->left, r->bottom - r->top);
}

static void rectToQD(CGRect r, Rect* out)
{
	out->left = (short) floor(r.origin.x + 0.5);
	out->top = (short) floor(r.origin.y + 0.5);
	out->right = (short) floor(r.origin.x + r.size.width + 0.5);
	out->bottom = (short) floor(r.origin.y + r.size.height + 0.5);
}

// The frame of the control in the coordinates of its window's content
static CGRect windowFrame(ControlRef c)
{
	CGRect r = c->frame;

	for (ControlRef p = c->parent; p; p = p->parent)
	{
		r.origin.x += p->frame.origin.x - p->boundsOrigin.x;
		r.origin.y += p->frame.origin.y - p->boundsOrigin.y;
	}
	// The root view of a compositing window covers the frame of the window too (its title bar); the coordinates of the
	// window's content start below it
	ControlRef top = c;

	while (top->parent)
		top = top->parent;
	r.origin.y -= top->structureTop;
	return r;
}

static void invalidateControl(ControlRef c)
{
	if (c->window && c->visible)
		_HIWindowInvalidateRect(c->window, windowFrame(c));
}

// Draws what is invalid now. A window that is not a compositing window shows the control as it changes.
static void changed(ControlRef c)
{
	if (!c->window)
		return;
	invalidateControl(c);
	_HIWindowRenderNow(c->window);
}

// The control and its superviews are visible
static bool latentVisible(ControlRef c)
{
	for (; c; c = c->parent)
	{
		if (!c->visible)
			return false;
	}
	return true;
}

static void setWindowOf(ControlRef c, WindowRef window)
{
	c->window = window;
	_HISetEventTargetNext(c->target, window ? GetWindowEventTarget(window) : NULL);
	for (ControlRef child = c->firstChild; child; child = child->next)
		setWindowOf(child, window);
}

static void sendNotification(ControlRef c, UInt32 kind, void (^addParameters)(EventRef event))
{
	EventRef event;

	if (CreateEvent(NULL, kEventClassControl, kind, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return;
	SetEventParameter(event, kEventParamDirectObject, typeControlRef, sizeof(c), &c);
	if (addParameters)
		addParameters(event);
	SendEventToEventTarget(event, c->target);
	ReleaseEvent(event);
}

static void windowChangedFor(ControlRef c)
{
	if (c->window)
		sendNotification(c, kEventControlOwningWindowChanged, NULL);
}

// ------------------------------------------------------------------------------------------------
// The hierarchy
// ------------------------------------------------------------------------------------------------

static void unlinkControl(ControlRef c)
{
	ControlRef parent = c->parent;

	if (!parent)
		return;
	if (c->prev)
		c->prev->next = c->next;
	else
		parent->firstChild = c->next;
	if (c->next)
		c->next->prev = c->prev;
	else
		parent->lastChild = c->prev;
	c->parent = c->prev = c->next = NULL;
}

static void linkControl(ControlRef parent, ControlRef c, ControlRef before /* NULL: in front */)
{
	c->parent = parent;
	c->prev = before ? before->prev : parent->lastChild;
	c->next = before;
	if (c->prev)
		c->prev->next = c;
	else
		parent->firstChild = c;
	if (before)
		before->prev = c;
	else
		parent->lastChild = c;
}

static bool isAncestor(ControlRef ancestor, ControlRef c)
{
	for (; c; c = c->parent)
	{
		if (c == ancestor)
			return true;
	}
	return false;
}

static bool canEmbed(ControlRef c)
{
	return c->cls == kClassRoot || c->cls == kClassGroupBox || c->cls == kClassUserPane;
}

OSStatus HIViewAddSubview(HIViewRef inParent, HIViewRef inNewChild)
{
	if (!isView(inParent) || !isView(inNewChild))
		return paramErr;
	if (inParent == inNewChild || isAncestor(inNewChild, inParent) || inNewChild->cls == kClassRoot)
		return paramErr;

	if (inNewChild->parent)
	{
		invalidateControl(inNewChild);
		unlinkControl(inNewChild);
	}
	linkControl(inParent, inNewChild, NULL);
	setWindowOf(inNewChild, inParent->window);
	sendNotification(inParent, kEventControlAddedSubControl, ^(EventRef event) {
		ControlRef sub = inNewChild;
		SetEventParameter(event, kEventParamControlSubControl, typeControlRef, sizeof(sub), &sub);
	});
	windowChangedFor(inNewChild);
	invalidateControl(inNewChild);
	return noErr;
}

OSStatus HIViewRemoveFromSuperview(HIViewRef inView)
{
	if (!isView(inView))
		return paramErr;
	if (!inView->parent)
		return noErr;

	ControlRef parent = inView->parent;

	sendNotification(parent, kEventControlRemovingSubControl, ^(EventRef event) {
		ControlRef sub = inView;
		SetEventParameter(event, kEventParamControlSubControl, typeControlRef, sizeof(sub), &sub);
	});
	invalidateControl(inView);
	unlinkControl(inView);
	setWindowOf(inView, NULL);
	return noErr;
}

HIViewRef HIViewGetSuperview(HIViewRef inView)
{
	return isView(inView) ? inView->parent : NULL;
}

HIViewRef HIViewGetFirstSubview(HIViewRef inView)
{
	return isView(inView) ? inView->lastChild : NULL;
}

#ifndef __LP64__
HIViewRef HIViewGetLastSubview(HIViewRef inView)
{
	return isView(inView) ? inView->firstChild : NULL;
}
#endif

HIViewRef HIViewGetNextView(HIViewRef inView)
{
	return isView(inView) ? inView->prev : NULL;
}

#ifndef __LP64__
HIViewRef HIViewGetPreviousView(HIViewRef inView)
{
	return isView(inView) ? inView->next : NULL;
}
#endif

OSStatus HIViewSetZOrder(HIViewRef inView, HIViewZOrderOp inOp, HIViewRef inOther)
{
	if (!isView(inView) || !inView->parent)
		return paramErr;
	if (inOther && (!isView(inOther) || inOther->parent != inView->parent || inOther == inView))
		return paramErr;

	ControlRef parent = inView->parent;

	unlinkControl(inView);
	if (inOp == kHIViewZOrderAbove)
		linkControl(parent, inView, inOther ? inOther->next : NULL);
	else if (inOp == kHIViewZOrderBelow)
		linkControl(parent, inView, inOther ? inOther : parent->firstChild);
	else
	{
		linkControl(parent, inView, NULL);
		return paramErr;
	}
	invalidateControl(inView);
	return noErr;
}

WindowRef HIViewGetWindow(HIViewRef inView)
{
	return isView(inView) ? inView->window : NULL;
}

WindowRef GetControlOwner(ControlRef control)
{
	return isControl(control) ? control->window : NULL;
}

HIViewRef HIViewGetRoot(WindowRef inWindow)
{
	return IsValidWindowPtr(inWindow) ? _HIWindowGetRootControl(inWindow) : NULL;
}

OSStatus GetRootControl(WindowRef inWindow, ControlRef* outControl)
{
	if (!outControl)
		return paramErr;
	if (!IsValidWindowPtr(inWindow))
		return errInvalidWindowPtr;
	*outControl = _HIWindowGetRootControl(inWindow);
	return *outControl ? noErr : errNoRootControl;
}

OSStatus CreateRootControl(WindowRef inWindow, ControlRef* outControl)
{
	if (!outControl)
		return paramErr;
	*outControl = NULL;
	if (!IsValidWindowPtr(inWindow))
		return errInvalidWindowPtr;
	if (_HIWindowGetRootControl(inWindow))
		return errRootAlreadyExists;

	CGRect content = _HIWindowGetContentRect(inWindow);
	CGFloat structureTop = _HIWindowIsCompositing(inWindow) ? _HIWindowStructureTop(inWindow) : 0;
	ControlRef root = newControl(kClassRoot, CGRectMake(0, 0, content.size.width, content.size.height + structureTop));

	if (root)
		root->structureTop = structureTop;

	if (!root)
		return memFullErr;
	root->window = inWindow;
	_HISetEventTargetNext(root->target, GetWindowEventTarget(inWindow));
	root->id = (ControlID) { 0, 0 };
	root->features = kControlSupportsEmbedding;
	_HIWindowSetRootControl(inWindow, root);
	*outControl = root;
	return noErr;
}

const HIViewID kHIViewWindowContentID = { 'wind', 1 };

void _HIControlsWindowCreated(WindowRef window)
{
	// A compositing window has its root view, with a content view that fills it
	if (!_HIWindowIsCompositing(window))
		return;

	ControlRef root;

	if (CreateRootControl(window, &root) != noErr)
		return;

	ControlRef content = newControl(kClassUserPane, CGRectMake(0, root->structureTop, root->frame.size.width, root->frame.size.height - root->structureTop));

	if (!content)
		return;
	content->id = kHIViewWindowContentID;
	content->features = kControlSupportsEmbedding;
	linkControl(root, content, NULL);
	setWindowOf(content, window);
}

static void disposeTree(ControlRef c)
{
	while (c->lastChild)
		disposeTree(c->lastChild);
	sendNotification(c, kEventControlDispose, NULL);
	unlinkControl(c);
	while (c->properties)
	{
		struct ControlProperty* p = c->properties;

		c->properties = p->next;
		free(p);
	}
	if (c->title)
		CFRelease(c->title);
	if (c->menu && c->menuOwned)
		CFRelease((CFTypeRef) c->menu);
	_HIDestroyEventTarget(c->target);
	c->magic = 0;
	free(c);
}

void _HIControlsWindowDisposing(WindowRef window)
{
	ControlRef root = _HIWindowGetRootControl(window);

	if (root)
	{
		_HIWindowSetRootControl(window, NULL);
		disposeTree(root);
	}
}

static void disposeOne(ControlRef c)
{
	WindowRef window = c->window;
	CGRect where = windowFrame(c);

	if (window && c->cls == kClassRoot)
		_HIWindowSetRootControl(window, NULL);
	disposeTree(c);
	if (window && IsValidWindowPtr(window))
		_HIWindowInvalidateRect(window, where);
}

#ifndef __LP64__
void DisposeControl(ControlRef theControl)
{
	if (isControl(theControl))
		disposeOne(theControl);
}
#endif

void _HIControlsWindowResized(WindowRef window, int width, int height)
{
	ControlRef root = _HIWindowGetRootControl(window);

	if (!root)
		return;
	root->frame.size = CGSizeMake(width, height + root->structureTop);
	// The content view of a compositing window follows
	for (ControlRef c = root->firstChild; c; c = c->next)
	{
		if (c->id.signature == kHIViewWindowContentID.signature && c->id.id == kHIViewWindowContentID.id)
			c->frame = CGRectMake(0, root->structureTop, width, height);
	}
}

void _HIControlsWindowActivationChanged(WindowRef window)
{
	ControlRef root = _HIWindowGetRootControl(window);

	if (root)
		_HIWindowInvalidateRect(window, windowFrame(root));
}

// ------------------------------------------------------------------------------------------------
// IDs and finding
// ------------------------------------------------------------------------------------------------

static ControlRef findByID(ControlRef c, const ControlID* id)
{
	if (c->id.signature == id->signature && c->id.id == id->id && !(c->cls == kClassRoot))
		return c;
	for (ControlRef child = c->lastChild; child; child = child->prev)
	{
		ControlRef found = findByID(child, id);

		if (found)
			return found;
	}
	return NULL;
}

OSStatus HIViewFindByID(HIViewRef inStartView, HIViewID inID, HIViewRef* outControl)
{
	if (!outControl)
		return paramErr;
	*outControl = NULL;
	if (!isView(inStartView))
		return paramErr;

	// The content view of a window that has only the classic root is the root itself
	if (inID.signature == kHIViewWindowContentID.signature && inID.id == kHIViewWindowContentID.id)
	{
		ControlRef found = findByID(inStartView, &inID);

		*outControl = found ? found : (inStartView->cls == kClassRoot ? inStartView : NULL);
		return *outControl ? noErr : errUnknownControl;
	}
	*outControl = findByID(inStartView, &inID);
	return *outControl ? noErr : errUnknownControl;
}

OSStatus GetControlByID(WindowRef inWindow, const ControlID* inID, ControlRef* outControl)
{
	if (!inID || !outControl)
		return paramErr;
	*outControl = NULL;
	if (!IsValidWindowPtr(inWindow))
		return errInvalidWindowPtr;

	ControlRef root = _HIWindowGetRootControl(inWindow);

	if (!root)
		return errNoRootControl;
	return HIViewFindByID(root, *inID, outControl);
}

#ifndef __LP64__
OSStatus GetControlID(ControlRef inControl, ControlID* outID)
{
	if (!isControl(inControl) || !outID)
		return paramErr;
	*outID = inControl->id;
	return noErr;
}

OSStatus SetControlID(ControlRef inControl, const ControlID* inID)
{
	if (!isControl(inControl) || !inID)
		return paramErr;
	inControl->id = *inID;
	return noErr;
}
#endif

// ------------------------------------------------------------------------------------------------
// Geometry
// ------------------------------------------------------------------------------------------------

OSStatus HIViewGetBounds(HIViewRef inView, HIRect* outRect)
{
	if (!isView(inView) || !outRect)
		return paramErr;
	*outRect = CGRectMake(inView->boundsOrigin.x, inView->boundsOrigin.y, inView->frame.size.width, inView->frame.size.height);
	return noErr;
}

OSStatus HIViewGetFrame(HIViewRef inView, HIRect* outRect)
{
	if (!isView(inView) || !outRect)
		return paramErr;
	*outRect = inView->frame;
	return noErr;
}

OSStatus HIViewSetFrame(HIViewRef inView, const HIRect* inRect)
{
	if (!isView(inView) || !inRect)
		return paramErr;
	if (CGRectEqualToRect(inView->frame, *inRect))
		return noErr;

	CGRect previous = inView->frame;

	invalidateControl(inView);
	inView->frame = *inRect;
	invalidateControl(inView);
	sendNotification(inView, kEventControlBoundsChanged, ^(EventRef event) {
		HIRect original = CGRectMake(inView->boundsOrigin.x, inView->boundsOrigin.y, previous.size.width, previous.size.height);
		HIRect current = CGRectMake(inView->boundsOrigin.x, inView->boundsOrigin.y, inRect->size.width, inRect->size.height);
		UInt32 attributes = 0;

		if (previous.origin.x != inRect->origin.x || previous.origin.y != inRect->origin.y)
			attributes |= 1 << 1; // kControlBoundsChangePositionChanged
		if (previous.size.width != inRect->size.width || previous.size.height != inRect->size.height)
			attributes |= 1 << 2; // kControlBoundsChangeSizeChanged
		SetEventParameter(event, kEventParamAttributes, typeUInt32, sizeof(attributes), &attributes);
		SetEventParameter(event, kEventParamOriginalBounds, typeHIRect, sizeof(original), &original);
		SetEventParameter(event, kEventParamPreviousBounds, typeHIRect, sizeof(original), &original);
		SetEventParameter(event, kEventParamCurrentBounds, typeHIRect, sizeof(current), &current);
	});
	if (inView->window)
		_HIWindowRenderNow(inView->window);
	return noErr;
}

OSStatus HIViewMoveBy(HIViewRef inView, CGFloat inDX, CGFloat inDY)
{
	if (!isView(inView))
		return paramErr;

	CGRect frame = inView->frame;

	frame.origin.x += inDX;
	frame.origin.y += inDY;
	return HIViewSetFrame(inView, &frame);
}

OSStatus HIViewSetBoundsOrigin(HIViewRef inView, CGFloat inX, CGFloat inY)
{
	if (!isView(inView))
		return paramErr;
	inView->boundsOrigin = CGPointMake(inX, inY);
	changed(inView);
	return noErr;
}

OSStatus HIViewConvertPoint(HIPoint* ioPoint, HIViewRef inSourceView, HIViewRef inDestView)
{
	if (!ioPoint)
		return paramErr;
	if ((inSourceView && !isView(inSourceView)) || (inDestView && !isView(inDestView)))
		return paramErr;

	// A NULL view is the root view of the window (the coordinates start at the top of the window's frame)
	CGPoint p = *ioPoint;
	WindowRef window = inSourceView ? inSourceView->window : (inDestView ? inDestView->window : NULL);
	ControlRef root = window ? _HIWindowGetRootControl(window) : NULL;
	CGFloat rootTop = root ? root->structureTop : 0;

	if (!inSourceView)
		p.y -= rootTop;

	if (inSourceView)
	{
		CGRect f = windowFrame(inSourceView);

		p.x += f.origin.x - inSourceView->boundsOrigin.x;
		p.y += f.origin.y - inSourceView->boundsOrigin.y;
	}
	if (inDestView)
	{
		CGRect f = windowFrame(inDestView);

		p.x -= f.origin.x - inDestView->boundsOrigin.x;
		p.y -= f.origin.y - inDestView->boundsOrigin.y;
	}
	if (!inDestView)
		p.y += rootTop;
	*ioPoint = p;
	return noErr;
}

OSStatus HIViewConvertRect(HIRect* ioRect, HIViewRef inSourceView, HIViewRef inDestView)
{
	if (!ioRect)
		return paramErr;

	HIPoint origin = ioRect->origin;
	OSStatus status = HIViewConvertPoint(&origin, inSourceView, inDestView);

	if (status == noErr)
		ioRect->origin = origin;
	return status;
}

Rect* GetControlBounds(ControlRef control, Rect* bounds)
{
	if (!bounds)
		return NULL;
	if (!isControl(control))
	{
		*bounds = (Rect) { 0, 0, 0, 0 };
		return bounds;
	}
	rectToQD(windowFrame(control), bounds);
	return bounds;
}

#ifndef __LP64__
void SetControlBounds(ControlRef control, const Rect* bounds)
{
	if (!isControl(control) || !bounds)
		return;

	CGRect r = rectFromQD(bounds);

	// The rectangle is in the window's coordinates: the frame is in the superview's
	if (control->parent)
	{
		CGRect parent = windowFrame(control->parent);

		r.origin.x -= parent.origin.x - control->parent->boundsOrigin.x;
		r.origin.y -= parent.origin.y - control->parent->boundsOrigin.y;
	}
	HIViewSetFrame(control, &r);
}

void MoveControl(ControlRef theControl, SInt16 h, SInt16 v)
{
	if (!isControl(theControl))
		return;

	Rect bounds;

	GetControlBounds(theControl, &bounds);
	OffsetRect(&bounds, h - bounds.left, v - bounds.top);
	SetControlBounds(theControl, &bounds);
}

void SizeControl(ControlRef theControl, SInt16 w, SInt16 h)
{
	if (!isControl(theControl))
		return;

	Rect bounds;

	GetControlBounds(theControl, &bounds);
	bounds.right = bounds.left + w;
	bounds.bottom = bounds.top + h;
	SetControlBounds(theControl, &bounds);
}
#endif

// ------------------------------------------------------------------------------------------------
// Visibility, state
// ------------------------------------------------------------------------------------------------

OSStatus HIViewSetVisible(HIViewRef inView, Boolean inVisible)
{
	if (!isView(inView))
		return paramErr;
	if (inView->visible == (inVisible != 0))
		return noErr;
	if (!inVisible)
		invalidateControl(inView);
	inView->visible = inVisible != 0;
	if (inVisible)
		invalidateControl(inView);
	sendNotification(inView, kEventControlVisibilityChanged, NULL);
	if (inView->window)
		_HIWindowRenderNow(inView->window);
	return noErr;
}

Boolean HIViewIsVisible(HIViewRef inView)
{
	// A view is visible when it and its superviews are and the window is shown
	return isView(inView) && latentVisible(inView) && (!inView->window || _HIWindowIsShown(inView->window));
}

Boolean IsControlVisible(ControlRef inControl)
{
	return isControl(inControl) && inControl->visible;
}

#ifndef __LP64__
void ShowControl(ControlRef theControl)
{
	HIViewSetVisible(theControl, true);
}

void HideControl(ControlRef theControl)
{
	HIViewSetVisible(theControl, false);
}
#endif

OSStatus HIViewSetEnabled(HIViewRef inView, Boolean inEnabled)
{
	if (!isView(inView))
		return paramErr;
	if (inView->enabled == (inEnabled != 0))
		return noErr;
	inView->enabled = inEnabled != 0;
	sendNotification(inView, kEventControlEnabledStateChanged, NULL);
	changed(inView);
	return noErr;
}

Boolean HIViewIsEnabled(HIViewRef inView)
{
	return isView(inView) && inView->enabled;
}

OSStatus EnableControl(ControlRef inControl)
{
	return HIViewSetEnabled(inControl, true);
}

OSStatus DisableControl(ControlRef inControl)
{
	return HIViewSetEnabled(inControl, false);
}

#ifndef __LP64__
Boolean IsControlEnabled(ControlRef inControl)
{
	return HIViewIsEnabled(inControl);
}
#endif

OSStatus HIViewSetActivated(HIViewRef inView, Boolean inSetActivated)
{
	if (!isView(inView))
		return paramErr;
	if (inView->activated != (inSetActivated != 0))
	{
		inView->activated = inSetActivated != 0;
		sendNotification(inView, inSetActivated ? kEventControlActivate : kEventControlDeactivate, NULL);
		changed(inView);
	}
	return noErr;
}

Boolean HIViewIsActive(HIViewRef inView, Boolean* outIsLatentActive)
{
	if (!isView(inView))
		return false;

	bool latent = inView->activated;

	if (outIsLatentActive)
		*outIsLatentActive = latent;
	return latent && (!inView->window || _HIWindowControlsActive(inView->window));
}

OSStatus ActivateControl(ControlRef inControl)
{
	return HIViewSetActivated(inControl, true);
}

OSStatus DeactivateControl(ControlRef inControl)
{
	return HIViewSetActivated(inControl, false);
}

Boolean IsControlActive(ControlRef inControl)
{
	return HIViewIsActive(inControl, NULL);
}

OSStatus HIViewSetDrawingEnabled(HIViewRef inView, Boolean inEnabled)
{
	if (!isView(inView))
		return paramErr;
	inView->drawingEnabled = inEnabled != 0;
	return noErr;
}

Boolean HIViewIsDrawingEnabled(HIViewRef inView)
{
	return isView(inView) && inView->drawingEnabled;
}

OSStatus HIViewSetHilite(HIViewRef inView, HIViewPartCode inHilitePart)
{
	if (!isView(inView))
		return paramErr;
	if (inView->hilite != inHilitePart)
	{
		inView->hilite = inHilitePart;
		sendNotification(inView, kEventControlHiliteChanged, NULL);
		changed(inView);
	}
	return noErr;
}

#ifndef __LP64__
void HiliteControl(ControlRef theControl, ControlPartCode hiliteState)
{
	HIViewSetHilite(theControl, hiliteState);
}
#endif

OSStatus HIViewSetNeedsDisplay(HIViewRef inView, Boolean inNeedsDisplay)
{
	if (!isView(inView))
		return paramErr;
	if (inNeedsDisplay)
		invalidateControl(inView);
	return noErr;
}

OSStatus HIViewSetNeedsDisplayInRect(HIViewRef inView, const HIRect* inRect, Boolean inNeedsDisplay)
{
	if (!isView(inView) || !inRect)
		return paramErr;
	if (inNeedsDisplay && inView->window && inView->visible)
	{
		CGRect r = *inRect;
		CGRect f = windowFrame(inView);

		r.origin.x += f.origin.x - inView->boundsOrigin.x;
		r.origin.y += f.origin.y - inView->boundsOrigin.y;
		r = CGRectIntersection(r, f);
		if (!CGRectIsNull(r))
			_HIWindowInvalidateRect(inView->window, r);
	}
	return noErr;
}

OSStatus HIViewRender(HIViewRef inView)
{
	if (!isView(inView))
		return paramErr;
	if (inView->window)
		_HIWindowRenderNow(inView->window);
	return noErr;
}

// ------------------------------------------------------------------------------------------------
// Values
// ------------------------------------------------------------------------------------------------

static void valueChanged(ControlRef c)
{
	sendNotification(c, kEventControlValueFieldChanged, NULL);
	changed(c);
}

SInt32 HIViewGetValue(HIViewRef inView)
{
	return isControl(inView) ? inView->value : 0;
}

#ifndef __LP64__
SInt32 GetControl32BitValue(ControlRef theControl)
{
	return HIViewGetValue(theControl);
}
#endif

static SInt32 clampValue(ControlRef c, SInt32 v)
{
	if (c->max >= c->min)
		return v < c->min ? c->min : (v > c->max ? c->max : v);
	return v;
}

OSStatus HIViewSetValue(HIViewRef inView, SInt32 inValue)
{
	if (!isView(inView))
		return paramErr;
	inValue = clampValue(inView, inValue);
	if (inView->value != inValue)
	{
		inView->value = inValue;
		valueChanged(inView);
	}
	return noErr;
}

SInt32 HIViewGetMinimum(HIViewRef inView)
{
	return isView(inView) ? inView->min : 0;
}

SInt32 HIViewGetMaximum(HIViewRef inView)
{
	return isView(inView) ? inView->max : 0;
}

OSStatus HIViewSetMinimum(HIViewRef inView, SInt32 inValue)
{
	if (!isView(inView))
		return paramErr;
	if (inView->min == inValue)
		return noErr;
	inView->min = inValue;
	if (inView->value < inValue)
		inView->value = inValue;
	valueChanged(inView);
	return noErr;
}

OSStatus HIViewSetMaximum(HIViewRef inView, SInt32 inValue)
{
	if (!isView(inView))
		return paramErr;
	if (inView->max == inValue)
		return noErr;
	inView->max = inValue;
	if (inView->value > inValue && inValue >= inView->min)
		inView->value = inValue;
	valueChanged(inView);
	return noErr;
}

SInt16 GetControlValue(ControlRef theControl)
{
	return (SInt16) HIViewGetValue(theControl);
}

void SetControlValue(ControlRef theControl, SInt16 newValue)
{
	HIViewSetValue(theControl, newValue);
}

#ifndef __LP64__
void SetControl32BitValue(ControlRef theControl, SInt32 newValue)
{
	HIViewSetValue(theControl, newValue);
}

SInt16 GetControlMinimum(ControlRef theControl)
{
	return (SInt16) HIViewGetMinimum(theControl);
}

void SetControlMinimum(ControlRef theControl, SInt16 newMinimum)
{
	HIViewSetMinimum(theControl, newMinimum);
}

SInt16 GetControlMaximum(ControlRef theControl)
{
	return (SInt16) HIViewGetMaximum(theControl);
}

void SetControlMaximum(ControlRef theControl, SInt16 newMaximum)
{
	HIViewSetMaximum(theControl, newMaximum);
}

SInt32 GetControlViewSize(ControlRef theControl)
{
	return isControl(theControl) ? theControl->viewSize : 0;
}

void SetControlViewSize(ControlRef theControl, SInt32 newViewSize)
{
	if (isControl(theControl) && theControl->viewSize != newViewSize)
	{
		theControl->viewSize = newViewSize;
		changed(theControl);
	}
}

SInt32 GetControlReference(ControlRef theControl)
{
	return isControl(theControl) ? (SInt32) theControl->refCon : 0;
}

void SetControlReference(ControlRef theControl, SInt32 data)
{
	if (isControl(theControl))
		theControl->refCon = data;
}
#endif

#ifndef __LP64__
SInt32 GetControl32BitMinimum(ControlRef theControl)
{
	return HIViewGetMinimum(theControl);
}

void SetControl32BitMinimum(ControlRef theControl, SInt32 newMinimum)
{
	HIViewSetMinimum(theControl, newMinimum);
}

SInt32 GetControl32BitMaximum(ControlRef theControl)
{
	return HIViewGetMaximum(theControl);
}

void SetControl32BitMaximum(ControlRef theControl, SInt32 newMaximum)
{
	HIViewSetMaximum(theControl, newMaximum);
}
#endif

// ------------------------------------------------------------------------------------------------
// Title, command, properties, kind
// ------------------------------------------------------------------------------------------------

OSStatus SetControlTitleWithCFString(ControlRef inControl, CFStringRef inString)
{
	if (!isControl(inControl))
		return paramErr;

	CFStringRef copy = inString ? CFStringCreateCopy(NULL, inString) : NULL;

	if (inControl->title)
		CFRelease(inControl->title);
	inControl->title = copy;
	sendNotification(inControl, kEventControlTitleChanged, NULL);
	changed(inControl);
	return noErr;
}

OSStatus CopyControlTitleAsCFString(ControlRef inControl, CFStringRef* outString)
{
	if (!isControl(inControl) || !outString)
		return paramErr;
	*outString = CFRetain(inControl->title ? inControl->title : CFSTR(""));
	return noErr;
}

#ifndef __LP64__
void SetControlTitle(ControlRef theControl, ConstStr255Param title)
{
	if (!isControl(theControl))
		return;

	CFStringRef string = title ? CFStringCreateWithPascalString(NULL, title, kCFStringEncodingMacRoman) : NULL;

	SetControlTitleWithCFString(theControl, string);
	if (string)
		CFRelease(string);
}

void GetControlTitle(ControlRef theControl, Str255 title)
{
	title[0] = 0;
	if (isControl(theControl) && theControl->title)
		CFStringGetPascalString(theControl->title, title, 256, kCFStringEncodingMacRoman);
}
#endif

OSStatus SetControlCommandID(ControlRef inControl, UInt32 inCommandID)
{
	if (!isControl(inControl))
		return paramErr;
	inControl->command = inCommandID;
	return noErr;
}

#ifndef __LP64__
OSStatus GetControlCommandID(ControlRef inControl, UInt32* outCommandID)
{
	if (!isControl(inControl) || !outCommandID)
		return paramErr;
	*outCommandID = inControl->command;
	return noErr;
}
#endif

OSStatus HIViewSetCommandID(HIViewRef inView, UInt32 inCommandID)
{
	return SetControlCommandID(inView, inCommandID);
}

OSStatus HIViewGetCommandID(HIViewRef inView, UInt32* outCommandID)
{
	if (!isView(inView) || !outCommandID)
		return paramErr;
	*outCommandID = inView->command;
	return noErr;
}

OSStatus SetControlProperty(ControlRef control, OSType creator, OSType tag, ByteCount size, const void* data)
{
	if (!isControl(control) || (size && !data))
		return paramErr;

	struct ControlProperty** p = &control->properties;

	while (*p && ((*p)->creator != creator || (*p)->tag != tag))
		p = &(*p)->next;
	if (*p && (*p)->size != size)
	{
		struct ControlProperty* old = *p;

		*p = old->next;
		free(old);
	}
	if (!*p)
	{
		struct ControlProperty* property = calloc(1, sizeof(*property) + size);

		if (!property)
			return memFullErr;
		property->creator = creator;
		property->tag = tag;
		property->size = size;
		property->next = control->properties;
		control->properties = property;
		p = &control->properties;
	}
	memcpy((*p)->data, data, size);
	return noErr;
}

OSStatus GetControlProperty(ControlRef control, OSType creator, OSType tag, ByteCount bufferSize, ByteCount* actualSize, void* buffer)
{
	if (!isControl(control))
		return paramErr;
	for (struct ControlProperty* p = control->properties; p; p = p->next)
	{
		if (p->creator == creator && p->tag == tag)
		{
			if (actualSize)
				*actualSize = p->size;
			if (bufferSize < p->size)
				return errDataSizeMismatch;
			memcpy(buffer, p->data, p->size);
			return noErr;
		}
	}
	// QUIRK: a property that is not there is errWindowPropertyNotFound, as for windows. Not documented. Evidence:
	// black-box observation on macOS 10.14.4 i386 (tier 2). Test: test_quirk_control_property_not_found
	return errWindowPropertyNotFound;
}

OSStatus RemoveControlProperty(ControlRef control, OSType creator, OSType tag)
{
	if (!isControl(control))
		return paramErr;
	for (struct ControlProperty** p = &control->properties; *p; p = &(*p)->next)
	{
		if ((*p)->creator == creator && (*p)->tag == tag)
		{
			struct ControlProperty* old = *p;

			*p = old->next;
			free(old);
			return noErr;
		}
	}
	return errWindowPropertyNotFound;
}

static const struct { int cls; OSType kind; } kKinds[] = {
	{ kClassRoot, 'root' }, { kClassUserPane, 'upan' }, { kClassPushButton, 'push' }, { kClassCheckBox, 'cbox' },
	{ kClassRadioButton, 'rdio' }, { kClassBevelButton, 'bevl' }, { kClassSlider, 'sldr' }, { kClassScrollBar, 'sbar' },
	{ kClassStaticText, 'stxt' }, { kClassGroupBox, 'grpb' }, { kClassSeparator, 'sepa' }, { kClassPopupButton, 'popb' },
	{ kClassProgressBar, 'prgb' },
};

OSStatus GetControlKind(ControlRef inControl, ControlKind* outControlKind)
{
	if (!isControl(inControl) || !outControlKind)
		return paramErr;
	for (size_t i = 0; i < sizeof(kKinds) / sizeof(kKinds[0]); i++)
	{
		if (kKinds[i].cls == inControl->cls)
		{
			outControlKind->signature = 'appl';
			outControlKind->kind = kKinds[i].kind;
			return noErr;
		}
	}
	return errUnknownControl;
}

// QUIRK: the feature bits GetControlFeatures() reports for the standard controls are the ones of the system's
// controls: push button 0x01020250, check box 0x01024240, radio button 0x01024a40, slider and scroll bar 0x00820641,
// static text 0x00020240, bevel button 0x01020240; a user pane has the features it was made with and data access.
// Not documented as a list. Evidence: black-box observation on macOS 10.14.4 i386 (tier 2, the probe of the Control
// Manager, CHECK in test_quirk_control_features of tests/src/carbon_controls.c).
// Test: test_quirk_control_features
static UInt32 featuresOf(ControlRef c)
{
	switch (c->cls)
	{
		case kClassRoot: return kControlSupportsEmbedding;
		case kClassUserPane: return c->features | kControlSupportsDataAccess;
		case kClassGroupBox: return kControlSupportsEmbedding | kControlSupportsDataAccess;
		case kClassPushButton: return 0x01020250;
		case kClassBevelButton: return 0x01020240;
		case kClassPopupButton: return 0x01020240;
		case kClassCheckBox: return 0x01024240;
		case kClassRadioButton: return 0x01024a40;
		case kClassSlider:
		case kClassScrollBar: return 0x00820641;
		case kClassStaticText: return 0x00020240;
		default: return kControlSupportsDataAccess;
	}
}

OSStatus GetControlFeatures(ControlRef inControl, UInt32* outFeatures)
{
	if (!isControl(inControl) || !outFeatures)
		return paramErr;
	*outFeatures = featuresOf(inControl);
	return noErr;
}

// ------------------------------------------------------------------------------------------------
// Embedding
// ------------------------------------------------------------------------------------------------

#ifndef __LP64__
OSStatus EmbedControl(ControlRef inControl, ControlRef inContainer)
{
	if (!isControl(inControl) || !isControl(inContainer))
		return paramErr;
	if (!canEmbed(inContainer))
		return errControlIsNotEmbedder;

	// The control stays where it is in the window: its frame is made relative to the container
	CGRect window = windowFrame(inControl);
	CGRect container = windowFrame(inContainer);
	OSStatus status = HIViewAddSubview(inContainer, inControl);

	if (status == noErr)
	{
		inControl->frame.origin.x = window.origin.x - container.origin.x + inContainer->boundsOrigin.x;
		inControl->frame.origin.y = window.origin.y - container.origin.y + inContainer->boundsOrigin.y;
	}
	return status;
}
#endif

// The view that holds the controls made in a window: the content view of a compositing window, the root otherwise
static ControlRef embeddingRoot(WindowRef window)
{
	ControlRef root = _HIWindowGetRootControl(window);

	if (!root)
		return NULL;
	for (ControlRef c = root->firstChild; c; c = c->next)
	{
		if (c->id.signature == kHIViewWindowContentID.signature && c->id.id == kHIViewWindowContentID.id)
			return c;
	}
	return root;
}

#ifndef __LP64__
OSStatus AutoEmbedControl(ControlRef inControl, WindowRef inWindow)
{
	if (!isControl(inControl))
		return paramErr;
	if (!IsValidWindowPtr(inWindow))
		return errInvalidWindowPtr;

	ControlRef container = embeddingRoot(inWindow);

	if (!container)
		return errNoRootControl;

	// The innermost embedder that holds the control's rectangle
	CGRect target = windowFrame(inControl);

	for (bool again = true; again;)
	{
		again = false;
		for (ControlRef c = container->lastChild; c; c = c->prev)
		{
			if (c != inControl && canEmbed(c) && c->visible && CGRectContainsRect(windowFrame(c), target))
			{
				container = c;
				again = true;
				break;
			}
		}
	}
	return EmbedControl(inControl, container);
}
#endif

#ifndef __LP64__
OSStatus CountSubControls(ControlRef inControl, UInt16* outNumChildren)
{
	if (!isControl(inControl) || !outNumChildren)
		return paramErr;
	if (!canEmbed(inControl))
		return errControlIsNotEmbedder;

	UInt16 n = 0;

	for (ControlRef c = inControl->firstChild; c; c = c->next)
		n++;
	*outNumChildren = n;
	return noErr;
}
#endif

OSStatus GetIndexedSubControl(ControlRef inControl, UInt16 inIndex, ControlRef* outSubControl)
{
	if (!isControl(inControl) || !outSubControl)
		return paramErr;
	if (!canEmbed(inControl))
		return errControlIsNotEmbedder;

	// Index 1 is the control in front, as the order of GetIndexedSubControl is of the controls' list
	UInt16 i = 1;

	for (ControlRef c = inControl->lastChild; c; c = c->prev, i++)
	{
		if (i == inIndex)
		{
			*outSubControl = c;
			return noErr;
		}
	}
	// QUIRK: an index with no control is paramErr. Not documented. Evidence: black-box observation on macOS 10.14.4
	// i386 (tier 2). Test: test_quirk_indexed_subcontrol_out_of_range
	return paramErr;
}

OSStatus GetSuperControl(ControlRef inControl, ControlRef* outParent)
{
	if (!isControl(inControl) || !outParent)
		return paramErr;
	*outParent = inControl->parent;
	return inControl->parent ? noErr : errControlIsNotEmbedder;
}

// ------------------------------------------------------------------------------------------------
// Event targets and events
// ------------------------------------------------------------------------------------------------

EventTargetRef GetControlEventTarget(ControlRef inControl)
{
	return isControl(inControl) ? inControl->target : NULL;
}

// ------------------------------------------------------------------------------------------------
// Drawing
// ------------------------------------------------------------------------------------------------

static bool windowIsActive(ControlRef c)
{
	return !c->window || _HIWindowControlsActive(c->window);
}

static ThemeDrawState drawStateOf(ControlRef c, bool pressed)
{
	bool active = c->activated && windowIsActive(c);

	if (!c->enabled)
		return active ? kThemeStateUnavailable : kThemeStateUnavailableInactive;
	if (!active)
		return kThemeStateInactive;
	return pressed ? kThemeStatePressed : kThemeStateActive;
}

static CTFontRef copyFont(ControlRef c, CGFloat* sizeOut)
{
	CGFloat size = sizeFont(c->size);

	if (c->hasFontStyle && (c->fontStyle.flags & (1 << 1)) && c->fontStyle.size > 0)
		size = c->fontStyle.size;
	if (sizeOut)
		*sizeOut = size;

	CTFontRef font = CTFontCreateUIFontForLanguage(c->size == kControlSizeNormal ? kCTFontUIFontSystem : kCTFontUIFontSmallSystem, size, NULL);

	if (!font)
		font = CTFontCreateWithName(CFSTR("Helvetica"), size, NULL);
	return font;
}

// Draws the string in the rectangle of the flipped context. just: -1 left, 0 center, 1 right.
static void drawString(CGContextRef ctx, CFStringRef string, CGRect rect, int just, ControlRef c, bool dim)
{
	if (!string || CFStringGetLength(string) == 0)
		return;

	CTFontRef font = copyFont(c, NULL);
	CGColorSpaceRef space = CGColorSpaceCreateDeviceGray();
	CGFloat comps[2] = { dim ? 0.5 : 0, 1 };
	CGColorRef color = CGColorCreate(space, comps);
	CFStringRef keys[2] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
	CFTypeRef values[2] = { font, color };
	CFDictionaryRef attrs = CFDictionaryCreate(NULL, (const void**) keys, (const void**) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFAttributedStringRef attributed = CFAttributedStringCreate(NULL, string, attrs);
	CTLineRef line = CTLineCreateWithAttributedString(attributed);
	CGFloat ascent, descent, leading;
	double width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
	CGFloat x = rect.origin.x;

	if (just == 0)
		x += (rect.size.width - width) / 2;
	else if (just > 0)
		x += rect.size.width - width;

	CGFloat baseline = rect.origin.y + (rect.size.height + ascent - descent) / 2;

	CGContextSaveGState(ctx);
	CGContextClipToRect(ctx, rect);
	CGContextSetTextMatrix(ctx, CGAffineTransformMakeScale(1, -1));
	CGContextSetTextPosition(ctx, x, baseline);
	CTLineDraw(line, ctx);
	CGContextRestoreGState(ctx);
	CFRelease(line);
	CFRelease(attributed);
	CFRelease(attrs);
	CGColorRelease(color);
	CGColorSpaceRelease(space);
	CFRelease(font);
}

static CGRect localRect(ControlRef c)
{
	return CGRectMake(c->boundsOrigin.x, c->boundsOrigin.y, c->frame.size.width, c->frame.size.height);
}

static void drawBackgroundOf(ControlRef c, CGContextRef ctx, CGRect rect)
{
	CGColorRef color = c->window ? _HIWindowCopyBackgroundColor(c->window) : NULL;

	if (color)
	{
		CGContextSetFillColorWithColor(ctx, color);
		CGContextFillRect(ctx, rect);
		CGColorRelease(color);
	}
}

static bool bevelHasMenu(ControlRef c)
{
	return c->cls == kClassBevelButton && c->menu != NULL;
}

#ifndef __LP64__
// QuickDraw pictures: the 'PICT' resource GetPicture() reads and DrawPicture() plays into the current port (they are
// QuickDraw calls that the public QuickDraw header of this tree does not declare yet; a picture handle is a Handle)
Handle GetPicture(short pictureID);
void DrawPicture(Handle picture, const Rect* dstRect);
void GetPort(GrafPtr* port);
void SetPort(GrafPtr port);
RgnHandle NewRgn(void);
void DisposeRgn(RgnHandle region);
void ClipRect(const Rect* rect);
void GetClip(RgnHandle region);
void SetClip(RgnHandle region);

// The picture of a bevel button (a resource, or a handle the application keeps), drawn at its own size in the middle
// of the button, into the window's port
static void drawBevelPicture(ControlRef c, CGRect rect)
{
	Handle picture = c->content.contentType == kControlContentPictRes ? GetPicture(c->content.u.resID) : c->content.u.picture;

	if (!picture || !*picture || GetHandleSize(picture) < 10)
		return;

	// The picture's frame follows its size field: top, left, bottom and right, big endian
	const SInt16* frame = (const SInt16*) ((const char*) *picture + 2);
	int height = (int) (SInt16) CFSwapInt16BigToHost(frame[2]) - (int) (SInt16) CFSwapInt16BigToHost(frame[0]);
	int width = (int) (SInt16) CFSwapInt16BigToHost(frame[3]) - (int) (SInt16) CFSwapInt16BigToHost(frame[1]);
	CGRect where = windowFrame(c);
	Rect dst;
	GrafPtr saved = NULL, port = (GrafPtr) GetWindowPort(c->window);

	if (width <= 0 || height <= 0 || !port)
		return;
	dst.left = (SInt16) (where.origin.x + rect.origin.x + (rect.size.width - width) / 2);
	dst.top = (SInt16) (where.origin.y + rect.origin.y + (rect.size.height - height) / 2);
	dst.right = dst.left + width;
	dst.bottom = dst.top + height;
	GetPort(&saved);
	SetPort(port);

	// A picture does not draw outside the button
	RgnHandle clip = NewRgn();
	Rect bounds;

	GetClip(clip);
	bounds.left = (SInt16) where.origin.x;
	bounds.top = (SInt16) where.origin.y;
	bounds.right = (SInt16) (where.origin.x + where.size.width);
	bounds.bottom = (SInt16) (where.origin.y + where.size.height);
	ClipRect(&bounds);
	DrawPicture(picture, &dst);
	SetClip(clip);
	DisposeRgn(clip);
	SetPort(saved);
}
#endif

static void drawBevelContent(ControlRef c, CGContextRef ctx, CGRect rect, bool dim)
{
	if (c->content.contentType == kControlContentCGImageRef && c->content.u.imageRef)
	{
		CGImageRef image = c->content.u.imageRef;
		CGFloat w = CGImageGetWidth(image), h = CGImageGetHeight(image);
		CGRect to = CGRectMake(rect.origin.x + (rect.size.width - w) / 2, rect.origin.y + (rect.size.height - h) / 2, w, h);

		CGContextSaveGState(ctx);
		CGContextTranslateCTM(ctx, 0, to.origin.y * 2 + h);
		CGContextScaleCTM(ctx, 1, -1);
		CGContextDrawImage(ctx, to, image);
		CGContextRestoreGState(ctx);
	}
#ifndef __LP64__
	else if (c->window && (c->content.contentType == kControlContentPictRes || c->content.contentType == kControlContentPictHandle))
		drawBevelPicture(c, rect);
#endif
	if (c->title)
		drawString(ctx, c->title, rect, 0, c, dim);
}

static void drawButtonLike(ControlRef c, CGContextRef ctx)
{
	CGRect bounds = localRect(c);
	bool pressed = c->hilite != 0 && c->hilite != kControlInactivePart && c->hilite != kControlDisabledPart;
	HIThemeButtonDrawInfo info;
	HIRect label = bounds;
	bool dim = !c->enabled;

	memset(&info, 0, sizeof(info));
	info.version = 0;
	info.state = drawStateOf(c, pressed);
	info.value = c->value == 0 ? kThemeButtonOff : (c->value == 2 && (c->cls == kClassCheckBox || c->cls == kClassRadioButton) ? kThemeButtonMixed : kThemeButtonOn);
	info.adornment = kThemeAdornmentNone;

	switch (c->cls)
	{
		case kClassPushButton:
			info.kind = c->size == kControlSizeSmall ? kThemePushButtonSmall : kThemePushButton;
			break;
		case kClassCheckBox:
			info.kind = kThemeCheckBox;
			break;
		case kClassRadioButton:
			info.kind = kThemeRadioButton;
			break;
		case kClassPopupButton:
			info.kind = kThemePopupButton;
			break;
		default:
			if (c->rounded)
				info.kind = kThemeRoundedBevelButton;
			else if (c->bevelThickness == 1 || bounds.size.height <= 16)
				info.kind = kThemeBevelButtonSmall;
			else if (c->bevelThickness == 3)
				info.kind = kThemeBevelButtonLarge;
			else
				info.kind = kThemeBevelButton;
			// A bevel button is on while its value is, when it toggles or sticks; with a menu the value is the menu's
			if (!(c->behavior & (kControlBehaviorToggles | kControlBehaviorSticky)) || c->menu)
				info.value = kThemeButtonOff;
			break;
	}
	if (c->cls == kClassBevelButton && (c->behavior & (kControlBehaviorToggles | kControlBehaviorSticky)) && c->value && info.state == kThemeStateActive)
		info.state = kThemeStatePressed;
	if (c->cls == kClassPopupButton || c->cls == kClassBevelButton)
	{
		if (c->cls == kClassPopupButton)
			info.adornment = kThemeAdornmentNone;
	}
	HIThemeDrawButton(&bounds, &info, ctx, kHIThemeOrientationNormal, &label);

	if (c->cls == kClassCheckBox || c->cls == kClassRadioButton || c->cls == kClassPushButton || c->cls == kClassPopupButton)
	{
		CFStringRef title = c->title;

		if (c->cls == kClassPopupButton && c->menu)
		{
			// The title of a pop-up button is its current item
			static __thread CFStringRef item;

			(void) item;
			title = c->title;
			if (!title && c->value > 0 && c->value <= (SInt32) CountMenuItems(c->menu))
			{
				CFStringRef text = NULL;

				CopyMenuItemTextAsCFString(c->menu, (MenuItemIndex) c->value, &text);
				drawString(ctx, text, label, c->cls == kClassPopupButton ? -1 : 0, c, dim);
				if (text)
					CFRelease(text);
				return;
			}
		}
		drawString(ctx, title, label, (c->cls == kClassCheckBox || c->cls == kClassRadioButton) ? -1 : (c->cls == kClassPopupButton ? -1 : 0), c, dim);
	}
	else
	{
		drawBevelContent(c, ctx, CGRectInset(bounds, 3, 2), dim);
		if (bevelHasMenu(c))
		{
			HIThemePopupArrowDrawInfo arrow = { 0, info.state, kThemeArrowDown, kThemeArrow5pt };
			HIRect arrowRect;

			if (c->menuPlacement == kControlBevelButtonMenuOnRight)
				arrowRect = CGRectMake(bounds.origin.x + bounds.size.width - 12, bounds.origin.y + (bounds.size.height - 6) / 2, 8, 6);
			else
				arrowRect = CGRectMake(bounds.origin.x + (bounds.size.width - 8) / 2, bounds.origin.y + bounds.size.height - 8, 8, 6);
			if (c->menuPlacement == kControlBevelButtonMenuOnRight)
				arrow.orientation = kThemeArrowRight;
			HIThemeDrawPopupArrow(&arrowRect, &arrow, ctx, kHIThemeOrientationNormal);
		}
	}
}

static void trackInfoOf(ControlRef c, HIThemeTrackDrawInfo* info)
{
	CGRect bounds = localRect(c);
	bool horizontal = c->cls == kClassScrollBar ? bounds.size.width >= bounds.size.height : c->horizontal;

	memset(info, 0, sizeof(*info));
	info->version = 0;
	info->bounds = CGRectMake(0, 0, bounds.size.width, bounds.size.height);
	info->min = c->min;
	info->max = c->max;
	info->value = c->value;
	info->attributes = (horizontal ? kThemeTrackHorizontal : 0) | kThemeTrackShowThumb;
	bool active = c->activated && windowIsActive(c);

	if (!c->enabled)
		info->enableState = kThemeTrackDisabled;
	else if (!active)
		info->enableState = kThemeTrackInactive;
	else if (c->cls == kClassScrollBar && c->max <= c->min)
		info->enableState = kThemeTrackNothingToScroll;
	else
		info->enableState = kThemeTrackActive;

	if (c->cls == kClassScrollBar)
	{
		CGFloat thickness = horizontal ? bounds.size.height : bounds.size.width;

		info->kind = thickness <= 12 ? kThemeScrollBarSmall : kThemeScrollBarMedium;
		// The arrows of a scroll bar are hidden unless the control asks for them (kControlScrollBarShowsArrowsTag)
		if (!c->showsArrows)
			info->attributes |= kThemeTrackNoScrollBarArrows;
		info->trackInfo.scrollbar.viewsize = c->viewSize;
		switch (c->hilite)
		{
			case kControlUpButtonPart: info->trackInfo.scrollbar.pressState = kThemeLeftOutsideArrowPressed; break;
			case kControlDownButtonPart: info->trackInfo.scrollbar.pressState = kThemeRightOutsideArrowPressed; break;
			case kControlPageUpPart: info->trackInfo.scrollbar.pressState = kThemeLeftTrackPressed; break;
			case kControlPageDownPart: info->trackInfo.scrollbar.pressState = kThemeRightTrackPressed; break;
			case kControlIndicatorPart: info->trackInfo.scrollbar.pressState = kThemeThumbPressed; break;
			default: break;
		}
	}
	else if (c->cls == kClassSlider)
	{
		CGFloat thickness = horizontal ? bounds.size.height : bounds.size.width;

		info->kind = thickness <= 16 ? kThemeSliderSmall : kThemeSliderMedium;
		info->trackInfo.slider.thumbDir = c->ticks ? kThemeThumbDownward : kThemeThumbPlain;
		if (c->hilite == kControlIndicatorPart)
			info->trackInfo.slider.pressState = kThemeThumbPressed;
	}
	else
	{
		info->kind = bounds.size.height <= 10 ? kThemeProgressBarMini : kThemeProgressBarMedium;
		if (c->indeterminate)
			info->kind = kThemeIndeterminateBarMedium;
	}
}

static OSStatus defaultDraw(ControlRef c, CGContextRef ctx)
{
	CGRect bounds = localRect(c);

	switch (c->cls)
	{
		case kClassRoot:
			drawBackgroundOf(c, ctx, bounds);
			break;
		case kClassPushButton:
		case kClassCheckBox:
		case kClassRadioButton:
		case kClassBevelButton:
		case kClassPopupButton:
			drawButtonLike(c, ctx);
			break;
		case kClassSlider:
		case kClassScrollBar:
		case kClassProgressBar:
		{
			HIThemeTrackDrawInfo info;

			trackInfoOf(c, &info);
			// The track is drawn in a frame whose origin is the view's origin
			CGContextSaveGState(ctx);
			CGContextTranslateCTM(ctx, bounds.origin.x, bounds.origin.y);
			HIThemeDrawTrack(&info, NULL, ctx, kHIThemeOrientationNormal);
			if (c->cls == kClassSlider && c->ticks > 1)
				HIThemeDrawTrackTickMarks(&info, c->ticks, ctx, kHIThemeOrientationNormal);
			CGContextRestoreGState(ctx);
			break;
		}
		case kClassStaticText:
		{
			int just = -1;

			if (c->hasFontStyle && (c->fontStyle.flags & (1 << 3)))
				just = c->fontStyle.just == 0 ? -1 : (c->fontStyle.just == 1 ? 0 : 1);
			drawString(ctx, c->title, bounds, just, c, !c->enabled);
			break;
		}
		case kClassGroupBox:
		{
			HIThemeGroupBoxDrawInfo info = { 0, drawStateOf(c, false), c->primary ? kHIThemeGroupBoxKindPrimary : kHIThemeGroupBoxKindSecondary };
			CGRect box = bounds;

			if (c->title)
			{
				box.origin.y += 8;
				box.size.height -= 8;
			}
			HIThemeDrawGroupBox(&box, &info, ctx, kHIThemeOrientationNormal);
			if (c->title)
				drawString(ctx, c->title, CGRectMake(bounds.origin.x + 12, bounds.origin.y, bounds.size.width - 24, 16), -1, c, !c->enabled);
			break;
		}
		case kClassSeparator:
		{
			HIThemeSeparatorDrawInfo info = { 0, drawStateOf(c, false) };

			HIThemeDrawSeparator(&bounds, &info, ctx, kHIThemeOrientationNormal);
			break;
		}
		default:
			break;
	}
	return noErr;
}

// Draws the view and its subviews, the parts that intersect the rectangle (window coordinates)
static void drawView(ControlRef c, CGContextRef ctx, CGRect dirty, CGRect clip, bool skipOwnDrawing)
{
	if (!c->visible)
		return;

	CGRect frame = windowFrame(c);
	CGRect visible = CGRectIntersection(frame, clip);

	if (CGRectIsNull(visible))
		return;

	CGRect part = CGRectIntersection(visible, dirty);

	if (CGRectIsNull(part) || CGRectIsEmpty(part))
		return;
	if (c->drawingEnabled && !skipOwnDrawing)
	{
		CGContextSaveGState(ctx);
		CGContextClipToRect(ctx, part);
		CGContextTranslateCTM(ctx, frame.origin.x - c->boundsOrigin.x, frame.origin.y - c->boundsOrigin.y);

		EventRef event;

		if (CreateEvent(NULL, kEventClassControl, kEventControlDraw, _HICurrentTime(), kEventAttributeNone, &event) == noErr)
		{
			ControlPartCode entire = kControlEntireControl;

			SetEventParameter(event, kEventParamDirectObject, typeControlRef, sizeof(c), &c);
			SetEventParameter(event, kEventParamCGContextRef, typeCGContextRef, sizeof(ctx), &ctx);
			SetEventParameter(event, kEventParamControlPart, typeControlPartCode, sizeof(entire), &entire);
			SendEventToEventTarget(event, c->target);
			ReleaseEvent(event);
		}
		CGContextRestoreGState(ctx);
	}
	for (ControlRef child = c->firstChild; child; child = child->next)
		drawView(child, ctx, dirty, visible, false);
}

void _HIControlsDrawWindow(WindowRef window, CGContextRef context, CGRect dirty)
{
	ControlRef root = _HIWindowGetRootControl(window);

	if (!root)
		return;
	// A window that is not a compositing window shows its application's background: the root draws none
	drawView(root, context, dirty, windowFrame(root), !_HIWindowIsCompositing(window));
}

// Draws the controls of a classic window now, whole or what intersects a rectangle (window coordinates)
static void drawControlsNow(ControlRef root, CGRect rect, ControlRef only)
{
	WindowRef window = root->window;
	CGContextRef ctx = _HIWindowBeginDrawing(window);

	if (!ctx)
		return;
	if (only)
		drawView(only, ctx, CGRectIntersection(rect, windowFrame(only)), windowFrame(root), false);
	else
		drawView(root, ctx, rect, windowFrame(root), true);
	_HIWindowEndDrawing(window, ctx, rect);
}

#ifndef __LP64__
void DrawControls(WindowRef theWindow)
{
	ControlRef root = IsValidWindowPtr(theWindow) ? _HIWindowGetRootControl(theWindow) : NULL;

	if (root)
		drawControlsNow(root, windowFrame(root), NULL);
}

void UpdateControls(WindowRef inWindow, RgnHandle inRegion)
{
	ControlRef root = IsValidWindowPtr(inWindow) ? _HIWindowGetRootControl(inWindow) : NULL;

	if (!root)
		return;

	Rect box;

	if (inRegion)
		GetRegionBounds(inRegion, &box);
	else
		rectToQD(windowFrame(root), &box);
	drawControlsNow(root, rectFromQD(&box), NULL);
}

void Draw1Control(ControlRef theControl)
{
	if (isControl(theControl) && theControl->window && _HIWindowGetRootControl(theControl->window))
		drawControlsNow(_HIWindowGetRootControl(theControl->window), windowFrame(theControl), theControl);
}
#endif

// ------------------------------------------------------------------------------------------------
// Hit testing
// ------------------------------------------------------------------------------------------------

static CGPoint toLocal(ControlRef c, CGPoint windowPoint)
{
	CGRect f = windowFrame(c);

	return CGPointMake(windowPoint.x - f.origin.x + c->boundsOrigin.x, windowPoint.y - f.origin.y + c->boundsOrigin.y);
}

static ControlPartCode scrollBarPartAt(ControlRef c, CGPoint local)
{
	HIThemeTrackDrawInfo info;
	ControlPartCode part = 0;
	HIPoint p = CGPointMake(local.x - c->boundsOrigin.x, local.y - c->boundsOrigin.y);

	trackInfoOf(c, &info);
	if (HIThemeHitTestTrack(&info, &p, &part))
		return part;
	return 0;
}

static ControlPartCode defaultHitTest(ControlRef c, CGPoint local)
{
	CGRect bounds = localRect(c);

	if (!CGRectContainsPoint(bounds, local))
		return kControlNoPart;
	switch (c->cls)
	{
		case kClassPushButton:
		case kClassBevelButton:
		case kClassPopupButton:
			return c->cls == kClassPopupButton ? kControlMenuPart : kControlButtonPart;
		case kClassCheckBox:
		case kClassRadioButton:
			return kControlCheckBoxPart;
		case kClassSlider:
		{
			HIThemeTrackDrawInfo info;
			ControlPartCode part = 0;
			HIPoint p = CGPointMake(local.x - bounds.origin.x, local.y - bounds.origin.y);

			trackInfoOf(c, &info);
			if (HIThemeHitTestTrack(&info, &p, &part))
				return part == kAppearancePartIndicator ? kControlIndicatorPart : part;
			return kControlIndicatorPart;
		}
		case kClassScrollBar:
			return scrollBarPartAt(c, local);
		default:
			return kControlNoPart;
	}
}

// The part of the control a point (in the window's content) hits; the control's own handlers decide
static OSStatus hitTestControl(ControlRef c, CGPoint windowPoint, ControlPartCode* outPart)
{
	EventRef event;
	CGPoint local = toLocal(c, windowPoint);

	*outPart = kControlNoPart;
	if (CreateEvent(NULL, kEventClassControl, kEventControlHitTest, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return memFullErr;
	SetEventParameter(event, kEventParamDirectObject, typeControlRef, sizeof(c), &c);
	SetEventParameter(event, kEventParamMouseLocation, typeHIPoint, sizeof(local), &local);

	OSStatus status = SendEventToEventTarget(event, c->target);
	ControlPartCode part = kControlNoPart;

	if (GetEventParameter(event, kEventParamControlPart, typeControlPartCode, NULL, sizeof(part), NULL, &part) == noErr)
		*outPart = part;
	ReleaseEvent(event);
	return status;
}

// The deepest visible control at a point of the window's content
static ControlRef hitView(ControlRef c, CGPoint p, ControlPartCode* outPart, bool deep)
{
	if (!c->visible)
		return NULL;

	CGRect frame = windowFrame(c);

	if (!CGRectContainsPoint(frame, p))
		return NULL;
	for (ControlRef child = c->lastChild; child; child = child->prev)
	{
		ControlRef found = hitView(child, p, outPart, deep);

		if (found)
			return found;
	}
	if (c->cls == kClassRoot || (c->id.signature == kHIViewWindowContentID.signature && c->id.id == kHIViewWindowContentID.id))
		return NULL;

	ControlPartCode part;

	hitTestControl(c, p, &part);
	// QUIRK: FindControlUnderMouse() returns the topmost visible control that contains the point also when the point
	// is in no part of it (static text, a user pane: part 0), and a disabled control with the part 0. Not documented.
	// Evidence: black-box observation on macOS 10.14.4 i386 (tier 2). Test: test_quirk_find_control_returns_inert_controls
	if (outPart)
		*outPart = c->enabled ? part : kControlNoPart;
	return c;
}

#ifndef __LP64__
OSStatus HIViewGetSubviewHit(HIViewRef inView, const HIPoint* inPoint, Boolean inDeep, HIViewRef* outView)
{
	if (!isView(inView) || !inPoint || !outView)
		return paramErr;

	// The point is in the coordinates of the view
	CGRect f = windowFrame(inView);
	CGPoint p = CGPointMake(inPoint->x + f.origin.x - inView->boundsOrigin.x, inPoint->y + f.origin.y - inView->boundsOrigin.y);

	*outView = NULL;
	for (ControlRef child = inView->lastChild; child; child = child->prev)
	{
		ControlRef found = hitView(child, p, NULL, inDeep);

		if (found)
		{
			*outView = found;
			break;
		}
	}
	return noErr;
}
#endif

OSStatus HIViewGetViewForMouseEvent(HIViewRef inView, EventRef inEvent, HIViewRef* outView)
{
	if (!isView(inView) || !inEvent || !outView)
		return paramErr;

	HIPoint location;
	CGRect f = windowFrame(inView);

	*outView = NULL;
	if (GetEventParameter(inEvent, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof(location), NULL, &location) != noErr)
		return eventParameterNotFoundErr;

	ControlRef found = hitView(inView, CGPointMake(location.x, location.y), NULL, true);

	(void) f;
	*outView = found ? found : (inView->cls == kClassRoot ? NULL : inView);
	return noErr;
}

// The control at a point of the window's content (the window's coordinates)
ControlRef FindControlUnderMouse(Point inWhere, WindowRef inWindow, ControlPartCode* outPart)
{
	ControlRef root = IsValidWindowPtr(inWindow) ? _HIWindowGetRootControl(inWindow) : NULL;
	ControlPartCode part = kControlNoPart;

	if (outPart)
		*outPart = kControlNoPart;
	if (!root)
		return NULL;

	ControlRef found = hitView(root, CGPointMake(inWhere.h, inWhere.v), &part, true);

	if (found && outPart)
		*outPart = part;
	return found;
}

#ifndef __LP64__
ControlPartCode FindControl(Point testPoint, WindowRef theWindow, ControlRef* theControl)
{
	ControlRef root = IsValidWindowPtr(theWindow) ? _HIWindowGetRootControl(theWindow) : NULL;
	ControlPartCode part = kControlNoPart;

	if (theControl)
		*theControl = NULL;
	if (!root)
		return kControlNoPart;

	ControlRef found = hitView(root, CGPointMake(testPoint.h, testPoint.v), &part, true);

	if (theControl)
		*theControl = found;
	return found ? part : kControlNoPart;
}

ControlPartCode TestControl(ControlRef theControl, Point testPoint)
{
	ControlPartCode part;

	if (!isControl(theControl) || !theControl->visible)
		return kControlNoPart;
	hitTestControl(theControl, CGPointMake(testPoint.h, testPoint.v), &part);
	return theControl->enabled || part == kControlNoPart ? part : kControlDisabledPart;
}
#endif

// ------------------------------------------------------------------------------------------------
// Clicking and tracking
// ------------------------------------------------------------------------------------------------

static OSStatus sendHit(ControlRef c, ControlPartCode part, UInt32 modifiers)
{
	EventRef event;
	OSStatus status;

	if (CreateEvent(NULL, kEventClassControl, kEventControlHit, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return eventNotHandledErr;
	SetEventParameter(event, kEventParamDirectObject, typeControlRef, sizeof(c), &c);
	SetEventParameter(event, kEventParamControlPart, typeControlPartCode, sizeof(part), &part);
	SetEventParameter(event, kEventParamKeyModifiers, typeUInt32, sizeof(modifiers), &modifiers);
	status = SendEventToEventTarget(event, c->target);
	ReleaseEvent(event);
	return status;
}

// The mouse events of a tracking loop. Returns false when the event queue ended; kind is 0 for a timeout.
static Boolean mouseFilter(EventRef event, void* info)
{
	return GetEventClass(event) == kEventClassMouse;
}

static bool nextMouseEvent(ControlRef c, EventTimeout timeout, UInt32* kind, CGPoint* windowPoint)
{
	EventRef event;
	OSStatus status = _HIReceiveNextEventMatching(mouseFilter, NULL, NULL, timeout, true, &event);

	*kind = 0;
	if (status == eventLoopTimedOutErr)
		return true;
	if (status != noErr)
		return status != eventLoopQuitErr;

	HIPoint global;

	*kind = GetEventKind(event);
	if (GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(global), NULL, &global) == noErr)
	{
		CGRect content = _HIWindowGetContentRect(c->window);

		*windowPoint = CGPointMake(global.x - content.origin.x, global.y - content.origin.y);
	}
	ReleaseEvent(event);
	return true;
}

static bool isMouseUp(UInt32 kind)
{
	return kind == kEventMouseUp;
}

static void callAction(ControlRef c, ControlActionUPP proc, ControlPartCode part)
{
	if (proc)
		proc(c, part);
	else if (c->action)
		c->action(c, part);
}

static void setHilite(ControlRef c, ControlPartCode part)
{
	if (c->hilite != part)
	{
		c->hilite = part;
		changed(c);
	}
}

// Tracks a button until the mouse is released: it is highlighted while the pointer is in it
static ControlPartCode trackButton(ControlRef c, CGPoint start, ControlPartCode part, ControlActionUPP action)
{
	CGPoint p = start;
	bool inside = true;
	UInt32 kind;

	setHilite(c, part);
	for (;;)
	{
		if (!nextMouseEvent(c, kEventDurationForever, &kind, &p))
			break;
		if (kind == 0)
			continue;
		inside = CGRectContainsPoint(windowFrame(c), p);
		if (isMouseUp(kind))
			break;
		setHilite(c, inside ? part : kControlNoPart);
	}
	setHilite(c, kControlNoPart);
	_HIWindowReleaseMouseGrab();
	if (!inside)
		return kControlNoPart;

	return part;
}

// Sends the command of an item of the control's menu, as a command event from a menu, to the control
static void sendMenuCommand(ControlRef c, MenuItemIndex item)
{
	struct HIMenuItemInfo info;
	EventRef event;
	HICommand hi;

	if (item < 1 || item > _HIMenuItemCount(c->menu))
		return;
	_HIMenuGetItem(c->menu, item, &info);
	if (info.command == 0)
		return;
	memset(&hi, 0, sizeof(hi));
	hi.attributes = kHICommandFromMenu;
	hi.commandID = info.command;
	hi.menu.menuRef = c->menu;
	hi.menu.menuItemIndex = item;
	if (CreateEvent(NULL, kEventClassCommand, kEventCommandProcess, _HICurrentTime(), kEventAttributeNone, &event) == noErr)
	{
		SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(hi), &hi);
		SendEventToEventTarget(event, c->target);
		ReleaseEvent(event);
	}
}

// Opens the pop-up menu of a control: the choice becomes its menu value
static ControlPartCode trackMenu(ControlRef c, CGPoint start)
{
	CGRect frame = windowFrame(c);
	CGRect content = _HIWindowGetContentRect(c->window);
	SInt16 top = (SInt16) (content.origin.y + frame.origin.y + frame.size.height);
	SInt16 left = (SInt16) (content.origin.x + frame.origin.x);

	if (c->cls == kClassPopupButton || c->menuPlacement == kControlBevelButtonMenuOnRight)
	{
		if (c->cls != kClassPopupButton)
		{
			top = (SInt16) (content.origin.y + frame.origin.y);
			left = (SInt16) (content.origin.x + frame.origin.x + frame.size.width);
		}
	}
	setHilite(c, kControlMenuPart);
	UInt32 choice = PopUpMenuSelect(c->menu, top, left, c->cls == kClassPopupButton ? (SInt16) c->value : (SInt16) c->menuValue);
	setHilite(c, kControlNoPart);
	_HIWindowReleaseMouseGrab();

	MenuItemIndex item = (MenuItemIndex) (choice & 0xffff);

	if (choice == 0 || item == 0)
		return kControlNoPart;
	if (c->cls == kClassPopupButton)
		HIViewSetValue(c, item);
	else if (!(c->menuBehavior & kControlBehaviorCommandMenu))
	{
		c->menuValue = (SInt16) item;
		if (c->menuBehavior & kControlBehaviorMultiValueMenu)
			; // the menu's own checks are the value
		changed(c);
	}
	// An item with a command makes its command event, sent to the control: it travels from the control
	// to its window and the application, so that a window that holds controls with menus handles their
	// commands, also when it is not the window that has the focus (a palette)
	sendMenuCommand(c, item);
	return kControlMenuPart;
}

static SInt32 valueForPoint(ControlRef c, CGPoint local, CGFloat grabOffset)
{
	HIThemeTrackDrawInfo info;
	CGFloat position = 0;
	SInt32 value = c->value;

	trackInfoOf(c, &info);

	bool horizontal = (info.attributes & kThemeTrackHorizontal) != 0;
	HIPoint offset = CGPointMake(local.x - c->boundsOrigin.x - (horizontal ? grabOffset : 0), local.y - c->boundsOrigin.y - (horizontal ? 0 : grabOffset));

	if (HIThemeGetTrackThumbPositionFromOffset(&info, &offset, &position) == noErr &&
		HIThemeGetTrackLiveValue(&info, position, &value) == noErr)
		return value;
	return c->value;
}

static ControlPartCode trackSlider(ControlRef c, CGPoint start, ControlActionUPP action)
{
	CGPoint p = start;
	UInt32 kind;
	HIThemeTrackDrawInfo info;
	HIRect thumb;
	CGFloat grab = 0;
	bool horizontal;

	trackInfoOf(c, &info);
	horizontal = (info.attributes & kThemeTrackHorizontal) != 0;
	if (HIThemeGetTrackPartBounds(&info, kAppearancePartIndicator, &thumb) == noErr)
	{
		CGPoint local = toLocal(c, start);

		// A click on the thumb keeps the grabbed point under the pointer; a click in the groove puts the thumb's centre there
		if (CGRectContainsPoint(thumb, CGPointMake(local.x - c->boundsOrigin.x, local.y - c->boundsOrigin.y)))
			grab = horizontal ? (local.x - c->boundsOrigin.x - thumb.origin.x) : (local.y - c->boundsOrigin.y - thumb.origin.y);
		else
			grab = (horizontal ? thumb.size.width : thumb.size.height) / 2;
	}
	setHilite(c, kControlIndicatorPart);
	{
		SInt32 value = valueForPoint(c, toLocal(c, start), grab);

		if (value != c->value)
			HIViewSetValue(c, value);
		callAction(c, action, kControlIndicatorPart);
	}
	for (;;)
	{
		if (!nextMouseEvent(c, kEventDurationForever, &kind, &p))
			break;
		if (kind == 0)
			continue;
		if (isMouseUp(kind))
			break;

		SInt32 value = valueForPoint(c, toLocal(c, p), grab);

		if (value != c->value)
		{
			HIViewSetValue(c, value);
			callAction(c, action, kControlIndicatorPart);
		}
	}
	setHilite(c, kControlNoPart);
	_HIWindowReleaseMouseGrab();
	return kControlIndicatorPart;
}

// The standard action of a scroll bar's parts, when there is no action function
static void standardScrollAction(ControlRef c, ControlPartCode part)
{
	SInt32 page = c->viewSize > 1 ? c->viewSize - 1 : 1;

	switch (part)
	{
		case kControlUpButtonPart: HIViewSetValue(c, c->value - 1); break;
		case kControlDownButtonPart: HIViewSetValue(c, c->value + 1); break;
		case kControlPageUpPart: HIViewSetValue(c, c->value - page); break;
		case kControlPageDownPart: HIViewSetValue(c, c->value + page); break;
		default: break;
	}
}

static ControlPartCode trackScrollBar(ControlRef c, CGPoint start, ControlPartCode part, ControlActionUPP action)
{
	CGPoint p = start;
	UInt32 kind;
	ControlActionUPP proc = action ? action : c->action;

	if (part == kControlIndicatorPart)
	{
		HIThemeTrackDrawInfo info;
		HIRect thumb;
		CGFloat grab = 0;
		bool horizontal;
		SInt32 startValue = c->value;

		trackInfoOf(c, &info);
		horizontal = (info.attributes & kThemeTrackHorizontal) != 0;
		if (HIThemeGetTrackPartBounds(&info, kAppearancePartIndicator, &thumb) == noErr)
		{
			CGPoint local = toLocal(c, start);

			grab = horizontal ? (local.x - c->boundsOrigin.x - thumb.origin.x) : (local.y - c->boundsOrigin.y - thumb.origin.y);
		}
		setHilite(c, kControlIndicatorPart);
		for (;;)
		{
			if (!nextMouseEvent(c, kEventDurationForever, &kind, &p))
				break;
			if (kind == 0)
				continue;
			if (isMouseUp(kind))
				break;

			SInt32 value = valueForPoint(c, toLocal(c, p), grab);

			if (value != c->value)
			{
				HIViewSetValue(c, value);
				callAction(c, action, kControlIndicatorPart);
			}
		}
		setHilite(c, kControlNoPart);
		_HIWindowReleaseMouseGrab();
		(void) startValue;
		return kControlIndicatorPart;
	}

	// The arrows and the page areas repeat while the pointer is held on them
	bool inside = true;
	EventTimeout delay = 0.35;

	setHilite(c, part);
	for (;;)
	{
		if (inside)
		{
			if (proc)
				proc(c, part);
			else
				standardScrollAction(c, part);
		}
		for (;;)
		{
			if (!nextMouseEvent(c, delay, &kind, &p))
				goto done;
			if (kind == 0)
				break;
			if (isMouseUp(kind))
				goto done;
			// The pointer left the part: the repeat stops until it is back
			ControlPartCode now = scrollBarPartAt(c, toLocal(c, p));

			inside = (now == part);
			setHilite(c, inside ? part : kControlNoPart);
		}
		delay = 0.06;
	}
done:
	setHilite(c, kControlNoPart);
	_HIWindowReleaseMouseGrab();
	return part;
}

static ControlPartCode trackControlDefault(ControlRef c, CGPoint start, ControlPartCode part, ControlActionUPP action)
{
	switch (c->cls)
	{
		case kClassPushButton:
		case kClassCheckBox:
		case kClassRadioButton:
			return trackButton(c, start, part, action);
		case kClassBevelButton:
			if (c->menu)
			{
				ControlPartCode result = trackMenu(c, start);

				if (c->menuBehavior & kControlBehaviorCommandMenu)
					return result;
				return result;
			}
			return trackButton(c, start, part, action);
		case kClassPopupButton:
			return trackMenu(c, start);
		case kClassSlider:
			return trackSlider(c, start, action);
		case kClassScrollBar:
			return trackScrollBar(c, start, part, action);
		default:
			return kControlNoPart;
	}
}

// Sends the tracking event and returns the part that was hit when the mouse was released
static ControlPartCode trackControlEvent(ControlRef c, CGPoint startWindow, UInt32 modifiers, ControlActionUPP action)
{
	EventRef event;
	CGPoint local = toLocal(c, startWindow);
	ControlPartCode part = kControlNoPart;

	if (CreateEvent(NULL, kEventClassControl, kEventControlTrack, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return kControlNoPart;
	SetEventParameter(event, kEventParamDirectObject, typeControlRef, sizeof(c), &c);
	SetEventParameter(event, kEventParamMouseLocation, typeHIPoint, sizeof(local), &local);
	SetEventParameter(event, kEventParamKeyModifiers, typeUInt32, sizeof(modifiers), &modifiers);
	if (action)
		SetEventParameter(event, kEventParamControlAction, typeControlActionUPP, sizeof(action), &action);
	if (SendEventToEventTarget(event, c->target) == noErr)
		GetEventParameter(event, kEventParamControlPart, typeControlPartCode, NULL, sizeof(part), NULL, &part);
	ReleaseEvent(event);
	return part;
}

static ControlPartCode clickControl(ControlRef c, CGPoint startWindow, UInt32 modifiers, ControlActionUPP action)
{
	EventRef event;
	CGPoint local = toLocal(c, startWindow);
	ControlPartCode part = kControlNoPart;
	EventMouseButton button = kEventMouseButtonPrimary;
	UInt32 clicks = 1;

	if (CreateEvent(NULL, kEventClassControl, kEventControlClick, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return kControlNoPart;
	SetEventParameter(event, kEventParamDirectObject, typeControlRef, sizeof(c), &c);
	SetEventParameter(event, kEventParamMouseLocation, typeHIPoint, sizeof(local), &local);
	SetEventParameter(event, kEventParamKeyModifiers, typeUInt32, sizeof(modifiers), &modifiers);
	SetEventParameter(event, kEventParamMouseButton, typeMouseButton, sizeof(button), &button);
	SetEventParameter(event, kEventParamClickCount, typeUInt32, sizeof(clicks), &clicks);
	if (action)
		SetEventParameter(event, kEventParamControlAction, typeControlActionUPP, sizeof(action), &action);
	if (SendEventToEventTarget(event, c->target) == noErr)
		GetEventParameter(event, kEventParamControlPart, typeControlPartCode, NULL, sizeof(part), NULL, &part);
	ReleaseEvent(event);
	return part;
}

// The click of a control: the control tracks the mouse, then (when a part was hit) the hit is reported
static OSStatus defaultClick(ControlRef c, EventRef event)
{
	HIPoint local;
	UInt32 modifiers = 0;
	ControlActionUPP action = NULL;
	ControlPartCode part;
	CGRect f = windowFrame(c);

	if (GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(local), NULL, &local) != noErr)
		return eventNotHandledErr;
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers);
	GetEventParameter(event, kEventParamControlAction, typeControlActionUPP, NULL, sizeof(action), NULL, &action);

	CGPoint windowPoint = CGPointMake(local.x + f.origin.x - c->boundsOrigin.x, local.y + f.origin.y - c->boundsOrigin.y);

	if (!c->enabled || !c->visible)
		return noErr;
	part = trackControlEvent(c, windowPoint, modifiers, action);
	SetEventParameter(event, kEventParamControlPart, typeControlPartCode, sizeof(part), &part);
	if (part != kControlNoPart)
		sendHit(c, part, modifiers);
	return noErr;
}

static OSStatus defaultTrack(ControlRef c, EventRef event)
{
	HIPoint local;
	ControlActionUPP action = NULL;
	CGRect f = windowFrame(c);

	if (GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(local), NULL, &local) != noErr)
		return eventNotHandledErr;
	GetEventParameter(event, kEventParamControlAction, typeControlActionUPP, NULL, sizeof(action), NULL, &action);

	CGPoint windowPoint = CGPointMake(local.x + f.origin.x - c->boundsOrigin.x, local.y + f.origin.y - c->boundsOrigin.y);
	ControlPartCode part;

	hitTestControl(c, windowPoint, &part);
	if (part == kControlNoPart)
		return noErr;

	ControlPartCode result = trackControlDefault(c, windowPoint, part, action);

	SetEventParameter(event, kEventParamControlPart, typeControlPartCode, sizeof(result), &result);
	return noErr;
}

// What a hit does when no handler of the control took it: the command of the control is processed
static OSStatus defaultHit(ControlRef c, EventRef event)
{
	// What the standard control does when it is hit: check boxes and radio buttons that toggle by themselves, and bevel
	// buttons that toggle or stick, change their value
	if ((c->cls == kClassCheckBox || c->cls == kClassRadioButton) && c->autoToggle)
		// (a mixed check box becomes checked)
		HIViewSetValue(c, c->value == 1 ? 0 : 1);
	else if (c->cls == kClassBevelButton && !c->menu && (c->behavior & kControlBehaviorToggles))
		HIViewSetValue(c, c->value ? 0 : 1);
	else if (c->cls == kClassBevelButton && !c->menu && (c->behavior & kControlBehaviorSticky))
		HIViewSetValue(c, 1);
	OSStatus result = eventNotHandledErr;

	if (c->command)
	{
		EventRef command;
		HICommand hi;

		memset(&hi, 0, sizeof(hi));
		hi.attributes = kHICommandFromControl;
		hi.commandID = c->command;
		if (CreateEvent(NULL, kEventClassCommand, kEventCommandProcess, _HICurrentTime(), kEventAttributeNone, &command) == noErr)
		{
			SetEventParameter(command, kEventParamDirectObject, typeHICommand, sizeof(hi), &hi);
			// The hit is handled when the command it makes was
			if (SendEventToEventTarget(command, c->target) == noErr)
				result = noErr;
			ReleaseEvent(command);
		}
	}
	return result;
}

static OSStatus defaultSimulateHit(ControlRef c, EventRef event)
{
	ControlPartCode part = kControlButtonPart;
	UInt32 modifiers = 0;

	GetEventParameter(event, kEventParamControlPart, typeControlPartCode, NULL, sizeof(part), NULL, &part);
	GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers);
	if (part != kControlNoPart)
		return sendHit(c, part, modifiers);
	return eventNotHandledErr;
}

static OSStatus controlHandler(EventHandlerCallRef call, EventRef event, void* userData)
{
	ControlRef c = userData;

	if (!isControl(c))
		return eventNotHandledErr;
	switch (GetEventKind(event))
	{
		case kEventControlDraw:
		{
			CGContextRef ctx = NULL;

			if (GetEventParameter(event, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(ctx), NULL, &ctx) != noErr || !ctx)
				return eventNotHandledErr;
			return defaultDraw(c, ctx);
		}
		case kEventControlHitTest:
		{
			CGPoint local;

			if (GetEventParameter(event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(local), NULL, &local) != noErr)
				return eventNotHandledErr;

			ControlPartCode part = defaultHitTest(c, local);

			SetEventParameter(event, kEventParamControlPart, typeControlPartCode, sizeof(part), &part);
			return noErr;
		}
		case kEventControlClick:
			return defaultClick(c, event);
		case kEventControlTrack:
			return defaultTrack(c, event);
		case kEventControlSimulateHit:
			return defaultSimulateHit(c, event);
		case kEventControlHit:
			return defaultHit(c, event);
	}
	return eventNotHandledErr;
}

#ifndef __LP64__
ControlPartCode HandleControlClick(ControlRef inControl, Point inWhere, EventModifiers inModifiers, ControlActionUPP inAction)
{
	if (!isControl(inControl) || !inControl->window)
		return kControlNoPart;
	return clickControl(inControl, CGPointMake(inWhere.h, inWhere.v), inModifiers, inAction);
}

ControlPartCode TrackControl(ControlRef theControl, Point startPoint, ControlActionUPP actionProc)
{
	if (!isControl(theControl) || !theControl->window)
		return kControlNoPart;
	return trackControlEvent(theControl, CGPointMake(startPoint.h, startPoint.v), 0, actionProc);
}

ControlActionUPP NewControlActionUPP(ControlActionProcPtr userRoutine)
{
	return userRoutine;
}

void DisposeControlActionUPP(ControlActionUPP userUPP)
{
}

void SetControlAction(ControlRef theControl, ControlActionUPP actionProc)
{
	if (isControl(theControl))
		theControl->action = actionProc;
}

ControlActionUPP GetControlAction(ControlRef theControl)
{
	return isControl(theControl) ? theControl->action : NULL;
}
#endif

#ifndef __LP64__
OSStatus HIViewClick(HIViewRef inView, EventRef inEvent)
{
	if (!isView(inView) || !inEvent || !inView->window)
		return paramErr;

	HIPoint location;
	UInt32 modifiers = 0;

	if (GetEventParameter(inEvent, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof(location), NULL, &location) != noErr)
		return eventParameterNotFoundErr;
	GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers);
	clickControl(inView, CGPointMake(location.x, location.y), modifiers, NULL);
	return noErr;
}
#endif

OSStatus HIViewSimulateClick(HIViewRef inView, HIViewPartCode inPartToClick, OptionBits inModifiers, ControlPartCode* outPartClicked)
{
	if (!isView(inView))
		return paramErr;
	// QUIRK: simulating a click on a disabled or hidden control sends no event and leaves the part untouched; part 0
	// sends the kEventControlSimulateHit event, with part 0, and nothing follows. Not documented. Evidence: black-box
	// observation on macOS 10.14.4 i386 (tier 2). Test: test_quirk_simulate_click_disabled_and_part_zero
	if (!inView->enabled || !inView->visible)
		return noErr;
	if (outPartClicked)
		*outPartClicked = kControlNoPart;

	EventRef event;
	ControlPartCode part = inPartToClick;
	UInt32 modifiers = inModifiers;

	if (CreateEvent(NULL, kEventClassControl, kEventControlSimulateHit, _HICurrentTime(), kEventAttributeNone, &event) != noErr)
		return memFullErr;
	SetEventParameter(event, kEventParamDirectObject, typeControlRef, sizeof(inView), &inView);
	SetEventParameter(event, kEventParamControlPart, typeControlPartCode, sizeof(part), &part);
	SetEventParameter(event, kEventParamKeyModifiers, typeUInt32, sizeof(modifiers), &modifiers);
	// QUIRK: the part that HIViewSimulateClick() reports is 0 when the hit was handled: by a handler of the control or
	// because the command it makes was handled. A hit that nobody handled reports the part. Not documented. Evidence:
	// black-box observation on macOS 10.14.4 i386 (tier 2). Test: test_quirk_simulate_click_part_when_handled
	OSStatus result = SendEventToEventTarget(event, inView->target);

	if (outPartClicked)
		*outPartClicked = result == noErr ? kControlNoPart : part;
	ReleaseEvent(event);
	return noErr;
}

// The window gets the mouse down in its content: the control under it handles it
bool _HIControlsWindowClick(WindowRef window, Point local, UInt32 modifiers)
{
	ControlRef root = _HIWindowGetRootControl(window);

	if (!root)
		return false;

	ControlPartCode part = kControlNoPart;
	ControlRef c = hitView(root, CGPointMake(local.h, local.v), &part, true);

	if (!c || part == kControlNoPart)
		return false;
	clickControl(c, CGPointMake(local.h, local.v), modifiers, NULL);
	return true;
}

// ------------------------------------------------------------------------------------------------
// Control data
// ------------------------------------------------------------------------------------------------

static OSStatus copyData(const void* source, Size size, Size bufferSize, void* buffer, Size* actual)
{
	if (actual)
		*actual = size;
	if (buffer)
	{
		// QUIRK: the buffer of a tag of a fixed size must be of exactly that size, a larger one is as wrong as a smaller one.
		// Not documented. Evidence: black-box observation on macOS 10.14.4 i386 (tier 2). Test: test_quirk_control_data_exact_size
		if (bufferSize != size)
			return errDataSizeMismatch;
		memcpy(buffer, source, size);
	}
	return noErr;
}

static OSStatus checkPart(ControlRef c, ControlPartCode part)
{
	return part == kControlEntireControl ? noErr : errDataNotSupported;
}

OSStatus SetControlData(ControlRef inControl, ControlPartCode inPart, ResType inTagName, Size inSize, const void* inData)
{
	if (!isControl(inControl) || (inSize && !inData))
		return paramErr;

	ControlRef c = inControl;

	switch (inTagName)
	{
		case kControlKindTag:
			return errDataNotSupported;
		case kControlSizeTag:
			if (inSize != sizeof(ControlSize))
				return errDataSizeMismatch;
			c->size = *(const ControlSize*) inData;
			changed(c);
			return noErr;
		case kControlFontStyleTag:
			if (inSize != sizeof(ControlFontStyleRec))
				return errDataSizeMismatch;
			c->fontStyle = *(const ControlFontStyleRec*) inData;
			c->hasFontStyle = true;
			changed(c);
			return noErr;
		default:
			break;
	}
	switch (c->cls)
	{
		case kClassBevelButton:
			if (inTagName == kControlBevelButtonMenuValueTag)
			{
				if (inSize != sizeof(SInt16))
					return errDataSizeMismatch;
				c->menuValue = *(const SInt16*) inData;
				changed(c);
				return noErr;
			}
			if (inTagName == kControlBevelButtonMenuRefTag)
			{
				if (inSize != sizeof(MenuRef))
					return errDataSizeMismatch;
				if (c->menu && c->menuOwned)
					CFRelease((CFTypeRef) c->menu);
				c->menu = *(MenuRef const*) inData;
				c->menuOwned = false;
				changed(c);
				return noErr;
			}
			if (inTagName == kControlBevelButtonContentTag)
			{
				if (inSize != sizeof(ControlButtonContentInfo))
					return errDataSizeMismatch;
				c->content = *(const ControlButtonContentInfo*) inData;
				changed(c);
				return noErr;
			}
			if (inTagName == kControlBevelButtonTransformTag || inTagName == kControlBevelButtonTextAlignTag ||
				inTagName == kControlBevelButtonTextOffsetTag || inTagName == kControlBevelButtonGraphicAlignTag ||
				inTagName == kControlBevelButtonGraphicOffsetTag || inTagName == kControlBevelButtonTextPlaceTag ||
				inTagName == kControlBevelButtonCenterPopupGlyphTag || inTagName == kControlBevelButtonMenuDelayTag ||
				inTagName == kControlBevelButtonScaleIconTag)
				return noErr;
			break;
		case kClassStaticText:
			if (inTagName == kControlStaticTextCFStringTag)
			{
				if (inSize != sizeof(CFStringRef))
					return errDataSizeMismatch;
				return SetControlTitleWithCFString(c, *(const CFStringRef*) inData);
			}
			if (inTagName == kControlStaticTextTextTag)
			{
				CFStringRef text = CFStringCreateWithBytes(NULL, inData, inSize, kCFStringEncodingMacRoman, false);
				OSStatus status = SetControlTitleWithCFString(c, text);

				if (text)
					CFRelease(text);
				return status;
			}
			if (inTagName == kControlStaticTextTruncTag || inTagName == kControlStaticTextTextHeightTag)
				return noErr;
			break;
		case kClassScrollBar:
			if (inTagName == kControlScrollBarShowsArrowsTag)
			{
				if (inSize != sizeof(Boolean))
					return errDataSizeMismatch;
				c->showsArrows = *(const Boolean*) inData != 0;
				changed(c);
				return noErr;
			}
			break;
		default:
			break;
	}
	(void) inPart;
	return errDataNotSupported;
}

OSStatus GetControlData(ControlRef inControl, ControlPartCode inPart, ResType inTagName, Size inBufferSize, void* inBuffer, Size* outActualSize)
{
	if (!isControl(inControl))
		return paramErr;

	ControlRef c = inControl;

	switch (inTagName)
	{
		case kControlKindTag:
		{
			ControlKind kind;

			GetControlKind(c, &kind);
			return copyData(&kind, sizeof(kind), inBufferSize, inBuffer, outActualSize);
		}
		case kControlSizeTag:
			return copyData(&c->size, sizeof(ControlSize), inBufferSize, inBuffer, outActualSize);
		case kControlFontStyleTag:
			return copyData(&c->fontStyle, sizeof(ControlFontStyleRec), inBufferSize, inBuffer, outActualSize);
		default:
			break;
	}
	switch (c->cls)
	{
		case kClassBevelButton:
			if (inTagName == kControlBevelButtonMenuValueTag)
				return copyData(&c->menuValue, sizeof(SInt16), inBufferSize, inBuffer, outActualSize);
			if (inTagName == kControlBevelButtonMenuRefTag && c->menu)
				return copyData(&c->menu, sizeof(MenuRef), inBufferSize, inBuffer, outActualSize);
			if (inTagName == kControlBevelButtonContentTag)
				return copyData(&c->content, sizeof(ControlButtonContentInfo), inBufferSize, inBuffer, outActualSize);
			break;
		case kClassScrollBar:
			if (inTagName == kControlScrollBarShowsArrowsTag)
			{
				Boolean shows = c->showsArrows;

				return copyData(&shows, sizeof(shows), inBufferSize, inBuffer, outActualSize);
			}
			break;
		case kClassStaticText:
			if (inTagName == kControlStaticTextCFStringTag)
			{
				CFStringRef text = CFRetain(c->title ? c->title : CFSTR(""));

				return copyData(&text, sizeof(text), inBufferSize, inBuffer, outActualSize);
			}
			if (inTagName == kControlStaticTextTextTag)
			{
				char buffer[1024];
				Size length = 0;

				if (c->title && CFStringGetBytes(c->title, CFRangeMake(0, CFStringGetLength(c->title)), kCFStringEncodingMacRoman, '?', false, (UInt8*) buffer, sizeof(buffer), (CFIndex*) &length) <= 0)
					length = 0;
				if (outActualSize)
					*outActualSize = length;
				if (inBuffer)
					memcpy(inBuffer, buffer, length < inBufferSize ? length : inBufferSize);
				return noErr;
			}
			break;
		default:
			break;
	}
	(void) inPart;
	return errDataNotSupported;
}

#ifndef __LP64__
OSStatus GetControlDataSize(ControlRef inControl, ControlPartCode inPart, ResType inTagName, Size* outMaxSize)
{
	return GetControlData(inControl, inPart, inTagName, 0, NULL, outMaxSize);
}
#endif

// ------------------------------------------------------------------------------------------------
// Creating
// ------------------------------------------------------------------------------------------------

// Puts a new control in a window (its content), or leaves it loose without one
static OSStatus placeControl(WindowRef window, ControlRef c, const Rect* bounds)
{
	if (!c)
		return memFullErr;
	if (!window)
		return noErr;

	ControlRef container = embeddingRoot(window);

	if (!container)
	{
		disposeTree(c);
		return errNoRootControl;
	}
	// The rectangle is in the window's coordinates
	CGRect parentFrame = windowFrame(container);

	c->frame.origin.x -= parentFrame.origin.x - container->boundsOrigin.x;
	c->frame.origin.y -= parentFrame.origin.y - container->boundsOrigin.y;
	return HIViewAddSubview(container, c);
}

static OSStatus makeControl(WindowRef window, const Rect* bounds, int cls, ControlRef* out, ControlRef* made)
{
	*out = NULL;
	if (!out || !bounds)
		return paramErr;
	if (window && !IsValidWindowPtr(window))
		return errInvalidWindowPtr;
	if (window && !embeddingRoot(window))
		return errNoRootControl;

	ControlRef c = newControl(cls, rectFromQD(bounds));

	if (!c)
		return memFullErr;
	*made = c;
	return noErr;
}

static OSStatus finishControl(WindowRef window, ControlRef c, const Rect* bounds, ControlRef* out)
{
	OSStatus status = placeControl(window, c, bounds);

	if (status == noErr)
		*out = c;
	return status;
}

static void setTitle(ControlRef c, CFStringRef title)
{
	if (title)
		c->title = CFStringCreateCopy(NULL, title);
}

static OSStatus createBevelButton(WindowRef window, const Rect* boundsRect, CFStringRef title, UInt16 thickness, UInt16 behavior, ControlButtonContentInfoPtr info, SInt16 menuID, UInt16 menuBehavior, UInt16 menuPlacement, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, kClassBevelButton, outControl, &c);
	if (status != noErr)
		return status;
	setTitle(c, title);
	c->max = 2;
	c->bevelThickness = thickness;
	c->behavior = behavior;
	c->menuBehavior = menuBehavior;
	c->menuPlacement = menuPlacement;
	c->menuID = menuID;
	if (menuID != 0)
		c->menu = GetMenuHandle(menuID);
	if (c->menu)
		c->menuValue = 1;
	if (info)
		c->content = *info;
	return finishControl(window, c, boundsRect, outControl);
}

OSStatus CreatePushButtonControl(WindowRef window, const Rect* boundsRect, CFStringRef title, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, kClassPushButton, outControl, &c);
	if (status != noErr)
		return status;
	setTitle(c, title);
	return finishControl(window, c, boundsRect, outControl);
}

static OSStatus createToggle(int cls, WindowRef window, const Rect* boundsRect, CFStringRef title, SInt32 initialValue, Boolean autoToggle, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, cls, outControl, &c);
	if (status != noErr)
		return status;
	setTitle(c, title);
	c->max = 2;
	c->value = initialValue;
	c->autoToggle = autoToggle;
	return finishControl(window, c, boundsRect, outControl);
}

OSStatus CreateCheckBoxControl(WindowRef window, const Rect* boundsRect, CFStringRef title, SInt32 initialValue, Boolean autoToggle, ControlRef* outControl)
{
	return createToggle(kClassCheckBox, window, boundsRect, title, initialValue, autoToggle, outControl);
}

static OSStatus createRadioButton(WindowRef window, const Rect* boundsRect, CFStringRef title, SInt32 initialValue, Boolean autoToggle, ControlRef* outControl)
{
	return createToggle(kClassRadioButton, window, boundsRect, title, initialValue, autoToggle, outControl);
}

static OSStatus createSlider(WindowRef window, const Rect* boundsRect, SInt32 value, SInt32 minimum, SInt32 maximum, ControlSliderOrientation orientation, UInt16 numTickMarks, Boolean liveTracking, ControlActionUPP liveTrackingProc, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, kClassSlider, outControl, &c);
	if (status != noErr)
		return status;
	c->min = minimum;
	c->max = maximum;
	c->value = value;
	c->ticks = numTickMarks;
	c->live = liveTracking;
	c->action = liveTrackingProc;
	c->horizontal = (boundsRect->right - boundsRect->left) >= (boundsRect->bottom - boundsRect->top);
	return finishControl(window, c, boundsRect, outControl);
}

static OSStatus createScrollBar(WindowRef window, const Rect* boundsRect, SInt32 value, SInt32 minimum, SInt32 maximum, SInt32 viewSize, Boolean liveTracking, ControlActionUPP liveTrackingProc, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, kClassScrollBar, outControl, &c);
	if (status != noErr)
		return status;
	c->min = minimum;
	c->max = maximum;
	c->value = value;
	c->viewSize = viewSize;
	c->live = liveTracking;
	c->action = liveTrackingProc;
	return finishControl(window, c, boundsRect, outControl);
}

OSStatus CreateStaticTextControl(WindowRef window, const Rect* boundsRect, CFStringRef text, const ControlFontStyleRec* style, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, kClassStaticText, outControl, &c);
	if (status != noErr)
		return status;
	setTitle(c, text);
	if (style)
	{
		c->fontStyle = *style;
		c->hasFontStyle = true;
	}
	return finishControl(window, c, boundsRect, outControl);
}

static OSStatus createGroupBox(WindowRef window, const Rect* boundsRect, CFStringRef title, Boolean primary, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, kClassGroupBox, outControl, &c);
	if (status != noErr)
		return status;
	setTitle(c, title);
	c->primary = primary;
	return finishControl(window, c, boundsRect, outControl);
}

static OSStatus createSeparator(WindowRef window, const Rect* boundsRect, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, kClassSeparator, outControl, &c);
	if (status != noErr)
		return status;
	return finishControl(window, c, boundsRect, outControl);
}

static OSStatus createUserPane(WindowRef window, const Rect* boundsRect, UInt32 features, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, kClassUserPane, outControl, &c);
	if (status != noErr)
		return status;
	c->features = features;
	return finishControl(window, c, boundsRect, outControl);
}

static OSStatus createPopupButton(WindowRef window, const Rect* boundsRect, CFStringRef title, SInt16 menuID, Boolean variableWidth, SInt16 titleWidth, SInt16 titleJustification, UInt8 titleStyle, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, kClassPopupButton, outControl, &c);
	if (status != noErr)
		return status;
	setTitle(c, title);
	c->menuID = menuID;
	if (menuID != 0)
		c->menu = GetMenuHandle(menuID);
	c->value = c->menu ? 1 : 0;
	c->min = 0;
	c->max = c->menu ? CountMenuItems(c->menu) : 0;
	c->popupTitleWidth = titleWidth;
	return finishControl(window, c, boundsRect, outControl);
}

static OSStatus createProgressBar(WindowRef window, const Rect* boundsRect, SInt32 value, SInt32 minimum, SInt32 maximum, Boolean indeterminate, ControlRef* outControl)
{
	ControlRef c;
	OSStatus status;

	if (!outControl)
		return paramErr;
	status = makeControl(window, boundsRect, kClassProgressBar, outControl, &c);
	if (status != noErr)
		return status;
	c->min = minimum;
	c->max = maximum;
	c->value = value;
	c->indeterminate = indeterminate;
	return finishControl(window, c, boundsRect, outControl);
}


#ifndef __LP64__
OSStatus CreateBevelButtonControl(WindowRef window, const Rect* boundsRect, CFStringRef title, UInt16 thickness, UInt16 behavior, ControlButtonContentInfoPtr info, SInt16 menuID, UInt16 menuBehavior, UInt16 menuPlacement, ControlRef* outControl)
{
	return createBevelButton(window, boundsRect, title, thickness, behavior, info, menuID, menuBehavior, menuPlacement, outControl);
}

OSStatus CreateGroupBoxControl(WindowRef window, const Rect* boundsRect, CFStringRef title, Boolean primary, ControlRef* outControl)
{
	return createGroupBox(window, boundsRect, title, primary, outControl);
}

OSStatus CreatePopupButtonControl(WindowRef window, const Rect* boundsRect, CFStringRef title, SInt16 menuID, Boolean variableWidth, SInt16 titleWidth, SInt16 titleJustification, UInt8 titleStyle, ControlRef* outControl)
{
	return createPopupButton(window, boundsRect, title, menuID, variableWidth, titleWidth, titleJustification, titleStyle, outControl);
}

OSStatus CreateProgressBarControl(WindowRef window, const Rect* boundsRect, SInt32 value, SInt32 minimum, SInt32 maximum, Boolean indeterminate, ControlRef* outControl)
{
	return createProgressBar(window, boundsRect, value, minimum, maximum, indeterminate, outControl);
}

OSStatus CreateRadioButtonControl(WindowRef window, const Rect* boundsRect, CFStringRef title, SInt32 initialValue, Boolean autoToggle, ControlRef* outControl)
{
	return createRadioButton(window, boundsRect, title, initialValue, autoToggle, outControl);
}

OSStatus CreateScrollBarControl(WindowRef window, const Rect* boundsRect, SInt32 value, SInt32 minimum, SInt32 maximum, SInt32 viewSize, Boolean liveTracking, ControlActionUPP liveTrackingProc, ControlRef* outControl)
{
	return createScrollBar(window, boundsRect, value, minimum, maximum, viewSize, liveTracking, liveTrackingProc, outControl);
}

OSStatus CreateSeparatorControl(WindowRef window, const Rect* boundsRect, ControlRef* outControl)
{
	return createSeparator(window, boundsRect, outControl);
}

OSStatus CreateSliderControl(WindowRef window, const Rect* boundsRect, SInt32 value, SInt32 minimum, SInt32 maximum, ControlSliderOrientation orientation, UInt16 numTickMarks, Boolean liveTracking, ControlActionUPP liveTrackingProc, ControlRef* outControl)
{
	return createSlider(window, boundsRect, value, minimum, maximum, orientation, numTickMarks, liveTracking, liveTrackingProc, outControl);
}

OSStatus CreateUserPaneControl(WindowRef window, const Rect* boundsRect, UInt32 features, ControlRef* outControl)
{
	return createUserPane(window, boundsRect, features, outControl);
}

#endif

#ifndef __LP64__
ControlRef NewControl(WindowRef owningWindow, const Rect* boundsRect, ConstStr255Param controlTitle, Boolean initiallyVisible, SInt16 initialValue, SInt16 minimumValue, SInt16 maximumValue, SInt16 procID, SInt32 controlReference)
{
	ControlRef c = NULL;
	CFStringRef title = controlTitle ? CFStringCreateWithPascalString(NULL, controlTitle, kCFStringEncodingMacRoman) : NULL;
	OSStatus status = paramErr;

	if (!boundsRect)
		return NULL;
	if (procID >= kControlBevelButtonSmallBevelProc && procID < kControlBevelButtonSmallBevelProc + 8 && (procID & ~3) == 32)
		status = createBevelButton(owningWindow, boundsRect, title, procID & 3, 0, NULL, 0, 0, 0, &c);
	else if (procID >= kControlSliderProc && procID < kControlSliderProc + 8)
		status = createSlider(owningWindow, boundsRect, initialValue, minimumValue, maximumValue, kControlSliderDoesNotPoint, 0, (procID & 1) != 0, NULL, &c);
	else if (procID >= kControlScrollBarProc && procID < kControlScrollBarProc + 8)
		status = createScrollBar(owningWindow, boundsRect, initialValue, minimumValue, maximumValue, 0, (procID & 2) != 0, NULL, &c);
	else if (procID >= kControlStaticTextProc && procID < kControlStaticTextProc + 8)
		status = CreateStaticTextControl(owningWindow, boundsRect, title, NULL, &c);
	else if (procID == kControlPushButtonProc)
		status = CreatePushButtonControl(owningWindow, boundsRect, title, &c);
	else if (procID == kControlCheckBoxProc)
		status = CreateCheckBoxControl(owningWindow, boundsRect, title, initialValue, true, &c);
	else if (procID == kControlRadioButtonProc)
		status = createRadioButton(owningWindow, boundsRect, title, initialValue, true, &c);
	else if (procID >= kControlUserPaneProc && procID < kControlUserPaneProc + 8)
		status = createUserPane(owningWindow, boundsRect, kControlSupportsEmbedding, &c);
	else if (procID == kControlSeparatorLineProc)
		status = createSeparator(owningWindow, boundsRect, &c);
	else if (procID >= kControlGroupBoxTextTitleProc && procID < kControlGroupBoxTextTitleProc + 8)
		status = createGroupBox(owningWindow, boundsRect, title, true, &c);
	else if (procID == kControlProgressBarProc)
		status = createProgressBar(owningWindow, boundsRect, initialValue, minimumValue, maximumValue, false, &c);
	if (title)
		CFRelease(title);
	if (status != noErr || !c)
		return NULL;
	c->refCon = controlReference;
	if (c->cls != kClassPushButton)
	{
		c->min = minimumValue;
		c->max = maximumValue;
		c->value = initialValue;
	}
	if (!initiallyVisible)
		HIViewSetVisible(c, false);
	return c;
}
#endif

// Bevel button menu accessors

#ifndef __LP64__
SInt16 GetBevelButtonMenuValue(ControlRef inButton, MenuItemIndex* outValue)
{
	if (!isControl(inButton) || inButton->cls != kClassBevelButton || !outValue)
		return paramErr;
	*outValue = (MenuItemIndex) inButton->menuValue;
	return noErr;
}
#endif

#ifndef __LP64__
OSStatus SetBevelButtonMenuValue(ControlRef inButton, MenuItemIndex inValue)
{
	if (!isControl(inButton) || inButton->cls != kClassBevelButton)
		return paramErr;
	inButton->menuValue = (SInt16) inValue;
	changed(inButton);
	return noErr;
}
#endif

// ------------------------------------------------------------------------------------------------
// Controls from nibs
// ------------------------------------------------------------------------------------------------

static SInt64 nibNumber(CFDictionaryRef d, CFStringRef key, SInt64 fallback)
{
	CFTypeRef value = CFDictionaryGetValue(d, key);
	SInt64 n;

	if (value && CFGetTypeID(value) == CFNumberGetTypeID() && CFNumberGetValue(value, kCFNumberSInt64Type, &n))
		return n;
	return fallback;
}

static bool nibBool(CFDictionaryRef d, CFStringRef key)
{
	CFTypeRef value = CFDictionaryGetValue(d, key);

	return value && CFGetTypeID(value) == CFBooleanGetTypeID() && CFBooleanGetValue(value);
}

static bool nibBounds(CFDictionaryRef d, Rect* out)
{
	CFStringRef string = CFDictionaryGetValue(d, CFSTR("bounds"));
	char text[128];
	int top, left, bottom, right;

	if (!string || CFGetTypeID(string) != CFStringGetTypeID() || !CFStringGetCString(string, text, sizeof(text), kCFStringEncodingUTF8) ||
		sscanf(text, "%d %d %d %d", &top, &left, &bottom, &right) != 4)
		return false;
	*out = (Rect) { (short) top, (short) left, (short) bottom, (short) right };
	return true;
}

static bool classIs(CFDictionaryRef d, CFStringRef name)
{
	CFStringRef cls = CFDictionaryGetValue(d, CFSTR("class"));

	return cls && CFGetTypeID(cls) == CFStringGetTypeID() && CFStringCompare(cls, name, 0) == kCFCompareEqualTo;
}

static OSStatus makeFromNib(CFDictionaryRef d, WindowRef window, ControlRef container)
{
	Rect bounds;
	ControlRef c = NULL;
	CFStringRef title = CFDictionaryGetValue(d, CFSTR("title"));
	SInt64 size = nibNumber(d, CFSTR("controlSize"), 0);

	if (title && CFGetTypeID(title) != CFStringGetTypeID())
		title = NULL;
	if (!nibBounds(d, &bounds))
		return paramErr;

	if (classIs(d, CFSTR("IBCarbonBevelButton")))
	{
		ControlButtonContentInfo content = { 0 };
		MenuRef menu = NULL;
		CFDictionaryRef menuDescription = CFDictionaryGetValue(d, CFSTR("menu"));
		UInt16 behavior = (UInt16) nibNumber(d, CFSTR("behavior"), 0);

		content.contentType = (ControlContentType) nibNumber(d, CFSTR("contentType"), 0);
		content.u.resID = (SInt16) nibNumber(d, CFSTR("contentResID"), 0);
		if (content.contentType == kControlContentTextOnly)
			memset(&content.u, 0, sizeof(content.u));
		if (createBevelButton(NULL, &bounds, title, (UInt16) nibNumber(d, CFSTR("bevelThickness"), 1), behavior, &content, 0,
			(UInt16) nibNumber(d, CFSTR("menuBehavior"), 0), (UInt16) nibNumber(d, CFSTR("menuPlacement"), 0), &c) != noErr)
			return memFullErr;
		if (menuDescription && CFGetTypeID(menuDescription) == CFDictionaryGetTypeID() && _HICreateMenuFromNibDescription(menuDescription, &menu) == noErr)
		{
			c->menu = menu;
			c->menuOwned = true;
			c->menuValue = (SInt16) nibNumber(d, CFSTR("selectedItem"), 1);
		}
		c->rounded = nibBool(d, CFSTR("rounded"));
	}
	else if (classIs(d, CFSTR("IBCarbonSlider")))
	{
		SInt32 value = (SInt32) nibNumber(d, CFSTR("initialValue"), 50);

		if (createSlider(NULL, &bounds, value, (SInt32) nibNumber(d, CFSTR("minimum"), 0), (SInt32) nibNumber(d, CFSTR("maximum"), 100),
			kControlSliderPointsDownOrRight, (UInt16) nibNumber(d, CFSTR("numTickMarks"), 0), nibBool(d, CFSTR("isLive")), NULL, &c) != noErr)
			return memFullErr;
	}
	else if (classIs(d, CFSTR("IBCarbonScrollBar")))
	{
		if (createScrollBar(NULL, &bounds, (SInt32) nibNumber(d, CFSTR("initialValue"), 0), (SInt32) nibNumber(d, CFSTR("minimum"), 0),
			(SInt32) nibNumber(d, CFSTR("maximum"), 100), (SInt32) nibNumber(d, CFSTR("viewSize"), 0), nibBool(d, CFSTR("isLive")), NULL, &c) != noErr)
			return memFullErr;
	}
	else if (classIs(d, CFSTR("IBCarbonStaticText")))
	{
		if (CreateStaticTextControl(NULL, &bounds, title, NULL, &c) != noErr)
			return memFullErr;
	}
	else if (classIs(d, CFSTR("IBCarbonPushButton")))
	{
		if (CreatePushButtonControl(NULL, &bounds, title, &c) != noErr)
			return memFullErr;
	}
	else if (classIs(d, CFSTR("IBCarbonCheckBox")) || classIs(d, CFSTR("IBCarbonRadioButton")))
	{
		bool radio = classIs(d, CFSTR("IBCarbonRadioButton"));
		SInt32 value = (SInt32) nibNumber(d, CFSTR("initialValue"), 0);
		OSStatus status = radio ? createRadioButton(NULL, &bounds, title, value, true, &c) : CreateCheckBoxControl(NULL, &bounds, title, value, true, &c);

		if (status != noErr)
			return memFullErr;
	}
	else if (classIs(d, CFSTR("IBCarbonGroupBox")))
	{
		if (createGroupBox(NULL, &bounds, title, !nibBool(d, CFSTR("secondary")), &c) != noErr)
			return memFullErr;
	}
	else if (classIs(d, CFSTR("IBCarbonSeparator")))
	{
		if (createSeparator(NULL, &bounds, &c) != noErr)
			return memFullErr;
	}
	else if (classIs(d, CFSTR("IBCarbonProgressBar")))
	{
		if (createProgressBar(NULL, &bounds, (SInt32) nibNumber(d, CFSTR("initialValue"), 0), (SInt32) nibNumber(d, CFSTR("minimum"), 0),
			(SInt32) nibNumber(d, CFSTR("maximum"), 100), nibBool(d, CFSTR("indeterminate")), &c) != noErr)
			return memFullErr;
	}
	else if (classIs(d, CFSTR("IBCarbonUserPane")) || classIs(d, CFSTR("IBCarbonView")))
	{
		if (createUserPane(NULL, &bounds, kControlSupportsEmbedding, &c) != noErr)
			return memFullErr;
	}
	else
		return noErr; // a control the Control Manager does not make: left out

	// The nib records the size as the Control Manager's enumeration; a "small" flag asks for the small size
	if (size >= kControlSizeNormal && size <= kControlSizeMini)
		c->size = (ControlSize) size;
	if (nibBool(d, CFSTR("small")))
		c->size = kControlSizeSmall;
	c->id.signature = (OSType) nibNumber(d, CFSTR("controlSignature"), 0);
	c->id.id = (SInt32) nibNumber(d, CFSTR("controlID"), 0);
	c->command = (UInt32) nibNumber(d, CFSTR("command"), 0);
	if (nibNumber(d, CFSTR("initialValue"), -1) >= 0 && (c->cls == kClassBevelButton))
		c->value = (SInt32) nibNumber(d, CFSTR("initialValue"), 0);
	if (nibBool(d, CFSTR("invisible")))
		c->visible = false;
	if (nibBool(d, CFSTR("disabled")))
		c->enabled = false;

	// The rectangle of a nib control is in its container's coordinates
	if (container)
		HIViewAddSubview(container, c);

	CFArrayRef subviews = CFDictionaryGetValue(d, CFSTR("subviews"));

	if (subviews && CFGetTypeID(subviews) == CFArrayGetTypeID())
	{
		for (CFIndex i = 0; i < CFArrayGetCount(subviews); i++)
		{
			CFDictionaryRef sub = CFArrayGetValueAtIndex(subviews, i);

			if (CFGetTypeID(sub) == CFDictionaryGetTypeID())
				makeFromNib(sub, window, c);
		}
	}
	return noErr;
}

OSStatus _HICreateControlsFromNibDescription(CFDictionaryRef rootDescription, WindowRef window)
{
	CFArrayRef subviews = CFDictionaryGetValue(rootDescription, CFSTR("subviews"));
	ControlRef root = _HIWindowGetRootControl(window);
	ControlRef container;

	if (!root)
	{
		OSStatus status = CreateRootControl(window, &root);

		if (status != noErr)
			return status;
	}
	container = embeddingRoot(window);
	if (!subviews || CFGetTypeID(subviews) != CFArrayGetTypeID())
		return noErr;
	for (CFIndex i = 0; i < CFArrayGetCount(subviews); i++)
	{
		CFDictionaryRef sub = CFArrayGetValueAtIndex(subviews, i);

		if (CFGetTypeID(sub) == CFDictionaryGetTypeID())
		{
			OSStatus status = makeFromNib(sub, window, container);

			if (status != noErr)
				return status;
		}
	}
	return noErr;
}
