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

#import <Foundation/NSObject.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#include <CoreFoundation/CFBag.h>
#include <FSEvents/FSEvents.h>
#include <dispatch/dispatch.h>

@interface FSEventsImpl : NSObject
{
	NSArray<NSString*>* _pathsToWatch;
	NSDictionary<NSNumber*, NSString*>* _wdMap;

	FSEventStreamCreateFlags _flags;
	FSEventStreamContext _context;
	FSEventStreamCallback _callback;
	dispatch_source_t _source;
	dispatch_queue_t _queue;
	CFMutableBagRef _runloops;
	CFRunLoopSourceRef _rlSource;
	int _fd;
	FSEventStreamEventId _lastEventID;
	bool _running;

	NSMutableArray* _pathArray;
	FSEventStreamEventFlags* _flagArray;
	FSEventStreamEventId* _idArray;
}

-(instancetype)initWithPaths:(NSArray*)pathsToWatch
						flags:(FSEventStreamCreateFlags)flags
					context:(FSEventStreamContext*)context
					callback:(FSEventStreamCallback)callback;
-(NSArray*)copyPathsToWatch;

-(void)setDispatchQueue:(dispatch_queue_t)queue;
-(void)scheduleWithRunLoop:(CFRunLoopRef)rl
					mode:(CFStringRef)rlMode;
-(void)unscheduleWithRunLoop:(CFRunLoopRef)rl
					mode:(CFStringRef)rlMode;
-(void)invalidate;
-(void)start;
-(void)stop;
-(FSEventStreamEventId)lastEventID;

@end

