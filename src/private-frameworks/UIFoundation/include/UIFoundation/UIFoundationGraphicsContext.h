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
// The same provider hands UIFoundation the few AppKit objects its text classes
// need but cannot create themselves: colours for the standard text roles, the
// fallback font, images made from attachment data and attachment cells. They
// come back as plain objects that UIFoundation only stores, compares and sends
// messages to.
//
// Without a registered provider (a process that uses UIFoundation without
// AppKit) the functions report no context and no objects, and drawing does
// nothing.

#ifndef _UIFoundationGraphicsContext_H_
#define _UIFoundationGraphicsContext_H_

#import <Foundation/NSObject.h>
#include <CoreGraphics/CGContext.h>

@class NSData;

// Colours the text system falls back on when attributes name none.
typedef NS_ENUM(NSInteger, _UIFoundationTextColorRole) {
    // Text without a foreground colour; also underlines and strikethroughs
    // without a colour of their own.
    _UIFoundationTextColorRoleText = 0,
    // Glyphs drawn without any colour at all.
    _UIFoundationTextColorRoleControlText,
    // Selected text without a selected foreground colour.
    _UIFoundationTextColorRoleSelectedText,
    // The selection highlight without a selected background colour.
    _UIFoundationTextColorRoleSelectedTextBackground,
    // The marker under misspelled words.
    _UIFoundationTextColorRoleSpellingIndicator,
    // A new shadow's colour.
    _UIFoundationTextColorRoleShadow,
};

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

@optional

// A colour object for the role, or nil. Not retained for the caller.
- (id)textColorForRole:(_UIFoundationTextColorRole)role;

// The font used for text that has no font attribute, or nil. Not retained
// for the caller.
- (id)defaultTextFont;

// An image made from the data, or nil if the data holds no image. Not
// retained for the caller.
- (id)imageWithData:(NSData *)data;

// A new attachment cell for a text attachment, owned by the caller, or nil.
- (id)newTextAttachmentCell;

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

// Wrappers for the optional methods: nil when no provider is registered or it
// does not implement the method. Ownership is as for the methods.
FOUNDATION_EXPORT id _UIFoundationTextColorForRole(_UIFoundationTextColorRole role);
FOUNDATION_EXPORT id _UIFoundationDefaultTextFont(void);
FOUNDATION_EXPORT id _UIFoundationImageWithData(NSData *data);
FOUNDATION_EXPORT id _UIFoundationNewTextAttachmentCell(void);

#ifdef __cplusplus
}
#endif

#endif
