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

// The implementation is 64-bit only, like GameController on macOS (its 32-bit
// Objective-C ABI cannot hold instance variables declared in the @implementation
// and the framework is x86_64 and arm64 only there); the 32-bit slice keeps the stub.
#ifdef __LP64__
#import <GameController/GCControllerElement.h>
#import <GameController/GCController.h>
#import "GCInputInternal.h"

@implementation GCControllerElement {
	GCControllerElement *_collection;
	GCPhysicalInputProfile *_profile;
	BOOL _analog;
	BOOL _boundToSystemGesture;
	GCSystemGestureState _preferredSystemGestureState;
	NSSet<NSString *> *_aliases;
	NSString *_sfSymbolsName;
	NSString *_localizedName;
	NSString *_unmappedSfSymbolsName;
	NSString *_unmappedLocalizedName;
}

- (instancetype)init
{
	return [self initWithCollection:nil isAnalog:YES aliases:[NSSet set]];
}

- (instancetype)initWithCollection:(GCControllerElement *)collection isAnalog:(BOOL)analog aliases:(NSSet<NSString *> *)aliases
{
	self = [super init];
	if (!self)
		return nil;
	_collection = collection; // un-retained weak reference
	_analog = analog;
	_aliases = [aliases copy];
	return self;
}

- (void)dealloc
{
	[_aliases release];
	[_sfSymbolsName release];
	[_localizedName release];
	[_unmappedSfSymbolsName release];
	[_unmappedLocalizedName release];
	[super dealloc];
}

- (GCControllerElement *)collection
{
	return _collection;
}

- (BOOL)isAnalog
{
	return _analog;
}

- (NSSet<NSString *> *)aliases
{
	return _aliases ? _aliases : [NSSet set];
}

- (BOOL)isBoundToSystemGesture
{
	return _boundToSystemGesture;
}

- (GCSystemGestureState)preferredSystemGestureState
{
	return _preferredSystemGestureState;
}

- (void)setPreferredSystemGestureState:(GCSystemGestureState)state
{
	_preferredSystemGestureState = state;
}

// The four names are independent: setting one does not change the others.
- (NSString *)sfSymbolsName { return _sfSymbolsName; }
- (NSString *)localizedName { return _localizedName; }
- (NSString *)unmappedSfSymbolsName { return _unmappedSfSymbolsName; }
- (NSString *)unmappedLocalizedName { return _unmappedLocalizedName; }

- (void)setSfSymbolsName:(NSString *)name
{
	if (name != _sfSymbolsName) {
		[_sfSymbolsName release];
		_sfSymbolsName = [name retain];
	}
}

- (void)setLocalizedName:(NSString *)name
{
	if (name != _localizedName) {
		[_localizedName release];
		_localizedName = [name retain];
	}
}

- (void)setUnmappedSfSymbolsName:(NSString *)name
{
	if (name != _unmappedSfSymbolsName) {
		[_unmappedSfSymbolsName release];
		_unmappedSfSymbolsName = [name retain];
	}
}

- (void)setUnmappedLocalizedName:(NSString *)name
{
	if (name != _unmappedLocalizedName) {
		[_unmappedLocalizedName release];
		_unmappedLocalizedName = [name retain];
	}
}

- (void)_setAnalog:(BOOL)analog
{
	_analog = analog;
}

- (void)_setAliases:(NSSet<NSString *> *)aliases
{
	if (aliases != _aliases) {
		[_aliases release];
		_aliases = [aliases copy];
	}
}

- (void)_setBoundToSystemGesture:(BOOL)bound
{
	_boundToSystemGesture = bound;
}

- (void)_setProfile:(GCPhysicalInputProfile *)profile
{
	_profile = profile; // un-retained: the profile owns its elements
}

- (GCPhysicalInputProfile *)_profile
{
	return _profile ? _profile : [_collection _profile];
}

// The public setters of the values (-setValue: and friends) write only to elements of a profile that
// belongs to a controller created by the application, or to no device at all; the elements of a real
// device are updated by the device alone.
- (BOOL)_allowsPublicWrites
{
	id<GCDevice> device = [[self _profile] device];
	if (!device)
		return YES;
	if ([device isKindOfClass:[GCController class]])
		return [(GCController *)device isSnapshot];
	return NO;
}

- (dispatch_queue_t)_handlerQueue
{
	GCPhysicalInputProfile *profile = [self _profile];
	return profile ? [profile _handlerQueue] : dispatch_get_main_queue();
}

- (void)_copyStateFromElement:(GCControllerElement *)element
{
}

@end
#else
#import <GameController/GCControllerElement.h>
#import <darling-stub-forwarding.h>

@implementation GCControllerElement

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
