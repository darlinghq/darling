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

#import <GameController/GCColor.h>

@implementation GCColor

@synthesize red = _red, green = _green, blue = _blue;

+ (BOOL)supportsSecureCoding
{
	return YES;
}

- (instancetype)initWithRed:(float)red green:(float)green blue:(float)blue
{
	if ((self = [super init])) {
		_red = red;
		_green = green;
		_blue = blue;
	}
	return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	return [self initWithRed:[coder decodeFloatForKey:@"red"]
	                   green:[coder decodeFloatForKey:@"green"]
	                    blue:[coder decodeFloatForKey:@"blue"]];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
	[coder encodeFloat:_red forKey:@"red"];
	[coder encodeFloat:_green forKey:@"green"];
	[coder encodeFloat:_blue forKey:@"blue"];
}

- (id)copyWithZone:(NSZone *)zone
{
	return [self retain];
}

@end
