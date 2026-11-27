/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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


#import <ProtocolBuffer/PBCodable.h>
#import <darling-stub-forwarding.h>

@implementation PBCodable

- (id)initWithData:(NSData*)data
{
	self = [super init];
	if (self != nil) {
		PBDataReader* reader = [[PBDataReader alloc] initWithData: data];
		BOOL ok = [self readFrom: reader];
		[reader release];
		if (!ok) {
			[self release];
			return nil;
		}
	}
	return self;
}

- (BOOL)readFrom:(PBDataReader*)reader
{
	return YES;
}

- (void)writeTo:(PBDataWriter*)writer
{
}

- (NSData*)data
{
	PBDataWriter* writer = [[PBDataWriter alloc] init];
	[self writeTo: writer];
	NSData* data = [[writer data] copy];
	[writer release];
	return [data autorelease];
}

- (NSDictionary*)dictionaryRepresentation
{
	return [NSDictionary dictionary];
}

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
