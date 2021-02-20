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

#include <Foundation/Foundation.h>

#define SFAccessPolicy _SFAccessPolicy

typedef NS_ENUM(NSInteger, SFAccessible) {
	SFAccessibleAfterFirstUnlock,
};

typedef NS_ENUM(NSInteger, SFSharingPolicy) {
	SFSharingPolicyThisDeviceOnly,
};

// not sure what `accessibility` is
// it's not a class, so i guess it's a structure?
typedef struct SFAccessibilityStructure {
	SFAccessible mode;
} SFAccessibilityStructure;

@interface SFAccessPolicy : NSObject

@property (nonatomic) SFAccessibilityStructure accessibility;
@property (nonatomic) SFSharingPolicy sharingPolicy;
@property (copy, nonatomic) NSString* accessGroup;

@end
