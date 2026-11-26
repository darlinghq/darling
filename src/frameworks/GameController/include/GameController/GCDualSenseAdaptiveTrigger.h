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

#ifndef _GCDualSenseAdaptiveTrigger_H_
#define _GCDualSenseAdaptiveTrigger_H_

#import <GameController/GCControllerButtonInput.h>

NS_ASSUME_NONNULL_BEGIN

enum {
	GCDualSenseAdaptiveTriggerDiscretePositionCount NS_SWIFT_NAME(GCDualSenseAdaptiveTrigger.discretePositionCount) = 10,
};

typedef NS_ENUM(NSInteger, GCDualSenseAdaptiveTriggerMode) {
	GCDualSenseAdaptiveTriggerModeOff = 0,
	GCDualSenseAdaptiveTriggerModeFeedback = 1,
	GCDualSenseAdaptiveTriggerModeWeapon = 2,
	GCDualSenseAdaptiveTriggerModeVibration = 3,
	GCDualSenseAdaptiveTriggerModeSlopeFeedback = 4
} NS_SWIFT_NAME(GCDualSenseAdaptiveTrigger.Mode);

typedef NS_ENUM(NSInteger, GCDualSenseAdaptiveTriggerStatus) {
	GCDualSenseAdaptiveTriggerStatusUnknown = -1,
	GCDualSenseAdaptiveTriggerStatusFeedbackNoLoad,
	GCDualSenseAdaptiveTriggerStatusFeedbackLoadApplied,
	GCDualSenseAdaptiveTriggerStatusWeaponReady,
	GCDualSenseAdaptiveTriggerStatusWeaponFiring,
	GCDualSenseAdaptiveTriggerStatusWeaponFired,
	GCDualSenseAdaptiveTriggerStatusVibrationNotVibrating,
	GCDualSenseAdaptiveTriggerStatusVibrationIsVibrating,
	GCDualSenseAdaptiveTriggerStatusSlopeFeedbackReady,
	GCDualSenseAdaptiveTriggerStatusSlopeFeedbackApplyingLoad,
	GCDualSenseAdaptiveTriggerStatusSlopeFeedbackFinished,
} NS_SWIFT_NAME(GCDualSenseAdaptiveTrigger.Status);

typedef struct {
	float values[GCDualSenseAdaptiveTriggerDiscretePositionCount];
} GCDualSenseAdaptiveTriggerPositionalAmplitudes NS_SWIFT_NAME(GCDualSenseAdaptiveTrigger.PositionalAmplitudes);

typedef struct {
	float values[GCDualSenseAdaptiveTriggerDiscretePositionCount];
} GCDualSenseAdaptiveTriggerPositionalResistiveStrengths NS_SWIFT_NAME(GCDualSenseAdaptiveTrigger.PositionalResistiveStrengths);

/* A trigger of a DualSense controller, whose resistance the application can program. The modes are
 * recorded and reported back; this implementation does not send them to the controller. */
@interface GCDualSenseAdaptiveTrigger : GCControllerButtonInput

@property (nonatomic, readonly) GCDualSenseAdaptiveTriggerMode mode;
@property (nonatomic, readonly) GCDualSenseAdaptiveTriggerStatus status;
@property (nonatomic, readonly) float armPosition;

- (void)setModeSlopeFeedbackWithStartPosition:(float)startPosition endPosition:(float)endPosition startStrength:(float)startStrength endStrength:(float)endStrength NS_SWIFT_NAME(setModeSlopeFeedback(startPosition:endPosition:startStrength:endStrength:));
- (void)setModeFeedbackWithStartPosition:(float)startPosition resistiveStrength:(float)resistiveStrength;
- (void)setModeFeedbackWithResistiveStrengths:(GCDualSenseAdaptiveTriggerPositionalResistiveStrengths)positionalResistiveStrengths NS_SWIFT_NAME(setModeFeedback(resistiveStrengths:));
- (void)setModeWeaponWithStartPosition:(float)startPosition endPosition:(float)endPosition resistiveStrength:(float)resistiveStrength;
- (void)setModeVibrationWithStartPosition:(float)startPosition amplitude:(float)amplitude frequency:(float)frequency;
- (void)setModeVibrationWithAmplitudes:(GCDualSenseAdaptiveTriggerPositionalAmplitudes)positionalAmplitudes frequency:(float)frequency NS_SWIFT_NAME(setModeVibration(amplitudes:frequency:));
- (void)setModeOff;

@end

NS_ASSUME_NONNULL_END

#endif
