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

// The component accessors describe the planes the surface was created with: the
// kIOSurfacePlaneComponent* arrays of each kIOSurfacePlaneInfo entry and the
// kIOSurfaceSubsampling creation property. A surface created without them has no
// components and no subsampling (the unknown values); a value set on the surface
// afterwards does not change them, and a surface that another process created does
// not carry them here.

#import <IOSurface/IOSurface.h>
#import <IOSurface/IOSurfaceObjC.h>
#include <CoreFoundation/CoreFoundation.h>
#include <limits.h>

@interface IOSurface (ComponentInfo)
- (nullable id)_copyComponentValueForKey:(NSString *)key plane:(NSInteger)plane;
@end

// The array of components of one kind for a plane, or nil.
static NSArray *CopyComponentArray(IOSurfaceRef buffer, CFStringRef key, size_t planeIndex)
{
	if (buffer == NULL || planeIndex > (size_t) NSIntegerMax)
		return nil;
	id value = [(IOSurface *) buffer _copyComponentValueForKey: (NSString *) key plane: (NSInteger) planeIndex];
	if (![value isKindOfClass: [NSArray class]])
	{
		[value release];
		return nil;
	}
	return value;
}

// The five kinds of per-component arrays of a plane-info entry.
static void ComponentKeys(CFStringRef keys[5])
{
	keys[0] = kIOSurfacePlaneComponentNames;
	keys[1] = kIOSurfacePlaneComponentTypes;
	keys[2] = kIOSurfacePlaneComponentRanges;
	keys[3] = kIOSurfacePlaneComponentBitDepths;
	keys[4] = kIOSurfacePlaneComponentBitOffsets;
}

// One component value, or 0 when the plane has no such array or component.
static int64_t ComponentValue(IOSurfaceRef buffer, CFStringRef key, size_t planeIndex, size_t componentIndex)
{
	NSArray *array = CopyComponentArray(buffer, key, planeIndex);
	int64_t result = 0;
	if (componentIndex < array.count)
	{
		id element = array[componentIndex];
		if ([element isKindOfClass: [NSNumber class]])
			result = [element longLongValue];
	}
	[array release];
	return result;
}

size_t IOSurfaceGetNumberOfComponentsOfPlane(IOSurfaceRef buffer, size_t planeIndex)
{
	CFStringRef keys[5];
	ComponentKeys(keys);
	size_t count = 0;
	for (size_t i = 0; i < 5; i++)
	{
		NSArray *array = CopyComponentArray(buffer, keys[i], planeIndex);
		if (array.count > count)
			count = array.count;
		[array release];
	}
	return count;
}

IOSurfaceComponentName IOSurfaceGetNameOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex,
                                                          size_t componentIndex)
{
	return (IOSurfaceComponentName) ComponentValue(buffer, kIOSurfacePlaneComponentNames, planeIndex, componentIndex);
}

IOSurfaceComponentType IOSurfaceGetTypeOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex,
                                                         size_t componentIndex)
{
	return (IOSurfaceComponentType) ComponentValue(buffer, kIOSurfacePlaneComponentTypes, planeIndex, componentIndex);
}

IOSurfaceComponentRange IOSurfaceGetRangeOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex,
                                                           size_t componentIndex)
{
	return (IOSurfaceComponentRange) ComponentValue(buffer, kIOSurfacePlaneComponentRanges, planeIndex, componentIndex);
}

size_t IOSurfaceGetBitDepthOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex, size_t componentIndex)
{
	int64_t value = ComponentValue(buffer, kIOSurfacePlaneComponentBitDepths, planeIndex, componentIndex);
	return value > 0 ? (size_t) value : 0;
}

size_t IOSurfaceGetBitOffsetOfComponentOfPlane(IOSurfaceRef buffer, size_t planeIndex, size_t componentIndex)
{
	int64_t value = ComponentValue(buffer, kIOSurfacePlaneComponentBitOffsets, planeIndex, componentIndex);
	return value > 0 ? (size_t) value : 0;
}

IOSurfaceSubsampling IOSurfaceGetSubsampling(IOSurfaceRef buffer)
{
	if (buffer == NULL)
		return kIOSurfaceSubsamplingUnknown;
	id value = [(IOSurface *) buffer _copyComponentValueForKey: (NSString *) kIOSurfaceSubsampling plane: -1];
	IOSurfaceSubsampling result = kIOSurfaceSubsamplingUnknown;
	if ([value isKindOfClass: [NSNumber class]])
		result = (IOSurfaceSubsampling) [value intValue];
	[value release];
	return result;
}

// The value macOS gives for the size properties that have a limit: the largest
// 32-bit signed value. The properties without a limit answer 0.
size_t IOSurfaceGetPropertyMaximum(CFStringRef property)
{
	if (property == NULL)
		return 0;
	if (CFEqual(property, kIOSurfaceWidth) || CFEqual(property, kIOSurfaceHeight)
		|| CFEqual(property, kIOSurfaceBytesPerRow) || CFEqual(property, kIOSurfacePlaneWidth)
		|| CFEqual(property, kIOSurfacePlaneHeight) || CFEqual(property, kIOSurfacePlaneBytesPerRow))
		return INT32_MAX;
	return 0;
}
