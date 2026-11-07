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

@implementation ODNode

+ (instancetype)nodeWithSession:(ODSession *)inSession type:(ODNodeType)inType error:(NSError **)outError
{
	return [[[self alloc] initWithSession: inSession type: inType error: outError] autorelease];
}

+ (instancetype)nodeWithSession:(ODSession *)inSession name:(NSString *)inName error:(NSError **)outError
{
	return [[[self alloc] initWithSession: inSession name: inName error: outError] autorelease];
}

- (instancetype)_initWithNodeRef:(ODNodeRef)node error:(CFErrorRef)error outError:(NSError **)outError
{
	ODSetError(outError, error);
	if (node == NULL)
	{
		[self release];
		return nil;
	}

	if ((self = [super init]) != nil)
		_internal = (void *) node;
	else
		CFRelease(node);
	return self;
}

- (instancetype)initWithSession:(ODSession *)inSession type:(ODNodeType)inType error:(NSError **)outError
{
	CFErrorRef error = NULL;
	ODNodeRef node = ODNodeCreateWithNodeType(kCFAllocatorDefault, [inSession _sessionRef], inType, &error);
	return [self _initWithNodeRef: node error: error outError: outError];
}

- (instancetype)initWithSession:(ODSession *)inSession name:(NSString *)inName error:(NSError **)outError
{
	CFErrorRef error = NULL;
	ODNodeRef node = ODNodeCreateWithName(kCFAllocatorDefault, [inSession _sessionRef], (CFStringRef) inName, &error);
	return [self _initWithNodeRef: node error: error outError: outError];
}

- (void)dealloc
{
	if (_internal != NULL)
		CFRelease((ODNodeRef) _internal);
	[super dealloc];
}

- (ODNodeRef)_nodeRef
{
	return (ODNodeRef) _internal;
}

- (NSString *)nodeName
{
	return [[(NSString *) ODNodeGetName((ODNodeRef) _internal) copy] autorelease];
}

- (NSArray *)subnodeNamesAndReturnError:(NSError **)outError
{
	CFErrorRef error = NULL;
	CFArrayRef names = ODNodeCopySubnodeNames((ODNodeRef) _internal, &error);

	ODSetError(outError, error);
	return [(NSArray *) names autorelease];
}

- (NSArray *)unreachableSubnodeNamesAndReturnError:(NSError **)outError
{
	CFErrorRef error = NULL;
	CFArrayRef names = ODNodeCopyUnreachableSubnodeNames((ODNodeRef) _internal, &error);

	ODSetError(outError, error);
	return [(NSArray *) names autorelease];
}

- (NSArray *)supportedRecordTypesAndReturnError:(NSError **)outError
{
	CFErrorRef error = NULL;
	CFArrayRef types = ODNodeCopySupportedRecordTypes((ODNodeRef) _internal, &error);

	ODSetError(outError, error);
	return [(NSArray *) types autorelease];
}

- (ODRecord *)recordWithRecordType:(ODRecordType)inRecordType name:(NSString *)inRecordName attributes:(id)inAttributes
	error:(NSError **)outError
{
	CFErrorRef error = NULL;
	ODRecordRef record = ODNodeCopyRecord((ODNodeRef) _internal, (ODRecordType) inRecordType,
		(CFStringRef) inRecordName, (CFTypeRef) inAttributes, &error);
	ODRecord *result = record ? [ODRecord _recordWithRecordRef: record] : nil;

	if (record != NULL)
		CFRelease(record);
	ODSetError(outError, error);
	return result;
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
