/*
 * This file is part of Darling.
 *
 * Copyright (C) 2023 Darling Developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import <Foundation/Foundation.h>
#import <darling-stub-forwarding.h>

@interface NSAttributeDescription : NSObject
@end

@implementation NSAttributeDescription

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

@interface NSEntityDescription : NSObject
@end

@implementation NSEntityDescription

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

@interface NSEntityMapping : NSObject
@end

@implementation NSEntityMapping

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

@interface NSFetchRequest : NSObject
@end

@implementation NSFetchRequest

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

@interface NSFetchRequestExpression : NSObject
@end

@implementation NSFetchRequestExpression

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

@interface NSFetchedPropertyDescription : NSObject
@end

@implementation NSFetchedPropertyDescription

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

@interface NSManagedObject : NSObject
@end

@implementation NSManagedObject

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

@interface NSManagedObjectContext : NSObject
@end

@implementation NSManagedObjectContext

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

@interface NSManagedObjectID : NSObject
@end

@implementation NSManagedObjectID

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

@interface NSManagedObjectModel : NSObject
@end

@implementation NSManagedObjectModel

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

@interface NSMappingModel : NSObject
@end

@implementation NSMappingModel

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

@interface NSPersistentStoreCoordinator : NSObject
@end

@implementation NSPersistentStoreCoordinator

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

@interface NSPropertyMapping : NSObject
@end

@implementation NSPropertyMapping

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

@interface _NSPersistenceUtilities : NSObject
@end

@implementation _NSPersistenceUtilities

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

