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

#import <GameController/GCMouse.h>

NSString *const GCMouseDidConnectNotification = @"GCMouseDidConnectNotification";
NSString *const GCMouseDidDisconnectNotification = @"GCMouseDidDisconnectNotification";

@implementation GCMouseInput

@synthesize mouseMovedHandler = _mouseMovedHandler;

- (void)dealloc
{
	[_mouseMovedHandler release];
	[super dealloc];
}

- (GCDeviceButtonInput *)leftButton
{
	return nil;
}

- (GCDeviceButtonInput *)rightButton
{
	return nil;
}

- (GCDeviceButtonInput *)middleButton
{
	return nil;
}

- (NSArray *)auxiliaryButtons
{
	return nil;
}

- (GCDeviceDirectionPad *)scroll
{
	return nil;
}

@end

@implementation GCMouse

+ (NSArray<GCMouse *> *)mice
{
	return @[];
}

+ (GCMouse *)current
{
	return nil;
}

- (GCMouseInput *)mouseInput
{
	return nil;
}

- (NSString *)vendorName
{
	return nil;
}

- (NSString *)productCategory
{
	return @"Mouse";
}

- (BOOL)isAttachedToDevice
{
	return NO;
}

- (dispatch_queue_t)handlerQueue
{
	return dispatch_get_main_queue();
}

- (void)setHandlerQueue:(dispatch_queue_t)queue
{
}

@end
