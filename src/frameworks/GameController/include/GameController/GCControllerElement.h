/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel
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

#ifndef _GCControllerElement_H_
#define _GCControllerElement_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/* How input for an element that a system gesture is bound to is handled in the application. */
typedef NS_ENUM(NSInteger, GCSystemGestureState) {
	GCSystemGestureStateEnabled = 0,
	GCSystemGestureStateAlwaysReceive,
	GCSystemGestureStateDisabled
} NS_SWIFT_NAME(GCControllerElement.SystemGestureState);

@interface GCControllerElement : NSObject

@property (nonatomic, weak, readonly, nullable) GCControllerElement *collection;
@property (nonatomic, readonly, getter=isAnalog) BOOL analog;
@property (nonatomic, copy, readonly) NSSet<NSString *> *aliases;

/* Whether a system gesture is bound to the element (NO unless the controller's profile says otherwise). */
@property (nonatomic, readonly, getter=isBoundToSystemGesture) BOOL boundToSystemGesture;

/* The way the application wants input of the element handled; the system may not honour it. */
@property (nonatomic, readwrite) GCSystemGestureState preferredSystemGestureState;

/* Symbol and display names of the element, with and without the user's remapping applied. */
@property (nonatomic, strong, nullable) NSString *sfSymbolsName;
@property (nonatomic, strong, nullable) NSString *localizedName;
@property (nonatomic, strong, nullable) NSString *unmappedSfSymbolsName;
@property (nonatomic, strong, nullable) NSString *unmappedLocalizedName;

@end

NS_ASSUME_NONNULL_END

#endif
