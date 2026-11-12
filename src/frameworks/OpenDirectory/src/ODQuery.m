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
#import <darling-stub-forwarding.h>

@implementation ODQuery

+ (ODQuery *)queryWithNode:(ODNode *)inNode forRecordTypes:(id)inRecordTypeOrList attribute:(ODAttributeType)inAttribute
	matchType:(ODMatchType)inMatchType queryValues:(id)inQueryValueOrList returnAttributes:(id)inReturnAttributeOrList
	maximumResults:(NSInteger)inMaximumResults error:(NSError **)outError
{
	return [[[self alloc] initWithNode: inNode forRecordTypes: inRecordTypeOrList attribute: inAttribute
		matchType: inMatchType queryValues: inQueryValueOrList returnAttributes: inReturnAttributeOrList
		maximumResults: inMaximumResults error: outError] autorelease];
}

- (instancetype)initWithNode:(ODNode *)inNode forRecordTypes:(id)inRecordTypeOrList attribute:(ODAttributeType)inAttribute
	matchType:(ODMatchType)inMatchType queryValues:(id)inQueryValueOrList returnAttributes:(id)inReturnAttributeOrList
	maximumResults:(NSInteger)inMaximumResults error:(NSError **)outError
{
	CFErrorRef error = NULL;
	ODQueryRef query = ODQueryCreateWithNode(kCFAllocatorDefault, [inNode _nodeRef], (CFTypeRef) inRecordTypeOrList,
		(ODAttributeType) inAttribute, inMatchType, (CFTypeRef) inQueryValueOrList, (CFTypeRef) inReturnAttributeOrList,
		inMaximumResults, &error);

	ODSetError(outError, error);
	if (query == NULL)
	{
		[self release];
		return nil;
	}

	if ((self = [super init]) != nil)
		_internal = (void *) query;
	else
		CFRelease(query);
	return self;
}

- (void)dealloc
{
	if (_internal != NULL)
		CFRelease((ODQueryRef) _internal);
	[super dealloc];
}

- (id)copyWithZone:(NSZone *)zone
{
	// A query has no mutable state, so a copy can share the CF query.
	ODQuery *copy = [[ODQuery allocWithZone: zone] init];

	if (copy != nil && _internal != NULL)
		copy->_internal = (void *) CFRetain((ODQueryRef) _internal);
	return copy;
}

- (NSArray *)resultsAllowingPartial:(BOOL)inAllowPartialResults error:(NSError **)outError
{
	CFErrorRef error = NULL;
	CFArrayRef records = ODQueryCopyResults((ODQueryRef) _internal, inAllowPartialResults, &error);
	NSArray *result = records ? [ODRecord _recordsWithRecordRefs: records] : nil;

	if (records != NULL)
		CFRelease(records);
	ODSetError(outError, error);
	return result;
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
