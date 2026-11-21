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

// The tree of parameters an audio unit publishes: nodes, groups, the tree
// itself and single parameters (macOS 10.11 and later).

#ifndef AudioToolbox_AUParameters_h
#define AudioToolbox_AUParameters_h

#ifdef __OBJC2__

#import <AudioToolbox/AUComponent.h>
#import <AudioToolbox/AudioUnitProperties.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class AUParameter;

// The value of a parameter.
typedef float AUValue;

// Numeric identifier of a parameter within a tree. Not guaranteed persistent;
// hosts that store parameters should use the key path.
typedef uint64_t AUParameterAddress;

// What a parameter change represents when it comes from a user interface
// gesture: a plain value change, the start of a gesture, or its end.
typedef NS_ENUM(uint32_t, AUParameterAutomationEventType) {
	AUParameterAutomationEventTypeValue = 0,
	AUParameterAutomationEventTypeTouch = 1,
	AUParameterAutomationEventTypeRelease = 2
};

// A parameter change recorded with the host time at which it happened.
typedef struct AURecordedParameterEvent {
	uint64_t hostTime;
	AUParameterAddress address;
	AUValue value;
} AURecordedParameterEvent;

// A parameter change or gesture marker with the host time at which it happened.
typedef struct AUParameterAutomationEvent {
	uint64_t hostTime;
	AUParameterAddress address;
	AUValue value;
	AUParameterAutomationEventType eventType;
	uint64_t reserved;
} AUParameterAutomationEvent;

// Called after the value of a parameter changed.
typedef void (^AUParameterObserver)(AUParameterAddress address, AUValue value);

// Called with the changes of parameter values as recorded events.
typedef void (^AUParameterRecordingObserver)(NSInteger numberEvents, const AURecordedParameterEvent *events);

// Called with the changes of parameter values and gestures as automation events.
typedef void (^AUParameterAutomationObserver)(NSInteger numberEvents, const AUParameterAutomationEvent *events);

// Identifies an installed observer.
typedef void *AUParameterObserverToken;

// A node of a parameter tree: either a group or a parameter.
NS_CLASS_AVAILABLE(10_11, 9_0)
@interface AUParameterNode : NSObject

// Permanent, non-localized name; unique among the children of one parent.
@property (NS_NONATOMIC_IOSONLY, readonly, copy) NSString *identifier;

// The identifiers of the node's ancestors and the node itself, joined with
// periods. Passing it to -valueForKeyPath: of the tree returns the node.
@property (NS_NONATOMIC_IOSONLY, readonly, copy) NSString *keyPath;

// Localized name for display.
@property (NS_NONATOMIC_IOSONLY, readonly, copy) NSString *displayName;

// The display name, possibly abbreviated to the given number of characters.
- (NSString *)displayNameWithLength:(NSInteger)maximumLength;

// Adds an observer of one parameter, or of all parameters of a group or tree.
// It is called on an arbitrary thread after the value changed.
- (AUParameterObserverToken)tokenByAddingParameterObserver:(AUParameterObserver)observer;

// Like the above, but receives recorded events (value changes only).
- (AUParameterObserverToken)tokenByAddingParameterRecordingObserver:(AUParameterRecordingObserver)observer;

// Like the above, but receives automation events including gesture markers.
- (AUParameterObserverToken)tokenByAddingParameterAutomationObserver:(AUParameterAutomationObserver)observer NS_AVAILABLE(10_12, 10_0);

// Removes an observer. Waits for notifications already in flight.
- (void)removeParameterObserver:(AUParameterObserverToken)token;

@end

// A group of related parameters, KVC-compliant for its children: the value for
// a key is the child with that identifier.
NS_CLASS_AVAILABLE(10_11, 9_0)
@interface AUParameterGroup : AUParameterNode <NSSecureCoding>

@property (NS_NONATOMIC_IOSONLY, readonly) NSArray<AUParameterNode *> *children;

// All parameters below the group, depth first.
@property (NS_NONATOMIC_IOSONLY, readonly) NSArray<AUParameter *> *allParameters;

@end

// The top-level group of all of an audio unit's parameters.
NS_CLASS_AVAILABLE(10_11, 9_0)
@interface AUParameterTree : AUParameterGroup <NSSecureCoding>

// The parameter with the given address, or nil.
- (AUParameter *_Nullable)parameterWithAddress:(AUParameterAddress)address;

// Looks a parameter up by the identification of a version 2 audio unit
// parameter. Returns nil when the tree does not come from such a unit.
- (AUParameter *_Nullable)parameterWithID:(AudioUnitParameterID)paramID scope:(AudioUnitScope)scope element:(AudioUnitElement)element;

@end

// A single parameter.
NS_CLASS_AVAILABLE(10_11, 9_0)
@interface AUParameter : AUParameterNode <NSSecureCoding>

@property (NS_NONATOMIC_IOSONLY, readonly) AUValue minValue;
@property (NS_NONATOMIC_IOSONLY, readonly) AUValue maxValue;
@property (NS_NONATOMIC_IOSONLY, readonly) AudioUnitParameterUnit unit;
@property (NS_NONATOMIC_IOSONLY, readonly, copy, nullable) NSString *unitName;
@property (NS_NONATOMIC_IOSONLY, readonly) AudioUnitParameterOptions flags;
@property (NS_NONATOMIC_IOSONLY, readonly) AUParameterAddress address;

// Names of the values of an indexed parameter.
@property (NS_NONATOMIC_IOSONLY, readonly, copy, nullable) NSArray<NSString *> *valueStrings;

// Addresses (as NSNumber) of the parameters whose values may change when this one changes.
@property (NS_NONATOMIC_IOSONLY, readonly, copy, nullable) NSArray<NSNumber *> *dependentParameters;

// The current value. Setting it notifies the audio unit and all observers.
@property (NS_NONATOMIC_IOSONLY) AUValue value;

// Sets the value without notifying the observer identified by originator.
- (void)setValue:(AUValue)value originator:(AUParameterObserverToken _Nullable)originator;

// As above, tagged with the host time of the change.
- (void)setValue:(AUValue)value originator:(AUParameterObserverToken _Nullable)originator atHostTime:(uint64_t)hostTime;

// As above, also telling whether the change is a plain value, or the start or the end of a gesture.
- (void)setValue:(AUValue)value originator:(AUParameterObserverToken _Nullable)originator atHostTime:(uint64_t)hostTime eventType:(AUParameterAutomationEventType)eventType NS_AVAILABLE(10_12, 10_0);

// String for a value (the current one when NULL).
- (NSString *)stringFromValue:(const AUValue *_Nullable)value;

// Value for a string.
- (AUValue)valueFromString:(NSString *)string;

@end

NS_ASSUME_NONNULL_END

#endif // __OBJC2__

#endif // AudioToolbox_AUParameters_h
