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

// Families of profiles. They differ in the names, symbols and defaults of their elements.
typedef NS_ENUM(NSInteger, GCProfileStyle) {
	GCProfileStyleGeneric,
	GCProfileStyleXbox,
	GCProfileStyleDualShock,
	GCProfileStyleDualSense,
	GCProfileStyleMicro,
};

// The names and defaults of one element, which is not a child of a direction pad, in a family.
typedef struct {
	const char *key;
	const char *localizedName;
	const char *sfSymbolsName;
	BOOL analog;
	BOOL boundToSystemGesture;
} GCElementSpec;

const GCElementSpec *GCElementSpecsForStyle(GCProfileStyle style, size_t *count);

typedef struct {
	GCKeyCode code;
	NSString *const *name;
} GCKeyTableEntry;

extern const GCKeyTableEntry GCKeyTable[];
extern const size_t GCKeyTableCount;

// The pressed state of a button follows its value: a value that rounds to nothing is not a press.
#define GC_PRESS_THRESHOLD 0.01f

@interface GCControllerElement ()
- (instancetype)initWithCollection:(GCControllerElement *)collection isAnalog:(BOOL)analog aliases:(NSSet<NSString *> *)aliases;
- (void)_setAnalog:(BOOL)analog;
- (void)_setAliases:(NSSet<NSString *> *)aliases;
- (void)_setBoundToSystemGesture:(BOOL)bound;
// The profile an element belongs to (not retained); the profile decides whether the public setters may write.
- (void)_setProfile:(GCPhysicalInputProfile *)profile;
- (GCPhysicalInputProfile *)_profile;
- (BOOL)_allowsPublicWrites;
- (dispatch_queue_t)_handlerQueue;
// Gives the element the state of the element with the same role in another profile.
- (void)_copyStateFromElement:(GCControllerElement *)element;
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
// Names the pad and its children after the key the pad is registered under in its profile.
- (void)_assignAliasesForKey:(NSString *)key;
@end

@interface GCPhysicalInputProfile ()
- (instancetype)initWithDevice:(id<GCDevice>)device;
- (void)_registerElement:(GCControllerElement *)element key:(NSString *)key;
- (void)_registerButton:(GCControllerButtonInput *)button key:(NSString *)key;
- (void)_registerAxis:(GCControllerAxisInput *)axis key:(NSString *)key;
- (void)_registerDpad:(GCControllerDirectionPad *)dpad key:(NSString *)key;
- (void)_registerTouchpad:(GCControllerTouchpad *)touchpad key:(NSString *)key;
- (void)_registerAlias:(NSString *)alias forElement:(GCControllerElement *)element;
- (void)_forceStateFromPhysicalInput:(GCPhysicalInputProfile *)physicalInput;
// Sets the names, symbols and analog/gesture defaults of the registered elements of a family.
- (void)_applyStyle:(GCProfileStyle)style;
- (void)_noteEvent;
- (void)_clearDevice;
- (dispatch_queue_t)_handlerQueue;
@end

@interface GCExtendedGamepad ()
- (instancetype)initWithController:(GCController *)controller;
- (instancetype)initWithController:(GCController *)controller style:(GCProfileStyle)style;
@end

@interface GCMicroGamepad ()
- (instancetype)initWithController:(GCController *)controller;
@end

@interface GCGamepad ()
- (instancetype)initWithController:(GCController *)controller;
@end

@interface GCController ()
// A controller created by the application: not attached to a device, its profiles accept writes.
- (instancetype)_initSnapshotWithExtendedGamepad:(BOOL)extended;
@end

// The data of a snapshot are valid when they hold a header and the size they declare fits in them.
static inline BOOL GCSnapshotHeaderIsValid(NSData *data)
{
	if (!data || [data length] < 4)
		return NO;
	uint16_t size;
	memcpy(&size, (const uint8_t *)[data bytes] + 2, sizeof(size));
	return size <= [data length];
}

// The state of a profile as the data of the snapshot classes, and back. Applying data that is not valid
// changes nothing and returns NO.
NSData *GCExtendedGamepadStateData(GCExtendedGamepad *gamepad);
BOOL GCApplyExtendedGamepadStateData(GCExtendedGamepad *gamepad, NSData *data);
NSData *GCMicroGamepadStateData(GCMicroGamepad *gamepad);
BOOL GCApplyMicroGamepadStateData(GCMicroGamepad *gamepad, NSData *data);
NSData *GCGamepadStateData(GCGamepad *gamepad);
BOOL GCApplyGamepadStateData(GCGamepad *gamepad, NSData *data);

@interface GCMotion ()
- (instancetype)initWithController:(GCController *)controller attitudeAndRotationRate:(BOOL)attitude;
@end

#endif
