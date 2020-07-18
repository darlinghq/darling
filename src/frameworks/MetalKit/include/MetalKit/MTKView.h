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

#if TARGET_OS_IPHONE
// #import <IOKit/IOKit.h>
// #define View UIView
#else // TARGET_OS_OSX
#import <AppKit/AppKit.h>
#define View NSView
#endif

#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>


@interface MTKView : View {
    id<MTLDevice> _device;
    MTLClearColor _clearColor;
}

@property(nonatomic, retain) id<MTLDevice> device;
@property(nonatomic) MTLClearColor clearColor;

@end
