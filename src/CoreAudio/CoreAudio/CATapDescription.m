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

// The description of a process tap. Nothing here creates a tap: this is the value object that
// clients fill in and hand to the tapping calls.

#import <CoreAudio/CATapDescription.h>

@implementation CATapDescription

@synthesize name = _name;
@synthesize UUID = _UUID;
@synthesize processes = _processes;
@synthesize bundleIDs = _bundleIDs;
@synthesize mono = _mono;
@synthesize exclusive = _exclusive;
@synthesize mixdown = _mixdown;
@synthesize privateTap = _privateTap;
@synthesize processRestoreEnabled = _processRestoreEnabled;
@synthesize muteBehavior = _muteBehavior;
@synthesize deviceUID = _deviceUID;
@synthesize stream = _stream;

// All variants start from the same state: a new UUID, no processes, private.
- (instancetype)init
{
	self = [super init];
	if (self != nil)
	{
		_UUID = [[NSUUID alloc] init];
		_processes = [@[] copy];
		_bundleIDs = [@[] copy];
		_privateTap = YES;
	}
	return self;
}

- (instancetype)_initWithProcesses:(NSArray<NSNumber*>*)processes mono:(BOOL)mono exclusive:(BOOL)exclusive mixdown:(BOOL)mixdown
{
	self = [self init];
	if (self != nil)
	{
		self.processes = processes;
		_mono = mono;
		_exclusive = exclusive;
		_mixdown = mixdown;
	}
	return self;
}

- (instancetype)initStereoMixdownOfProcesses:(NSArray<NSNumber*>*)processes
{
	return [self _initWithProcesses:processes mono:NO exclusive:NO mixdown:YES];
}

- (instancetype)initStereoGlobalTapButExcludeProcesses:(NSArray<NSNumber*>*)processes
{
	return [self _initWithProcesses:processes mono:NO exclusive:YES mixdown:YES];
}

- (instancetype)initMonoMixdownOfProcesses:(NSArray<NSNumber*>*)processes
{
	return [self _initWithProcesses:processes mono:YES exclusive:NO mixdown:YES];
}

- (instancetype)initMonoGlobalTapButExcludeProcesses:(NSArray<NSNumber*>*)processes
{
	return [self _initWithProcesses:processes mono:YES exclusive:YES mixdown:YES];
}

- (instancetype)initWithProcesses:(NSArray<NSNumber*>*)processes andDeviceUID:(NSString*)deviceUID withStream:(NSInteger)stream
{
	self = [self _initWithProcesses:processes mono:NO exclusive:NO mixdown:NO];
	if (self != nil)
	{
		self.deviceUID = deviceUID;
		self.stream = @(stream);
	}
	return self;
}

- (instancetype)initExcludingProcesses:(NSArray<NSNumber*>*)processes andDeviceUID:(NSString*)deviceUID withStream:(NSInteger)stream
{
	self = [self _initWithProcesses:processes mono:NO exclusive:YES mixdown:NO];
	if (self != nil)
	{
		self.deviceUID = deviceUID;
		self.stream = @(stream);
	}
	return self;
}

- (void)dealloc
{
	[_name release];
	[_UUID release];
	[_processes release];
	[_bundleIDs release];
	[_deviceUID release];
	[_stream release];
	[super dealloc];
}

@end
