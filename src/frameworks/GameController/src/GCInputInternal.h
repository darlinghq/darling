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

#ifndef _GCInputInternal_H_
#define _GCInputInternal_H_

#import <GameController/GameController.h>

@interface GCControllerElement ()
- (instancetype)initWithCollection:(GCControllerElement *)collection isAnalog:(BOOL)analog aliases:(NSSet<NSString *> *)aliases;
@end

@interface GCControllerButtonInput ()
- (void)_setValue:(float)value handlerQueue:(dispatch_queue_t)queue profileHandler:(void (^)(GCControllerElement *))profileHandler;
@end

@interface GCControllerAxisInput ()
- (void)_setValue:(float)value handlerQueue:(dispatch_queue_t)queue profileHandler:(void (^)(GCControllerElement *))profileHandler;
@end

@interface GCControllerDirectionPad ()
- (instancetype)initWithCollection:(GCControllerElement *)collection isAnalog:(BOOL)analog aliases:(NSSet<NSString *> *)aliases;
- (void)_setXAxis:(float)x yAxis:(float)y handlerQueue:(dispatch_queue_t)queue profileHandler:(void (^)(GCControllerElement *))profileHandler;
@end

@interface GCPhysicalInputProfile ()
- (instancetype)initWithDevice:(id<GCDevice>)device;
- (void)_registerElement:(GCControllerElement *)element key:(NSString *)key;
- (void)_registerButton:(GCControllerButtonInput *)button key:(NSString *)key;
- (void)_registerAxis:(GCControllerAxisInput *)axis key:(NSString *)key;
- (void)_registerDpad:(GCControllerDirectionPad *)dpad key:(NSString *)key;
@end

@interface GCExtendedGamepad ()
- (instancetype)initWithController:(GCController *)controller;
@end

@interface GCMicroGamepad ()
- (instancetype)initWithController:(GCController *)controller;
@end

#endif
