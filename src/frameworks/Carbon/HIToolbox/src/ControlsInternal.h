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

#ifndef HITOOLBOX_CONTROLS_INTERNAL_H
#define HITOOLBOX_CONTROLS_INTERNAL_H

// Not installed. The interface between the Window Manager (MacWindows.c) and the Control Manager
// (Controls.c), which keeps the view hierarchy of a window.

#include <HIToolbox/Controls.h>
#include <HIToolbox/HIView.h>
#include <HIToolbox/MacWindows.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// The Window Manager -> the Control Manager

// A window was made: a compositing window gets its root view
void _HIControlsWindowCreated(WindowRef window);
// The window is going away: its controls are disposed
void _HIControlsWindowDisposing(WindowRef window);
// The content of the window has a new size
void _HIControlsWindowResized(WindowRef window, int width, int height);
// The window became active or inactive: the controls draw differently
void _HIControlsWindowActivationChanged(WindowRef window);
// A click in the content of the window that the window's handlers left alone (the location is local,
// in the window's coordinates): the control under it handles it. False when there is none.
bool _HIControlsWindowClick(WindowRef window, Point local, UInt32 modifiers);
// Draws the controls of a window into a context that has its origin at the top left of the content and
// y axis pointing down; only what intersects the rectangle. With the background, the window's
// background is drawn first (a compositing window).
void _HIControlsDrawWindow(WindowRef window, CGContextRef context, CGRect dirty);

// The Control Manager -> the Window Manager

bool _HIWindowIsCompositing(WindowRef window);
// Content size and origin on the screen (the origin of the top left of the content)
CGRect _HIWindowGetContentRect(WindowRef window);
bool _HIWindowControlsActive(WindowRef window);
bool _HIWindowIsShown(WindowRef window);
// The height of what the frame of the window has above its content (the title bar)
CGFloat _HIWindowStructureTop(WindowRef window);
// The context that draws into the content of the window, flipped to the origin at the top left. NULL when
// the window cannot be drawn into. _HIWindowEndDrawing() puts the part that was drawn on the screen.
CGContextRef _HIWindowBeginDrawing(WindowRef window);
void _HIWindowEndDrawing(WindowRef window, CGContextRef context, CGRect drawn);
// Adds the rectangle (window coordinates) to what has to be drawn again, and asks for the update
void _HIWindowInvalidateRect(WindowRef window, CGRect rect);
// Draws now what is invalid (what the update event would do)
void _HIWindowRenderNow(WindowRef window);
// The color of the content's background, NULL for a window that has none (the caller releases it)
CGColorRef _HIWindowCopyBackgroundColor(WindowRef window);
void _HIWindowSetBackgroundBrush(WindowRef window, SInt16 brush);
// The mouse grab of the Window Manager ends: its mouse up was taken by a tracking loop
void _HIWindowReleaseMouseGrab(void);

// The root control that the window keeps for the Control Manager
ControlRef _HIWindowGetRootControl(WindowRef window);
void _HIWindowSetRootControl(WindowRef window, ControlRef root);

// Makes a menu from the description of a nib menu (IBCarbonRuntime.c)
OSStatus _HICreateMenuFromNibDescription(CFDictionaryRef description, MenuRef* outMenu);
// Makes the controls of the nib description of a root control in the window (Controls.c)
OSStatus _HICreateControlsFromNibDescription(CFDictionaryRef rootDescription, WindowRef window);

#ifdef __cplusplus
}
#endif

#endif
