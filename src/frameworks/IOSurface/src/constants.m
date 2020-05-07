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
#import <IOSurface/IOSurfaceRef.h>
#import <IOSurface/IOSurfaceObjC.h>

const CFStringRef kIOSurfaceAllocSize = CFSTR("IOSurfaceAllocSize");
const CFStringRef kIOSurfaceWidth = CFSTR("IOSurfaceWidth");
const CFStringRef kIOSurfaceHeight = CFSTR("IOSurfaceHeight");
const CFStringRef kIOSurfaceBytesPerRow = CFSTR("IOSurfaceBytesPerRow");
const CFStringRef kIOSurfaceBytesPerElement = CFSTR("IOSurfaceBytesPerElement");
const CFStringRef kIOSurfaceElementWidth = CFSTR("IOSurfaceElementWidth");
const CFStringRef kIOSurfaceElementHeight = CFSTR("IOSurfaceElementHeight");
const CFStringRef kIOSurfaceOffset = CFSTR("IOSurfaceOffset");
const CFStringRef kIOSurfacePlaneInfo = CFSTR("IOSurfacePlaneInfo");
const CFStringRef kIOSurfacePlaneWidth = CFSTR("IOSurfacePlaneWidth");
const CFStringRef kIOSurfacePlaneHeight = CFSTR("IOSurfacePlaneHeight");
const CFStringRef kIOSurfacePlaneBytesPerRow = CFSTR("IOSurfacePlaneBytesPerRow");
const CFStringRef kIOSurfacePlaneOffset = CFSTR("IOSurfacePlaneOffset");
const CFStringRef kIOSurfacePlaneSize = CFSTR("IOSurfacePlaneSize");
const CFStringRef kIOSurfacePlaneBase = CFSTR("IOSurfacePlaneBase");
const CFStringRef kIOSurfacePlaneBitsPerElement = CFSTR("IOSurfacePlaneBitsPerElement");
const CFStringRef kIOSurfacePlaneBytesPerElement = CFSTR("IOSurfacePlaneBytesPerElement");
const CFStringRef kIOSurfacePlaneElementWidth = CFSTR("IOSurfacePlaneElementWidth");
const CFStringRef kIOSurfacePlaneElementHeight = CFSTR("IOSurfacePlaneElementHeight");
const CFStringRef kIOSurfaceCacheMode = CFSTR("IOSurfaceCacheMode");
const CFStringRef kIOSurfaceIsGlobal = CFSTR("IOSurfaceIsGlobal");
const CFStringRef kIOSurfacePixelFormat = CFSTR("IOSurfacePixelFormat");
const CFStringRef kIOSurfacePixelSizeCastingAllowed = CFSTR("IOSurfacePixelSizeCastingAllowed");
const CFStringRef kIOSurfacePlaneComponentBitDepths = CFSTR("IOSurfacePlaneComponentBitDepths");
const CFStringRef kIOSurfacePlaneComponentBitOffsets = CFSTR("IOSurfacePlaneComponentBitOffsets");

IOSurfacePropertyKey IOSurfacePropertyKeyAllocSize = @"IOSurfacePropertyKeyAllocSize";
IOSurfacePropertyKey IOSurfacePropertyKeyWidth = @"IOSurfacePropertyKeyWidth";
IOSurfacePropertyKey IOSurfacePropertyKeyHeight = @"IOSurfacePropertyKeyHeight";
IOSurfacePropertyKey IOSurfacePropertyKeyBytesPerRow = @"IOSurfacePropertyKeyBytesPerRow";
IOSurfacePropertyKey IOSurfacePropertyKeyBytesPerElement = @"IOSurfacePropertyKeyBytesPerElement";
IOSurfacePropertyKey IOSurfacePropertyKeyElementWidth = @"IOSurfacePropertyKeyElementWidth";
IOSurfacePropertyKey IOSurfacePropertyKeyElementHeight = @"IOSurfacePropertyKeyElementHeight";
IOSurfacePropertyKey IOSurfacePropertyKeyOffset = @"IOSurfacePropertyKeyOffset";
IOSurfacePropertyKey IOSurfacePropertyKeyPlaneInfo = @"IOSurfacePropertyKeyPlaneInfo";
IOSurfacePropertyKey IOSurfacePropertyKeyPlaneWidth = @"IOSurfacePropertyKeyPlaneWidth";
IOSurfacePropertyKey IOSurfacePropertyKeyPlaneHeight = @"IOSurfacePropertyKeyPlaneHeight";
IOSurfacePropertyKey IOSurfacePropertyKeyPlaneBytesPerRow = @"IOSurfacePropertyKeyPlaneBytesPerRow";
IOSurfacePropertyKey IOSurfacePropertyKeyPlaneOffset = @"IOSurfacePropertyKeyPlaneOffset";
IOSurfacePropertyKey IOSurfacePropertyKeyPlaneSize = @"IOSurfacePropertyKeyPlaneSize";
IOSurfacePropertyKey IOSurfacePropertyKeyPlaneBase = @"IOSurfacePropertyKeyPlaneBase";
IOSurfacePropertyKey IOSurfacePropertyKeyPlaneBytesPerElement = @"IOSurfacePropertyKeyPlaneBytesPerElement";
IOSurfacePropertyKey IOSurfacePropertyKeyPlaneElementWidth = @"IOSurfacePropertyKeyPlaneElementWidth";
IOSurfacePropertyKey IOSurfacePropertyKeyPlaneElementHeight = @"IOSurfacePropertyKeyPlaneElementHeight";
IOSurfacePropertyKey IOSurfacePropertyKeyCacheMode = @"IOSurfacePropertyKeyCacheMode";
IOSurfacePropertyKey IOSurfacePropertyKeyPixelFormat = @"IOSurfacePropertyKeyPixelFormat";
IOSurfacePropertyKey IOSurfacePropertyKeyPixelSizeCastingAllowed = @"IOSurfacePropertyKeyPixelSizeCastingAllowed";

