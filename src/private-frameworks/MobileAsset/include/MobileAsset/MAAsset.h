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

#include <Foundation/Foundation.h>

// this is wrong; i *know* this is wrong
// this doesn't even belong here
// this *should* be in `MobileKeyBag/MobileKeyBag.h`, but apparently
// `OTATrustUtilities.m` only imports that on iOS, but still expects this to be
// defined on macOS (i figure it's probably an error in Apple's code,
// because even `SecAKSWrappers.c` uses a different constant on macOS)
#define kMobileKeyBagLockStatusNotificationID (-1)

// seem to be implicitly depended on by `OTATrustUtilities.m` in Security
#import <MobileAsset/ASAsset.h>
#import <MobileAsset/MADownloadOptions.h>

// not quite sure if this is an enum, or if this even belongs here
typedef NS_ENUM(NSInteger, MAPurgeResult) {
	MAPurgeSucceeded = 0, // conventional value for success
};

// also not sure about this one
typedef NS_ENUM(NSInteger, MADownLoadResult) {
	MADownloadSuccessful = 0, // same here; success = 0
	MADownloadDaemonNotReady = -1, // errors are usually negative
};

// i'm not even sure if this is supposed to exist
typedef NS_ENUM(NSInteger, MAAssetState) {
	MAInstalled,
	MAUnknown,
	MADownloading,
	MANotPresent,

	MAInstalledOnly, // not sure if this one is like the others
};

@interface MAAsset : NSObject

@property () NSDictionary* attributes;
@property () MAAssetState state;

@end
