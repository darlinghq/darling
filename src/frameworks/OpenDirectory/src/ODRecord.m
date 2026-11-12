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

// ODRecord declares no instance variables, so the records this framework
// hands out are of a private subclass that holds the CF record.
@interface _ODRecordWrapper : ODRecord
{
@public
	ODRecordRef _record;
}
@end

@implementation ODRecord

+ (ODRecord *)_recordWithRecordRef:(ODRecordRef)record
{
	_ODRecordWrapper *wrapper = [[_ODRecordWrapper alloc] init];

	if (wrapper != nil)
		wrapper->_record = (ODRecordRef) CFRetain(record);
	return [wrapper autorelease];
}

+ (NSArray *)_recordsWithRecordRefs:(CFArrayRef)records
{
	NSMutableArray *result = [NSMutableArray arrayWithCapacity: CFArrayGetCount(records)];

	for (CFIndex i = 0; i < CFArrayGetCount(records); i++)
	{
		ODRecord *record = [self _recordWithRecordRef: (ODRecordRef) CFArrayGetValueAtIndex(records, i)];
		if (record != nil)
			[result addObject: record];
	}
	return result;
}

- (ODRecordRef)_recordRef
{
	return NULL;
}

- (NSString *)recordType
{
	return [[(NSString *) ODRecordGetRecordType([self _recordRef]) copy] autorelease];
}

- (NSString *)recordName
{
	return [[(NSString *) ODRecordGetRecordName([self _recordRef]) copy] autorelease];
}

- (NSArray *)valuesForAttribute:(ODAttributeType)inAttribute error:(NSError **)outError
{
	CFErrorRef error = NULL;
	CFArrayRef values = ODRecordCopyValues([self _recordRef], (ODAttributeType) inAttribute, &error);

	ODSetError(outError, error);
	return [(NSArray *) values autorelease];
}

- (NSDictionary *)recordDetailsForAttributes:(NSArray *)inAttributes error:(NSError **)outError
{
	CFErrorRef error = NULL;
	CFDictionaryRef details = ODRecordCopyDetails([self _recordRef], (CFArrayRef) inAttributes, &error);

	ODSetError(outError, error);
	return [(NSDictionary *) details autorelease];
}

- (BOOL)verifyPassword:(NSString *)inPassword error:(NSError **)outError
{
	CFErrorRef error = NULL;
	bool ok = ODRecordVerifyPassword([self _recordRef], (CFStringRef) inPassword, &error);

	ODSetError(outError, error);
	return ok ? YES : NO;
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

@implementation _ODRecordWrapper

- (void)dealloc
{
	if (_record != NULL)
		CFRelease(_record);
	[super dealloc];
}

- (ODRecordRef)_recordRef
{
	return _record;
}

@end
