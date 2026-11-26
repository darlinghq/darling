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

// The implementation is 64-bit only, like GameController on macOS (its 32-bit
// Objective-C ABI cannot hold instance variables declared in the @implementation
// and the framework is x86_64 and arm64 only there); the 32-bit slice keeps the stub.
#ifdef __LP64__
#import <GameController/GCDualSenseAdaptiveTrigger.h>
#import "GCInputInternal.h"

// The programmed mode is recorded and reported back through -mode and -status once the profile belongs to a
// controller (a detached profile ignores it, as macOS does); the host side of the controller has no output
// path for it yet.
@implementation GCDualSenseAdaptiveTrigger {
	GCDualSenseAdaptiveTriggerMode _mode;
	GCDualSenseAdaptiveTriggerStatus _status;
	float _armPosition;
}

- (GCDualSenseAdaptiveTriggerMode)mode { return _mode; }
- (GCDualSenseAdaptiveTriggerStatus)status { return _status; }
- (float)armPosition { return _armPosition; }

- (void)setModeSlopeFeedbackWithStartPosition:(float)startPosition endPosition:(float)endPosition startStrength:(float)startStrength endStrength:(float)endStrength
{
	if (![[self _profile] device])
		return;
	_mode = GCDualSenseAdaptiveTriggerModeSlopeFeedback;
	_status = GCDualSenseAdaptiveTriggerStatusSlopeFeedbackReady;
}

- (void)setModeFeedbackWithStartPosition:(float)startPosition resistiveStrength:(float)resistiveStrength
{
	if (![[self _profile] device])
		return;
	_mode = GCDualSenseAdaptiveTriggerModeFeedback;
	_status = GCDualSenseAdaptiveTriggerStatusFeedbackNoLoad;
}

- (void)setModeFeedbackWithResistiveStrengths:(GCDualSenseAdaptiveTriggerPositionalResistiveStrengths)positionalResistiveStrengths
{
	if (![[self _profile] device])
		return;
	_mode = GCDualSenseAdaptiveTriggerModeFeedback;
	_status = GCDualSenseAdaptiveTriggerStatusFeedbackNoLoad;
}

- (void)setModeWeaponWithStartPosition:(float)startPosition endPosition:(float)endPosition resistiveStrength:(float)resistiveStrength
{
	if (![[self _profile] device])
		return;
	_mode = GCDualSenseAdaptiveTriggerModeWeapon;
	_status = GCDualSenseAdaptiveTriggerStatusWeaponReady;
}

- (void)setModeVibrationWithStartPosition:(float)startPosition amplitude:(float)amplitude frequency:(float)frequency
{
	if (![[self _profile] device])
		return;
	_mode = GCDualSenseAdaptiveTriggerModeVibration;
	_status = GCDualSenseAdaptiveTriggerStatusVibrationNotVibrating;
}

- (void)setModeVibrationWithAmplitudes:(GCDualSenseAdaptiveTriggerPositionalAmplitudes)positionalAmplitudes frequency:(float)frequency
{
	if (![[self _profile] device])
		return;
	_mode = GCDualSenseAdaptiveTriggerModeVibration;
	_status = GCDualSenseAdaptiveTriggerStatusVibrationNotVibrating;
}

- (void)setModeOff
{
	if (![[self _profile] device])
		return;
	_mode = GCDualSenseAdaptiveTriggerModeOff;
	_status = GCDualSenseAdaptiveTriggerStatusUnknown;
}

@end
#else
#import <GameController/GCDualSenseAdaptiveTrigger.h>
#import <darling-stub-forwarding.h>

@implementation GCDualSenseAdaptiveTrigger

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

+ (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
    return [super methodSignatureForSelector: aSelector] ?: DarlingStubMethodSignature(aSelector);
}

+ (void)forwardInvocation:(NSInvocation *)anInvocation
{
    DarlingStubForwardInvocation(self, anInvocation);
}

@end

#endif /* __LP64__ */
