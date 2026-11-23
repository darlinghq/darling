/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#import <AppKit/AppKitIvarLayout.h>
#import <AppKit/NSView.h>
#include <Foundation/Foundation.h>
#include <QTKit/QTKitDefines.h>

@class QTMovie;

// Legacy ObjC ABI (i386): instance size must stay 0x74, Apple's (NSView's 0x50
// plus 0x24 of Apple's own state, which we don't keep). An application's
// subclass puts its ivars right after it. Don't add ivars here.
@interface QTMovieView : NSView {
    NS_LEGACY_IVAR_PAD(_QTMovieViewReserved, 0x24)
}

- (QTMovie *)movie;
- (void)setMovie:(QTMovie *)movie;

- (void)play:(id)sender;
- (void)pause:(id)sender;
- (void)gotoBeginning:(id)sender;
- (void)gotoEnd:(id)sender;
- (void)stepForward:(id)sender;
- (void)stepBackward:(id)sender;
- (void)gotoPosterTime:(id)sender;

- (BOOL)isControllerVisible;
- (void)setControllerVisible:(BOOL)visible;
- (BOOL)preservesAspectRatio;
- (void)setPreservesAspectRatio:(BOOL)preserves;
- (NSColor *)fillColor;
- (void)setFillColor:(NSColor *)fillColor;
- (NSRect)movieBounds;

- (BOOL)isEditable;
- (void)setEditable:(BOOL)editable;

@end

QTKIT_EXTERN NSString * const QTMovieViewControllerVisibleBinding;
QTKIT_EXTERN NSString * const QTMovieViewFillColorBinding;
QTKIT_EXTERN NSString * const QTMovieViewMovieBinding;
QTKIT_EXTERN NSString * const QTMovieViewPreservesAspectRatioBinding;
