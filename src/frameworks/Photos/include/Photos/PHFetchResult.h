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

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// An ordered, immutable list of Photos objects returned by a fetch. The type
// parameter is the element class; Swift imports it as PHFetchResult<T>.
@interface PHFetchResult<__covariant ObjectType> : NSObject <NSCopying, NSFastEnumeration>

@property (readonly) NSUInteger count;

- (ObjectType)objectAtIndex:(NSUInteger)index;
- (ObjectType)objectAtIndexedSubscript:(NSUInteger)idx;
- (BOOL)containsObject:(ObjectType)anObject;
- (NSUInteger)indexOfObject:(ObjectType)anObject;
- (NSUInteger)indexOfObject:(ObjectType)anObject inRange:(NSRange)range;

@property (nonatomic, readonly, nullable) ObjectType firstObject;
@property (nonatomic, readonly, nullable) ObjectType lastObject;

- (NSArray<ObjectType> *)objectsAtIndexes:(NSIndexSet *)indexes;

- (void)enumerateObjectsUsingBlock:(void (NS_NOESCAPE ^)(ObjectType obj, NSUInteger idx, BOOL *stop))block;
- (void)enumerateObjectsWithOptions:(NSEnumerationOptions)opts
                         usingBlock:(void (NS_NOESCAPE ^)(ObjectType obj, NSUInteger idx, BOOL *stop))block;
- (void)enumerateObjectsAtIndexes:(NSIndexSet *)s
                          options:(NSEnumerationOptions)opts
                       usingBlock:(void (NS_NOESCAPE ^)(ObjectType obj, NSUInteger idx, BOOL *stop))block;

@end

NS_ASSUME_NONNULL_END
