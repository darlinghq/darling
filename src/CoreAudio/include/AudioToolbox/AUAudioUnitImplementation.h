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

// The parts of the audio unit implementer interface that concern parameter
// trees: factory methods and the implementor hooks on the nodes. The
// AUAudioUnit subclassing interface is not provided.

#ifndef AudioToolbox_AUAudioUnitImplementation_h
#define AudioToolbox_AUAudioUnitImplementation_h

#ifdef __OBJC2__

#import <AudioToolbox/AUParameters.h>

NS_ASSUME_NONNULL_BEGIN

// Factory methods for parameters, groups and trees. In non-ARC code they
// return autoreleased objects.
@interface AUParameterTree (Factory)

+ (AUParameter *)createParameterWithIdentifier:(NSString *)identifier name:(NSString *)name address:(AUParameterAddress)address min:(AUValue)min max:(AUValue)max unit:(AudioUnitParameterUnit)unit unitName:(NSString *_Nullable)unitName flags:(AudioUnitParameterOptions)flags valueStrings:(NSArray<NSString *> *_Nullable)valueStrings dependentParameters:(NSArray<NSNumber *> *_Nullable)dependentParameters;

+ (AUParameterGroup *)createGroupWithIdentifier:(NSString *)identifier name:(NSString *)name children:(NSArray<AUParameterNode *> *)children;

// A group without identifier and name, to be instantiated several times with
// -createGroupFromTemplate:identifier:name:addressOffset:.
+ (AUParameterGroup *)createGroupTemplate:(NSArray<AUParameterNode *> *)children;

// A copy of the template whose parameters have their addresses moved by addressOffset.
+ (AUParameterGroup *)createGroupFromTemplate:(AUParameterGroup *)templateGroup identifier:(NSString *)identifier name:(NSString *)name addressOffset:(AUParameterAddress)addressOffset;

+ (AUParameterTree *)createTreeWithChildren:(NSArray<AUParameterNode *> *)children;

@end

// Called when a parameter changes value, so the audio unit can apply it.
typedef void (^AUImplementorValueObserver)(AUParameter *param, AUValue value);

// Called to fetch the current value of a parameter from the audio unit.
typedef AUValue (^AUImplementorValueProvider)(AUParameter *param);

// Called to convert a value (the current one when NULL) to a string.
typedef NSString *__nonnull (^AUImplementorStringFromValueCallback)(AUParameter *param, const AUValue *__nullable value);

// Called to convert a string to a value.
typedef AUValue (^AUImplementorValueFromStringCallback)(AUParameter *param, NSString *string);

// Called to get the display name of a node abbreviated to the desired length.
typedef NSString *__nonnull (^AUImplementorDisplayNameWithLengthCallback)(AUParameterNode *node, NSInteger desiredLength);

// Hooks for the audio unit implementation. A hook set on a group or a tree
// applies to the nodes below it that have no hook of their own.
@interface AUParameterNode (AUParameterNodeImplementation)

@property (NS_NONATOMIC_IOSONLY, copy) AUImplementorValueObserver implementorValueObserver;
@property (NS_NONATOMIC_IOSONLY, copy) AUImplementorValueProvider implementorValueProvider;
@property (NS_NONATOMIC_IOSONLY, copy) AUImplementorStringFromValueCallback implementorStringFromValueCallback;
@property (NS_NONATOMIC_IOSONLY, copy) AUImplementorValueFromStringCallback implementorValueFromStringCallback;
@property (NS_NONATOMIC_IOSONLY, copy) AUImplementorDisplayNameWithLengthCallback implementorDisplayNameWithLengthCallback;

@end

NS_ASSUME_NONNULL_END

#endif // __OBJC2__

#endif // AudioToolbox_AUAudioUnitImplementation_h
