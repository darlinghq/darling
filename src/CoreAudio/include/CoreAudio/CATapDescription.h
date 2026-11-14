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

#ifndef _CATAPDESCRIPTION_H_
#define _CATAPDESCRIPTION_H_

#ifdef __OBJC__

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// What happens to the audio of a tapped process.
typedef NS_ENUM(NSInteger, CATapMuteBehavior) {
	CATapUnmuted = 0,           // the process stays audible
	CATapMuted = 1,             // captured by the tap, not sent to the hardware
	CATapMutedWhenTapped = 2    // audible, but muted while another client reads the tap
};

// Describes a tap: a stream that mixes the output audio of a set of processes.
@interface CATapDescription : NSObject {
@private
	NSString *_name;
	NSUUID *_UUID;
	NSArray *_processes;
	NSArray *_bundleIDs;
	BOOL _mono;
	BOOL _exclusive;
	BOOL _mixdown;
	BOOL _privateTap;
	BOOL _processRestoreEnabled;
	CATapMuteBehavior _muteBehavior;
	NSString *_deviceUID;
	NSNumber *_stream;
}

- (instancetype)init;
- (instancetype)initStereoMixdownOfProcesses:(NSArray<NSNumber*>*)processesObjectIDsToIncludeInTap;
- (instancetype)initStereoGlobalTapButExcludeProcesses:(NSArray<NSNumber*>*)processesObjectIDsToExcludeFromTap;
- (instancetype)initMonoMixdownOfProcesses:(NSArray<NSNumber*>*)processesObjectIDsToIncludeInTap;
- (instancetype)initMonoGlobalTapButExcludeProcesses:(NSArray<NSNumber*>*)processesObjectIDsToExcludeFromTap;
- (instancetype)initWithProcesses:(NSArray<NSNumber*>*)processesObjectIDsToIncludeInTap andDeviceUID:(NSString*)deviceUID withStream:(NSInteger)stream;
- (instancetype)initExcludingProcesses:(NSArray<NSNumber*>*)processesObjectIDsToExcludeFromTap andDeviceUID:(NSString*)deviceUID withStream:(NSInteger)stream;

@property (atomic, copy) NSString* name;
@property (atomic, copy) NSUUID* UUID;
// AudioObjectIDs of the process objects to tap or exclude.
@property (atomic, copy, readwrite) NSArray<NSNumber*>* processes;
// Bundle IDs of the processes to tap or exclude.
@property (atomic, copy, readwrite) NSArray<NSString*>* bundleIDs;
@property (atomic, readwrite, getter=isMono) BOOL mono;
// True if the tap includes all processes except the listed ones.
@property (atomic, readwrite, getter=isExclusive) BOOL exclusive;
@property (atomic, readwrite, getter=isMixdown) BOOL mixdown;
// True if only the process that created the tap can see it.
@property (atomic, readwrite, getter=isPrivate, setter=setPrivate:) BOOL privateTap;
@property (atomic, readwrite, getter=isProcessRestoreEnabled) BOOL processRestoreEnabled;
@property (atomic, readwrite, getter=isMuted) CATapMuteBehavior muteBehavior;
// Set if the tap captures a specific hardware device.
@property (atomic, copy, nullable) NSString* deviceUID;
// Set if the tap captures a specific stream of the device.
@property (atomic, copy, nullable) NSNumber* stream;

@end

NS_ASSUME_NONNULL_END

#endif // __OBJC__

#endif
