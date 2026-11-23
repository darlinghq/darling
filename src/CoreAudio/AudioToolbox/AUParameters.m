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

// Parameter trees of audio units: AUParameterNode, AUParameterGroup,
// AUParameterTree and AUParameter (AUParameters.h, AUAudioUnitImplementation.h).

// The headers are for the modern Objective-C runtime only, so the 32-bit slice has no parameter trees.
#ifdef __OBJC2__

#import <AudioToolbox/AUAudioUnitImplementation.h>
#import <dispatch/dispatch.h>
#import <mach/mach_time.h>
#import <stdlib.h>

static NSString *const AUInvalidNodeInfoException = @"AUInvalidNodeInfoException";

typedef enum {
	AUObserverKindPlain,
	AUObserverKindRecording,
	AUObserverKindAutomation,
} AUObserverKind;

// One installed observer. The token handed to clients is the address of this object.
@interface AUParameterObserverRecord : NSObject {
@public
	AUObserverKind kind;
	id block;
}
@end

@implementation AUParameterObserverRecord

- (void) dealloc {
	[block release];
	[super dealloc];
}

@end

// Notifications are delivered in order on one queue, away from the thread that changed the value.
static char AUParameterNotificationQueueKey;

static dispatch_queue_t AUParameterNotificationQueue(void) {
	static dispatch_queue_t queue;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		queue = dispatch_queue_create("com.apple.audio.AUParameterTree.notifications", DISPATCH_QUEUE_SERIAL);
		dispatch_queue_set_specific(queue, &AUParameterNotificationQueueKey, &AUParameterNotificationQueueKey, NULL);
	});
	return queue;
}

static BOOL AUParameterOnNotificationQueue(void) {
	return dispatch_get_specific(&AUParameterNotificationQueueKey) != NULL;
}

@interface AUParameterNode () {
@protected
	NSString *_identifier;
	NSString *_displayName;
	AUParameterNode *_parent; // not retained; a group clears it in dealloc
	NSMutableArray *_observers;
	AUImplementorValueObserver _implementorValueObserver;
	AUImplementorValueProvider _implementorValueProvider;
	AUImplementorStringFromValueCallback _implementorStringFromValueCallback;
	AUImplementorValueFromStringCallback _implementorValueFromStringCallback;
	AUImplementorDisplayNameWithLengthCallback _implementorDisplayNameWithLengthCallback;
}

- (instancetype) initWithIdentifier: (NSString *) identifier displayName: (NSString *) displayName;
- (AUParameterNode *) parentNode;
- (void) setParentNode: (AUParameterNode *) parent;
- (void) collectObserversInto: (NSMutableArray *) records excluding: (AUParameterObserverToken) originator;
- (id) nearestHook: (SEL) getter;

@end

@interface AUParameterGroup () {
@protected
	NSArray *_children;
}

- (instancetype) initWithIdentifier: (NSString *) identifier displayName: (NSString *) displayName children: (NSArray *) children;
- (AUParameterGroup *) copyWithAddressOffset: (AUParameterAddress) offset identifier: (NSString *) identifier displayName: (NSString *) displayName;
- (void) collectParametersInto: (NSMutableArray *) parameters;

@end

@interface AUParameter () {
@protected
	AUValue _minValue;
	AUValue _maxValue;
	AudioUnitParameterUnit _unit;
	NSString *_unitName;
	AudioUnitParameterOptions _flags;
	AUParameterAddress _address;
	NSArray *_valueStrings;
	NSArray *_dependentParameters;
	AUValue _value;
	BOOL _stale;
}

- (instancetype) initWithIdentifier: (NSString *) identifier name: (NSString *) name address: (AUParameterAddress) address min: (AUValue) min max: (AUValue) max unit: (AudioUnitParameterUnit) unit unitName: (NSString *) unitName flags: (AudioUnitParameterOptions) flags valueStrings: (NSArray *) valueStrings dependentParameters: (NSArray *) dependentParameters;
- (AUParameter *) copyWithAddressOffset: (AUParameterAddress) offset;

@end

@interface AUParameterTree () {
@protected
	NSDictionary *_parametersByAddress;
}

- (instancetype) initWithChildren: (NSArray *) children;
- (BOOL) indexParameters;

@end

@implementation AUParameterNode

- (instancetype) initWithIdentifier: (NSString *) identifier displayName: (NSString *) displayName {
	self = [super init];
	if (self) {
		_identifier = [identifier copy];
		_displayName = [displayName copy];
		_observers = [[NSMutableArray alloc] init];
	}
	return self;
}

- (void) dealloc {
	[_identifier release];
	[_displayName release];
	[_observers release];
	[_implementorValueObserver release];
	[_implementorValueProvider release];
	[_implementorStringFromValueCallback release];
	[_implementorValueFromStringCallback release];
	[_implementorDisplayNameWithLengthCallback release];
	[super dealloc];
}

+ (BOOL) supportsSecureCoding {
	return YES;
}

- (instancetype) initWithCoder: (NSCoder *) coder {
	NSString *identifier = [coder decodeObjectOfClass: [NSString class] forKey: @"identifier"];
	NSString *displayName = [coder decodeObjectOfClass: [NSString class] forKey: @"displayName"];

	return [self initWithIdentifier: identifier ?: @"" displayName: displayName ?: @""];
}

- (void) encodeWithCoder: (NSCoder *) coder {
	[coder encodeObject: _identifier forKey: @"identifier"];
	[coder encodeObject: _displayName forKey: @"displayName"];
}

- (NSString *) identifier {
	return _identifier;
}

- (NSString *) displayName {
	return _displayName;
}

- (AUParameterNode *) parentNode {
	return _parent;
}

- (void) setParentNode: (AUParameterNode *) parent {
	_parent = parent;
}

- (NSString *) keyPath {
	NSString *parentPath = [_parent keyPath];

	// Template groups and trees have empty identifiers and add nothing to the path.
	if ([parentPath length] == 0)
		return [[_identifier copy] autorelease];
	if ([_identifier length] == 0)
		return parentPath;
	return [NSString stringWithFormat: @"%@.%@", parentPath, _identifier];
}

// The hook set on this node, else on the closest ancestor having one.
- (id) nearestHook: (SEL) getter {
	for (AUParameterNode *node = self; node; node = node->_parent) {
		id hook = [node performSelector: getter];

		if (hook)
			return hook;
	}
	return nil;
}

- (NSString *) displayNameWithLength: (NSInteger) maximumLength {
	AUImplementorDisplayNameWithLengthCallback callback = [self nearestHook: @selector(implementorDisplayNameWithLengthCallback)];

	if (callback)
		return callback(self, maximumLength);
	return [self displayName];
}

- (AUParameterObserverToken) addObserverOfKind: (AUObserverKind) kind block: (id) block {
	AUParameterObserverRecord *record = [[AUParameterObserverRecord alloc] init];

	record->kind = kind;
	record->block = [block copy];
	@synchronized (self) {
		[_observers addObject: record];
	}
	[record release];
	return (AUParameterObserverToken) record;
}

- (AUParameterObserverToken) tokenByAddingParameterObserver: (AUParameterObserver) observer {
	return [self addObserverOfKind: AUObserverKindPlain block: observer];
}

- (AUParameterObserverToken) tokenByAddingParameterRecordingObserver: (AUParameterRecordingObserver) observer {
	return [self addObserverOfKind: AUObserverKindRecording block: observer];
}

- (AUParameterObserverToken) tokenByAddingParameterAutomationObserver: (AUParameterAutomationObserver) observer {
	return [self addObserverOfKind: AUObserverKindAutomation block: observer];
}

- (void) removeParameterObserver: (AUParameterObserverToken) token {
	BOOL removed = NO;

	@synchronized (self) {
		for (NSUInteger i = 0; i < [_observers count]; i++) {
			if ((void *) [_observers objectAtIndex: i] == token) {
				[_observers removeObjectAtIndex: i];
				removed = YES;
				break;
			}
		}
	}
	// Notifications already queued for this observer finish before the call returns.
	if (removed && !AUParameterOnNotificationQueue())
		dispatch_sync(AUParameterNotificationQueue(), ^{});
}

// The observers of this node and its ancestors, nearest first, except the one the token names.
- (void) collectObserversInto: (NSMutableArray *) records excluding: (AUParameterObserverToken) originator {
	for (AUParameterNode *node = self; node; node = node->_parent) {
		@synchronized (node) {
			for (AUParameterObserverRecord *record in node->_observers) {
				if ((void *) record != originator)
					[records addObject: record];
			}
		}
	}
}

#define AU_HOOK_ACCESSORS(type, name, ivar, setter) \
- (type) name { \
	@synchronized (self) { \
		return [[ivar retain] autorelease]; \
	} \
} \
- (void) setter: (type) hook { \
	@synchronized (self) { \
		if (hook != ivar) { \
			[ivar release]; \
			ivar = [hook copy]; \
		} \
	} \
}

AU_HOOK_ACCESSORS(AUImplementorValueObserver, implementorValueObserver, _implementorValueObserver, setImplementorValueObserver)
AU_HOOK_ACCESSORS(AUImplementorValueProvider, implementorValueProvider, _implementorValueProvider, setImplementorValueProvider)
AU_HOOK_ACCESSORS(AUImplementorStringFromValueCallback, implementorStringFromValueCallback, _implementorStringFromValueCallback, setImplementorStringFromValueCallback)
AU_HOOK_ACCESSORS(AUImplementorValueFromStringCallback, implementorValueFromStringCallback, _implementorValueFromStringCallback, setImplementorValueFromStringCallback)
AU_HOOK_ACCESSORS(AUImplementorDisplayNameWithLengthCallback, implementorDisplayNameWithLengthCallback, _implementorDisplayNameWithLengthCallback, setImplementorDisplayNameWithLengthCallback)

- (NSString *) description {
	return [NSString stringWithFormat: @"<%@: %p, \"%@\", \"%@\">", [self class], self, _identifier, _displayName];
}

@end

@implementation AUParameterGroup

+ (BOOL) supportsSecureCoding {
	return YES;
}

- (instancetype) initWithIdentifier: (NSString *) identifier displayName: (NSString *) displayName children: (NSArray *) children {
	self = [super initWithIdentifier: identifier displayName: displayName];
	if (self) {
		NSMutableSet *seen = [NSMutableSet set];

		for (AUParameterNode *child in children) {
			if ([seen containsObject: [child identifier]]) {
				NSString *reason = [NSString stringWithFormat: @"Multiple nodes with identifier '%@'", [child identifier]];

				[self release];
				[NSException raise: AUInvalidNodeInfoException format: @"%@", reason];
			}
			[seen addObject: [child identifier]];
		}
		_children = [children copy];
		for (AUParameterNode *child in _children)
			[child setParentNode: self];
	}
	return self;
}

- (instancetype) initWithCoder: (NSCoder *) coder {
	NSSet *classes = [NSSet setWithObjects: [NSArray class], [AUParameter class], [AUParameterGroup class], nil];
	NSArray *children = [coder decodeObjectOfClasses: classes forKey: @"children"];

	self = [super initWithCoder: coder];
	if (self) {
		NSMutableArray *checked = [NSMutableArray array];

		for (id child in children) {
			if ([child isKindOfClass: [AUParameterNode class]])
				[checked addObject: child];
		}
		_children = [checked copy];
		for (AUParameterNode *child in _children)
			[child setParentNode: self];
	}
	return self;
}

- (void) encodeWithCoder: (NSCoder *) coder {
	[super encodeWithCoder: coder];
	[coder encodeObject: _children forKey: @"children"];
}

- (void) dealloc {
	for (AUParameterNode *child in _children) {
		if ([child parentNode] == self)
			[child setParentNode: nil];
	}
	[_children release];
	[super dealloc];
}

- (NSArray *) children {
	return _children;
}

- (void) collectParametersInto: (NSMutableArray *) parameters {
	for (AUParameterNode *child in _children) {
		if ([child isKindOfClass: [AUParameterGroup class]])
			[(AUParameterGroup *) child collectParametersInto: parameters];
		else
			[parameters addObject: child];
	}
}

- (NSArray *) allParameters {
	NSMutableArray *parameters = [NSMutableArray array];

	[self collectParametersInto: parameters];
	return parameters;
}

// Key-value coding by child identifier; any other key finds nothing.
- (id) valueForKey: (NSString *) key {
	for (AUParameterNode *child in _children) {
		if ([[child identifier] isEqualToString: key])
			return child;
	}
	return nil;
}

- (AUParameterGroup *) copyWithAddressOffset: (AUParameterAddress) offset identifier: (NSString *) identifier displayName: (NSString *) displayName {
	NSMutableArray *children = [NSMutableArray array];

	for (AUParameterNode *child in _children) {
		if ([child isKindOfClass: [AUParameterGroup class]]) {
			AUParameterGroup *group = (AUParameterGroup *) child;

			[children addObject: [[group copyWithAddressOffset: offset identifier: [group identifier] displayName: [group displayName]] autorelease]];
		} else {
			[children addObject: [[(AUParameter *) child copyWithAddressOffset: offset] autorelease]];
		}
	}
	return [[AUParameterGroup alloc] initWithIdentifier: identifier displayName: displayName children: children];
}

@end

@implementation AUParameterTree

// Restated next to this class's own -initWithCoder:; a secure unarchiver refuses a class whose
// decoding initializer comes from below its +supportsSecureCoding.
+ (BOOL) supportsSecureCoding {
	return YES;
}

- (instancetype) initWithChildren: (NSArray *) children {
	self = [super initWithIdentifier: @"" displayName: @"" children: children];
	if (self && ![self indexParameters]) {
		[self release];
		return nil;
	}
	return self;
}

// Builds the address index; a repeated address is an error in the tree's description.
- (BOOL) indexParameters {
	NSMutableDictionary *index = [[NSMutableDictionary alloc] init];

	for (AUParameter *parameter in [self allParameters]) {
		NSNumber *key = [NSNumber numberWithUnsignedLongLong: [parameter address]];

		if ([index objectForKey: key]) {
			[index release];
			[NSException raise: AUInvalidNodeInfoException format: @"2 or more parameters share the same address: 0x%llx", (unsigned long long) [parameter address]];
		}
		[index setObject: parameter forKey: key];
	}
	_parametersByAddress = index;
	return YES;
}

- (instancetype) initWithCoder: (NSCoder *) coder {
	self = [super initWithCoder: coder];
	if (self && ![self indexParameters]) {
		[self release];
		return nil;
	}
	return self;
}

- (void) dealloc {
	[_parametersByAddress release];
	[super dealloc];
}

- (AUParameter *) parameterWithAddress: (AUParameterAddress) address {
	return [_parametersByAddress objectForKey: [NSNumber numberWithUnsignedLongLong: address]];
}

- (AUParameter *) parameterWithID: (AudioUnitParameterID) paramID scope: (AudioUnitScope) scope element: (AudioUnitElement) element {
	// Only trees bridged from a version 2 audio unit know such identifications.
	return nil;
}

@end

@implementation AUParameter

+ (BOOL) supportsSecureCoding {
	return YES;
}

- (instancetype) initWithIdentifier: (NSString *) identifier name: (NSString *) name address: (AUParameterAddress) address min: (AUValue) min max: (AUValue) max unit: (AudioUnitParameterUnit) unit unitName: (NSString *) unitName flags: (AudioUnitParameterOptions) flags valueStrings: (NSArray *) valueStrings dependentParameters: (NSArray *) dependentParameters {
	self = [super initWithIdentifier: identifier displayName: name];
	if (self) {
		_address = address;
		_minValue = min;
		_maxValue = max;
		_unit = unit;
		_unitName = [unitName copy];
		_flags = flags;
		_valueStrings = [valueStrings copy];
		_dependentParameters = [dependentParameters copy];
	}
	return self;
}

- (instancetype) initWithCoder: (NSCoder *) coder {
	self = [super initWithCoder: coder];
	if (self) {
		NSSet *stringArray = [NSSet setWithObjects: [NSArray class], [NSString class], nil];
		NSSet *numberArray = [NSSet setWithObjects: [NSArray class], [NSNumber class], nil];

		_minValue = [coder decodeFloatForKey: @"minValue"];
		_maxValue = [coder decodeFloatForKey: @"maxValue"];
		_unit = (AudioUnitParameterUnit) [coder decodeInt32ForKey: @"unit"];
		_unitName = [[coder decodeObjectOfClass: [NSString class] forKey: @"unitName"] copy];
		_flags = (AudioUnitParameterOptions) [coder decodeInt32ForKey: @"flags"];
		_address = (AUParameterAddress) [coder decodeInt64ForKey: @"address"];
		_valueStrings = [[coder decodeObjectOfClasses: stringArray forKey: @"valueStrings"] copy];
		_dependentParameters = [[coder decodeObjectOfClasses: numberArray forKey: @"dependentParameters"] copy];
		_value = [coder decodeFloatForKey: @"value"];
	}
	return self;
}

- (void) encodeWithCoder: (NSCoder *) coder {
	[super encodeWithCoder: coder];
	[coder encodeFloat: _minValue forKey: @"minValue"];
	[coder encodeFloat: _maxValue forKey: @"maxValue"];
	[coder encodeInt32: (int32_t) _unit forKey: @"unit"];
	[coder encodeObject: _unitName forKey: @"unitName"];
	[coder encodeInt32: (int32_t) _flags forKey: @"flags"];
	[coder encodeInt64: (int64_t) _address forKey: @"address"];
	[coder encodeObject: _valueStrings forKey: @"valueStrings"];
	[coder encodeObject: _dependentParameters forKey: @"dependentParameters"];
	[coder encodeFloat: [self value] forKey: @"value"];
}

- (void) dealloc {
	[_unitName release];
	[_valueStrings release];
	[_dependentParameters release];
	[super dealloc];
}

- (AUParameter *) copyWithAddressOffset: (AUParameterAddress) offset {
	AUParameter *copy = [[AUParameter alloc] initWithIdentifier: _identifier name: _displayName address: _address + offset min: _minValue max: _maxValue unit: _unit unitName: _unitName flags: _flags valueStrings: _valueStrings dependentParameters: _dependentParameters];

	copy->_value = _value;
	return copy;
}

- (AUValue) minValue {
	return _minValue;
}

- (AUValue) maxValue {
	return _maxValue;
}

- (AudioUnitParameterUnit) unit {
	return _unit;
}

- (NSString *) unitName {
	return _unitName;
}

- (AudioUnitParameterOptions) flags {
	return _flags;
}

- (AUParameterAddress) address {
	return _address;
}

- (NSArray *) valueStrings {
	return _valueStrings;
}

- (NSArray *) dependentParameters {
	return _dependentParameters;
}

// A value stored by a setter is refreshed from the audio unit's provider the next time it is
// needed; with no provider the stored value stands and stays marked for a provider installed later.
- (AUValue) value {
	BOOL stale;
	AUValue stored;

	@synchronized (self) {
		stale = _stale;
		stored = _value;
	}
	if (stale) {
		AUImplementorValueProvider provider = [self nearestHook: @selector(implementorValueProvider)];

		if (provider) {
			stored = provider(self);
			@synchronized (self) {
				_value = stored;
				_stale = NO;
			}
		}
	}
	return stored;
}

- (void) setValue: (AUValue) value {
	[self setValue: value originator: NULL];
}

- (void) setValue: (AUValue) value originator: (AUParameterObserverToken) originator {
	[self setValue: value originator: originator atHostTime: 0];
}

- (void) setValue: (AUValue) value originator: (AUParameterObserverToken) originator atHostTime: (uint64_t) hostTime {
	[self setValue: value originator: originator atHostTime: hostTime eventType: AUParameterAutomationEventTypeValue];
}

- (void) setValue: (AUValue) value originator: (AUParameterObserverToken) originator atHostTime: (uint64_t) hostTime eventType: (AUParameterAutomationEventType) eventType {
	AUImplementorValueObserver implementorObserver;
	AUImplementorValueProvider provider;
	NSMutableArray *records;

	if (hostTime == 0)
		hostTime = mach_absolute_time();

	// The value is not limited to the parameter's range.
	@synchronized (self) {
		_value = value;
		_stale = YES;
	}

	implementorObserver = [self nearestHook: @selector(implementorValueObserver)];
	if (implementorObserver)
		implementorObserver(self, value);

	provider = [self nearestHook: @selector(implementorValueProvider)];
	if (provider) {
		AUValue provided = provider(self);

		@synchronized (self) {
			_value = provided;
			_stale = NO;
		}
	}

	records = [NSMutableArray array];
	[self collectObserversInto: records excluding: originator];
	if ([records count] == 0)
		return;

	AUParameterAddress address = _address;
	NSArray *delivery = [records copy];

	dispatch_async(AUParameterNotificationQueue(), ^{
		for (AUParameterObserverRecord *record in delivery) {
			switch (record->kind) {
				case AUObserverKindPlain:
					((AUParameterObserver) record->block)(address, value);
					break;
				case AUObserverKindRecording:
					{
						// The recorded form has no event type; gesture markers arrive as events too.
						AURecordedParameterEvent event = { hostTime, address, value };

						((AUParameterRecordingObserver) record->block)(1, &event);
					}
					break;
				case AUObserverKindAutomation: {
					AUParameterAutomationEvent event = { hostTime, address, value, eventType, 0 };

					((AUParameterAutomationObserver) record->block)(1, &event);
					break;
				}
			}
		}
		[delivery release];
	});
}

- (NSString *) stringFromValue: (const AUValue *) value {
	AUImplementorStringFromValueCallback callback = [self nearestHook: @selector(implementorStringFromValueCallback)];

	if (callback)
		return callback(self, value);
	return [NSString stringWithFormat: @"%g", (double) (value ? *value : [self value])];
}

- (AUValue) valueFromString: (NSString *) string {
	AUImplementorValueFromStringCallback callback = [self nearestHook: @selector(implementorValueFromStringCallback)];

	if (callback)
		return callback(self, string);
	return 0;
}

- (NSString *) description {
	return [NSString stringWithFormat: @"<%@: %p, \"%@\", \"%@\", addr 0x%llx, val %.3f>", [self class], self, _identifier, _displayName, (unsigned long long) _address, (double) [self value]];
}

@end

@implementation AUParameterTree (Factory)

+ (AUParameter *) createParameterWithIdentifier: (NSString *) identifier name: (NSString *) name address: (AUParameterAddress) address min: (AUValue) min max: (AUValue) max unit: (AudioUnitParameterUnit) unit unitName: (NSString *) unitName flags: (AudioUnitParameterOptions) flags valueStrings: (NSArray *) valueStrings dependentParameters: (NSArray *) dependentParameters {
	return [[[AUParameter alloc] initWithIdentifier: identifier name: name address: address min: min max: max unit: unit unitName: unitName flags: flags valueStrings: valueStrings dependentParameters: dependentParameters] autorelease];
}

+ (AUParameterGroup *) createGroupWithIdentifier: (NSString *) identifier name: (NSString *) name children: (NSArray *) children {
	return [[[AUParameterGroup alloc] initWithIdentifier: identifier displayName: name children: children] autorelease];
}

+ (AUParameterGroup *) createGroupTemplate: (NSArray *) children {
	return [[[AUParameterGroup alloc] initWithIdentifier: @"" displayName: @"" children: children] autorelease];
}

+ (AUParameterGroup *) createGroupFromTemplate: (AUParameterGroup *) templateGroup identifier: (NSString *) identifier name: (NSString *) name addressOffset: (AUParameterAddress) addressOffset {
	return [[templateGroup copyWithAddressOffset: addressOffset identifier: identifier displayName: name] autorelease];
}

+ (AUParameterTree *) createTreeWithChildren: (NSArray *) children {
	return [[[AUParameterTree alloc] initWithChildren: children] autorelease];
}

@end

#endif // __OBJC2__
