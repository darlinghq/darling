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

#ifndef _GCMouse_H_
#define _GCMouse_H_

#import <Foundation/Foundation.h>
#import <GameController/GCPhysicalInputProfile.h>

NS_ASSUME_NONNULL_BEGIN

/* Posted when a mouse is connected or disconnected; the object is the GCMouse. */
extern NSString *const GCMouseDidConnectNotification;
extern NSString *const GCMouseDidDisconnectNotification;

@class GCMouseInput;

typedef void (^GCMouseMoved)(GCMouseInput *mouse, float deltaX, float deltaY);

@interface GCMouseInput : GCPhysicalInputProfile {
@private
	GCMouseMoved _mouseMovedHandler;
}

@property(nonatomic, copy, nullable) GCMouseMoved mouseMovedHandler;

@property(readonly) GCDeviceButtonInput *leftButton;
@property(readonly, nullable) GCDeviceButtonInput *rightButton;
@property(readonly, nullable) GCDeviceButtonInput *middleButton;
@property(readonly, nullable) NSArray<GCDeviceButtonInput *> *auxiliaryButtons;
@property(readonly) GCDeviceDirectionPad *scroll;

@end

@interface GCMouse : NSObject

/* All connected mice, and the one that most recently produced input. Darling does
 * not receive mice through this framework, so the list is empty and current is nil. */
@property(class, readonly) NSArray<GCMouse *> *mice;
@property(class, readonly, nullable) GCMouse *current;

@property(readonly) GCMouseInput *mouseInput;
@property(readonly, copy, nullable) NSString *vendorName;
@property(readonly, copy) NSString *productCategory;
@property(readonly, getter=isAttachedToDevice) BOOL attachedToDevice;
@property(nonatomic, strong, null_resettable) dispatch_queue_t handlerQueue;

@end

NS_ASSUME_NONNULL_END

#endif
