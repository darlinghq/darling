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

// Darling-private interface between UIFoundation and the framework that owns
// the current drawing context (AppKit).
//
// UIFoundation sits below AppKit and must not link against it, yet its text
// classes draw into whatever context AppKit has made current. AppKit registers
// an object conforming to the protocol below once at start-up, and
// UIFoundation asks it for the current context whenever it draws. AppKit's
// own context stack stays the single source of truth, so anything that
// changes the current NSGraphicsContext is seen by UIFoundation too.
//
// Without a registered provider (a process that uses UIFoundation without
// AppKit) the functions report no context, and drawing does nothing.

#ifndef _UIFoundationGraphicsContext_H_
#define _UIFoundationGraphicsContext_H_

#import <Foundation/NSObject.h>
#include <CoreGraphics/CGContext.h>

@protocol _UIFoundationGraphicsContextProvider <NSObject>

// The Core Graphics context drawing currently goes to, or NULL if there is
// none. The caller does not own the returned context.
- (CGContextRef)currentCGContext;

// Whether the current context's coordinate system has its origin at the top
// left, as in a flipped view.
- (BOOL)currentContextIsFlipped;

// Whether the current context draws to the screen, as opposed to printing or
// drawing into a PDF or a bitmap.
- (BOOL)currentContextIsDrawingToScreen;

@end

#ifdef __cplusplus
extern "C" {
#endif

// Registers the provider (retained). Meant to be called once while the
// providing framework initialises; a later call replaces the provider and
// releases the previous one. Passing nil unregisters.
FOUNDATION_EXPORT void _UIFoundationSetGraphicsContextProvider(id<_UIFoundationGraphicsContextProvider> provider);

// The registered provider, or nil.
FOUNDATION_EXPORT id<_UIFoundationGraphicsContextProvider> _UIFoundationGraphicsContextProvider(void);

// Convenience wrappers: NULL, NO and NO when no provider is registered.
FOUNDATION_EXPORT CGContextRef _UIFoundationCurrentCGContext(void);
FOUNDATION_EXPORT BOOL _UIFoundationCurrentContextIsFlipped(void);
FOUNDATION_EXPORT BOOL _UIFoundationCurrentContextIsDrawingToScreen(void);

#ifdef __cplusplus
}
#endif

#endif
