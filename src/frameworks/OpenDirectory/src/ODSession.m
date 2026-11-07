/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#import "ODInternal.h"

NSString *const ODFrameworkErrorDomain = @"com.apple.OpenDirectory";

void ODSetError(NSError **outError, CFErrorRef error)
{
	if (outError != NULL)
		*outError = error ? [(NSError *) error autorelease] : nil;
	else if (error != NULL)
		CFRelease(error);
}

@implementation ODSession

+ (ODSession *)defaultSession
{
	static ODSession *shared;
	static dispatch_once_t once;

	dispatch_once(&once, ^{
		shared = [[ODSession alloc] initWithOptions: nil error: NULL];
	});
	return shared;
}

+ (instancetype)sessionWithOptions:(NSDictionary *)inOptions error:(NSError **)outError
{
	return [[[self alloc] initWithOptions: inOptions error: outError] autorelease];
}

- (instancetype)initWithOptions:(NSDictionary *)inOptions error:(NSError **)outError
{
	CFErrorRef error = NULL;
	ODSessionRef session = ODSessionCreate(kCFAllocatorDefault, (CFDictionaryRef) inOptions, &error);

	ODSetError(outError, error);
	if (session == NULL)
	{
		[self release];
		return nil;
	}

	if ((self = [super init]) != nil)
		_internal = (void *) session;
	else
		CFRelease(session);
	return self;
}

- (void)dealloc
{
	if (_internal != NULL)
		CFRelease((ODSessionRef) _internal);
	[super dealloc];
}

- (ODSessionRef)_sessionRef
{
	return (ODSessionRef) _internal;
}

- (NSArray *)nodeNamesAndReturnError:(NSError **)outError
{
	CFErrorRef error = NULL;
	CFArrayRef names = ODSessionCopyNodeNames(kCFAllocatorDefault, (ODSessionRef) _internal, &error);

	ODSetError(outError, error);
	return [(NSArray *) names autorelease];
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return [NSMethodSignature signatureWithObjCTypes: "v@:"];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
