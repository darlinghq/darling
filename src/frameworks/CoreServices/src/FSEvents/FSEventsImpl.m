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

#import "FSEventsImpl.h"
#import <Foundation/NSDictionary.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <darling/emulation/linux_premigration/ext/sys/inotify.h>

static dispatch_queue_t g_fsEventsQueue = NULL;

// Neither the run loop source nor the dispatch source holds a reference
// to the stream; otherwise the stream would own a source that owns the
// stream, and it could never be freed.
static void rlPerform(void* info)
{
	FSEventsImpl* fse = (FSEventsImpl*) info;
	[fse _performFromRunLoop];
}

static void readHandler(void* context)
{
	@autoreleasepool
	{
		FSEventsImpl* fse = (FSEventsImpl*) context;
		[fse _readEvents];
	}
}

static void closeFdHandler(void* context)
{
	close((int) (intptr_t) context);
}

@implementation FSEventsImpl

-(instancetype)initWithPaths:(NSArray*)pathsToWatch
					latency:(CFTimeInterval)latency
					flags:(FSEventStreamCreateFlags)flags
					context:(FSEventStreamContext*)context
					callback:(FSEventStreamCallback)callback
{
	_pathsToWatch = [[NSArray alloc] initWithArray:pathsToWatch];
	_flags = flags;
	_latency = latency;
	_callback = callback;
	_fd = inotify_init1(IN_NONBLOCK);

	if (_fd == -1)
	{
		[self release];
		return nil;
	}

	if (context)
	{
		_context = *context;
		if (_context.retain)
			_context.info = (void*) _context.retain(_context.info);
	}

	CFRunLoopSourceContext rlcontext = {0};
	rlcontext.info = self;
	rlcontext.perform = rlPerform;

	_rlSource = CFRunLoopSourceCreate(NULL, 0, &rlcontext);
	NSMutableDictionary* wdmap = [[NSMutableDictionary alloc] initWithCapacity: [_pathsToWatch count]];

	for (NSString* path in _pathsToWatch)
	{
		int wd = inotify_add_watch(_fd, [path UTF8String], IN_ATTRIB | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE);
		/* Skipped: events for a path that appears later are not reported. */
		if (wd < 0)
			continue;
		[wdmap setObject: path
				forKey: [NSNumber numberWithInt: wd]];
	}
	_wdMap = wdmap;
	_rootWds = [[NSSet alloc] initWithArray: [_wdMap allKeys]];

	// inotify watches are not recursive, but a stream covers the whole
	// hierarchy below each path.
	@autoreleasepool
	{
		for (NSString* path in [_wdMap allValues])
			[self _watchSubdirectoriesOf: path reportContents: NO];
	}

	static dispatch_once_t once;
	dispatch_once(&once, ^{
		g_fsEventsQueue = dispatch_queue_create("FSEvents queue", DISPATCH_QUEUE_SERIAL);
	});

	_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, _fd, 0, g_fsEventsQueue);
	dispatch_set_context(_source, self);
	dispatch_source_set_event_handler_f(_source, readHandler);

	_pathArray = [[NSMutableArray alloc] init];
	_runloops = CFBagCreateMutable(NULL, 0, &kCFTypeBagCallBacks);

	return self;
}

-(void) dealloc
{
	// A started stream and a stream with a dispatch queue hold a reference
	// to themselves, so here the stream is stopped (its dispatch source is
	// suspended and no read handler is running) and has no dispatch queue.
	// A pending run loop delivery also holds a reference.
	if (_rlSource)
	{
		CFRunLoopSourceInvalidate(_rlSource);
		CFRelease(_rlSource);
	}
	if (_runloops)
		CFRelease(_runloops);

	if (_source)
	{
		// The descriptor may only be closed once the source no longer
		// monitors it, which the cancellation handler signals.
		dispatch_set_context(_source, (void*) (intptr_t) _fd);
		dispatch_source_set_cancel_handler_f(_source, closeFdHandler);
		dispatch_source_cancel(_source);
		if (!_running)
			dispatch_resume(_source);
		dispatch_release(_source);
	}
	else if (_fd != -1)
	{
		close(_fd);
	}

	[_pathsToWatch release];
	[_pathArray release];
	[_wdMap release];
	[_rootWds release];
	free(_flagArray);
	free(_idArray);

	if (_context.release)
		_context.release(_context.info);

	[super dealloc];
}

-(NSArray*)copyPathsToWatch
{
	return [_pathsToWatch retain];
}

-(void)setDispatchQueue:(dispatch_queue_t)queue
{
	__block dispatch_queue_t oldQueue;

	if (queue)
		dispatch_retain(queue);

	// _dispatchEvents reads _queue on the FSEvents queue.
	dispatch_sync(g_fsEventsQueue, ^{
		oldQueue = _queue;
		_queue = queue;
	});

	if (oldQueue)
		dispatch_release(oldQueue);

	// QUIRK: a stream that has a dispatch queue is not freed by its last
	// FSEventStreamRelease() until FSEventStreamInvalidate() (or a NULL
	// queue) removes it from the queue. Not documented. Evidence: black-box
	// observation on macOS 27.0.1 (tier 2): after SetDispatchQueue, Start,
	// Stop and Release, the context release callback is not called.
	// Test: tests/src/fsevents_file_events.c, check_quirk_queue_keeps_stream
	if (queue && !oldQueue)
		[self retain];
	else if (!queue && oldQueue)
		[self release];
}

-(void)scheduleWithRunLoop:(CFRunLoopRef)rl
					mode:(CFStringRef)rlMode
{
	@synchronized(self)
	{
		CFBagAddValue(_runloops, rl);
	}
	CFRunLoopAddSource(rl, _rlSource, rlMode);
}

-(void)unscheduleWithRunLoop:(CFRunLoopRef)rl
					mode:(CFStringRef)rlMode
{
	@synchronized(self)
	{
		CFBagRemoveValue(_runloops, rl);
	}
	CFRunLoopRemoveSource(rl, _rlSource, rlMode);
}

-(void)invalidate
{
	// After this, no callback is made, including for events that were
	// already read.
	__atomic_store_n(&_invalidated, true, __ATOMIC_RELEASE);

	CFRunLoopSourceInvalidate(_rlSource);
	[self _dropRunLoopSignal];

	[self retain];
	[self setDispatchQueue: NULL];
	[self release];
}

// Gives up the reference _dispatchEvents took for a run loop delivery.
// Returns whether there was one.
-(BOOL)_dropRunLoopSignal
{
	if (__atomic_exchange_n(&_runLoopSignalled, 0, __ATOMIC_ACQ_REL))
	{
		[self release];
		return YES;
	}
	return NO;
}

-(void)start
{
	if (!_running)
	{
		// QUIRK: a started stream is not freed by its last
		// FSEventStreamRelease(), even after FSEventStreamInvalidate(), and
		// keeps delivering events until FSEventStreamStop(). Not documented.
		// Evidence: black-box observation on macOS 27.0.1 (tier 2): a
		// stream released while started keeps calling its callback and its
		// context release callback is not called.
		// Test: tests/src/fsevents_file_events.c, check_quirk_started_keeps_stream
		[self retain];
		_running = TRUE;
		dispatch_resume(_source);
	}
}

-(void)stop
{
	if (_running)
	{
		dispatch_suspend(_source);
		// Wait for a read handler that is already running.
		dispatch_sync(g_fsEventsQueue, ^{});
		_running = FALSE;
		[self release];
	}
}

-(FSEventStreamEventId)lastEventID
{
	return _lastEventID;
}

-(FSEventStreamEventId)flushPendingEvents
{
	__block FSEventStreamEventId highest = 0;

	// Pick up events the kernel has queued but the read handler has not run for yet.
	dispatch_sync(g_fsEventsQueue, ^{
		[self _readEvents];
		const NSUInteger count = [_pathArray count];
		if (count > 0)
		{
			highest = _idArray[count - 1];
			// _dispatchEvents reads _queue, which is only set on this queue.
			[self _dispatchEvents];
		}
	});

	return highest;
}

-(NSString*)copyStreamDescription
{
	return [[NSString alloc] initWithFormat: @"<FSEventStream %p: paths = %@, latency = %f s, flags = 0x%08x, latestEventId = %llu>",
			self, _pathsToWatch, _latency, (unsigned) _flags, (unsigned long long) _lastEventID];
}

-(void)_readEvents
{
	// One read() returns as many whole inotify_event records as fit in the
	// buffer, each followed by its own variable-length name.
	char buffer[16 * (sizeof(struct inotify_event) + NAME_MAX + 1)]
		__attribute__((aligned(__alignof__(struct inotify_event))));

	while (1)
	{
		ssize_t rd = read(_fd, buffer, sizeof(buffer));
		if (rd < 0)
		{
			if (errno == EINTR)
				continue;
			else if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else
			{
				// The descriptor is unusable; stop watching it for good.
				dispatch_source_cancel(_source);
				break;
			}
		}
		else if (rd == 0)
		{
			break;
		}

		size_t offset = 0;
		while (rd - offset >= sizeof(struct inotify_event))
		{
			struct inotify_event* evt = (struct inotify_event*) (buffer + offset);
			const size_t recordSize = sizeof(struct inotify_event) + evt->len;

			if (recordSize > rd - offset)
				break;

			[self _processSingleEvent: evt];
			offset += recordSize;
		}
	}

	if ([_pathArray count] > 0 && !_deliveryArmed)
		[self _scheduleDelivery];
}

// Events that arrive before the callback runs are delivered together with
// the ones already queued, so the client sees at most one callback per
// latency period.
-(void)_scheduleDelivery
{
	CFTimeInterval delay = _latency;

	// With NoDefer, the first event after a quiet period is delivered at
	// once; later events wait for the latency to pass.
	if ((_flags & kFSEventStreamCreateFlagNoDefer) && CFAbsoluteTimeGetCurrent() - _lastDelivery >= _latency)
		delay = 0;

	_deliveryArmed = true;

	if (delay <= 0)
	{
		[self _dispatchEvents];
	}
	else
	{
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (delay * NSEC_PER_SEC)), g_fsEventsQueue, ^{
			[self _dispatchEvents];
		});
	}
}

-(void)_queueEventForPath:(NSString*)path
					flags:(FSEventStreamEventFlags)flags
{
	[_pathArray addObject: path];
	const int newCount = [_pathArray count];

	_flagArray = (FSEventStreamEventFlags*) realloc(_flagArray, sizeof(*_flagArray) * newCount);
	_flagArray[newCount-1] = flags;

	_idArray = (FSEventStreamEventId*) realloc(_idArray, sizeof(*_idArray) * newCount);
	_lastEventID = _FSEventsAllocateEventID();
	_idArray[newCount-1] = _lastEventID;
}

-(void)_processSingleEvent:(struct inotify_event*)evt
{
	FSEventStreamEventFlags flags = 0;
	NSString* fullPath;

	if (evt->mask & IN_Q_OVERFLOW)
	{
		// The kernel queue overflowed and events were lost (wd is -1 here).
		// Tell the client to rescan everything it watches.
		for (NSNumber* rootWd in _rootWds)
		{
			NSString* rootPath = [_wdMap objectForKey: rootWd];
			if (rootPath)
			{
				[self _queueEventForPath: rootPath
								flags: kFSEventStreamEventFlagMustScanSubDirs | kFSEventStreamEventFlagKernelDropped];
			}
		}
		return;
	}

	NSNumber* wdKey = [NSNumber numberWithInt: evt->wd];
	NSString* watchPath = [_wdMap objectForKey: wdKey];
	if (!watchPath)
		return;

	if (evt->mask & IN_IGNORED)
	{
		// The watch is gone (the item was deleted or unmounted); the change
		// itself was reported by an earlier event.
		[_wdMap removeObjectForKey: wdKey];
		return;
	}

	// A change to a watched subdirectory itself (as opposed to its entries)
	// is also reported, with a name, by the watch on its parent.
	if (evt->len == 0 && ![_rootWds containsObject: wdKey])
		return;

	NSString* itemPath = (evt->len == 0) ? watchPath : [self _pathInDirectory: watchPath name: evt->name];

	if (!(_flags & kFSEventStreamCreateFlagFileEvents))
	{
		fullPath = watchPath;
	}
	else
	{
		if (evt->mask & IN_ISDIR)
			flags |= kFSEventStreamEventFlagItemIsDir;
		else
			flags |= kFSEventStreamEventFlagItemIsFile; // TODO: symlinks
		
		if (evt->mask & (IN_DELETE|IN_DELETE_SELF))
			flags |= kFSEventStreamEventFlagItemRemoved;
		if (evt->mask & (IN_MOVE_SELF|IN_MOVED_FROM|IN_MOVED_TO))
			flags |= kFSEventStreamEventFlagItemRenamed;
		if (evt->mask & (IN_MODIFY))
			flags |= kFSEventStreamEventFlagItemModified;
		if (evt->mask & (IN_CREATE))
			flags |= kFSEventStreamEventFlagItemCreated;
		// IN_ATTRIB covers permissions, ownership, timestamps, link count and
		// extended attributes without saying which one changed.
		if (evt->mask & IN_ATTRIB)
			flags |= kFSEventStreamEventFlagItemInodeMetaMod;

		fullPath = itemPath;
	}

	[self _queueEventForPath: fullPath
					flags: flags];

	if (evt->len != 0 && (evt->mask & IN_ISDIR))
	{
		if (evt->mask & IN_CREATE)
		{
			// Entries may have been created before the watch was added.
			if ([self _addSubdirectoryWatch: itemPath])
				[self _watchSubdirectoriesOf: itemPath reportContents: YES];
		}
		else if (evt->mask & IN_MOVED_TO)
		{
			if ([self _addSubdirectoryWatch: itemPath])
				[self _watchSubdirectoriesOf: itemPath reportContents: NO];
		}
		else if (evt->mask & IN_MOVED_FROM)
		{
			// The watches would otherwise keep reporting under the old path.
			// If the directory was moved within the hierarchy, IN_MOVED_TO
			// adds them again under the new one.
			[self _removeWatchesUnder: itemPath];
		}
	}
}

-(BOOL)_addSubdirectoryWatch:(NSString*)path
{
	int wd = inotify_add_watch(_fd, [path UTF8String], IN_ATTRIB | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE | IN_ONLYDIR | IN_DONT_FOLLOW);
	if (wd < 0)
		return NO;
	[_wdMap setObject: path
			forKey: [NSNumber numberWithInt: wd]];
	return YES;
}

// Adds watches for all directories below `top` that are on the same file
// system. With reportContents, every entry found is also reported as
// created: it appeared after the event that created `top`.
-(void)_watchSubdirectoriesOf:(NSString*)top
			reportContents:(BOOL)reportContents
{
	struct stat st;
	if (lstat([top UTF8String], &st) != 0 || !S_ISDIR(st.st_mode))
		return;

	const dev_t dev = st.st_dev;
	const BOOL fileEvents = (_flags & kFSEventStreamCreateFlagFileEvents) != 0;
	NSMutableArray* pending = [[NSMutableArray alloc] initWithObjects: top, nil];

	while ([pending count] > 0)
	{
		NSString* dir = [[pending lastObject] retain];
		[pending removeLastObject];

		DIR* d = opendir([dir UTF8String]);
		if (d != NULL)
		{
			BOOL reportedDir = NO;
			struct dirent* ent;

			while ((ent = readdir(d)) != NULL)
			{
				if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
					continue;

				NSString* path = [self _pathInDirectory: dir name: ent->d_name];
				int type = ent->d_type;

				if (type == DT_UNKNOWN && lstat([path UTF8String], &st) == 0)
					type = S_ISDIR(st.st_mode) ? DT_DIR : (S_ISLNK(st.st_mode) ? DT_LNK : DT_REG);

				if (reportContents)
				{
					if (fileEvents)
					{
						FSEventStreamEventFlags flags = kFSEventStreamEventFlagItemCreated;
						if (type == DT_DIR)
							flags |= kFSEventStreamEventFlagItemIsDir;
						else if (type == DT_LNK)
							flags |= kFSEventStreamEventFlagItemIsSymlink;
						else
							flags |= kFSEventStreamEventFlagItemIsFile;
						[self _queueEventForPath: path
										flags: flags];
					}
					else if (!reportedDir)
					{
						[self _queueEventForPath: dir
										flags: 0];
						reportedDir = YES;
					}
				}

				if (type == DT_DIR && lstat([path UTF8String], &st) == 0 && st.st_dev == dev)
				{
					if ([self _addSubdirectoryWatch: path])
						[pending addObject: path];
				}
			}
			closedir(d);
		}
		[dir release];
	}

	[pending release];
}

-(void)_removeWatchesUnder:(NSString*)path
{
	NSString* prefix = [path stringByAppendingString: @"/"];

	for (NSNumber* wd in [_wdMap allKeys])
	{
		if ([_rootWds containsObject: wd])
			continue;

		NSString* watchPath = [_wdMap objectForKey: wd];
		if ([watchPath isEqualToString: path] || [watchPath hasPrefix: prefix])
		{
			inotify_rm_watch(_fd, [wd intValue]);
			[_wdMap removeObjectForKey: wd];
		}
	}
}

-(NSString*)_pathInDirectory:(NSString*)dir
						name:(const char*)name
{
	if ([dir hasSuffix: @"/"])
		return [NSString stringWithFormat: @"%@%s", dir, name];
	return [NSString stringWithFormat: @"%@/%s", dir, name];
}

-(void)_dispatchEvents
{
	if (__atomic_load_n(&_invalidated, __ATOMIC_ACQUIRE))
		return;

	if (_queue)
	{
		// The block keeps the stream alive until the callback has run.
		dispatch_async(_queue, ^{
			[self _doCallback];
		});
	}
	else
	{
		// Likewise, a reference for the run loop delivery. It is given up
		// when the source performs, or when the stream is invalidated (an
		// invalidated source never performs).
		if (__atomic_exchange_n(&_runLoopSignalled, 1, __ATOMIC_ACQ_REL) == 0)
			[self retain];
		CFRunLoopSourceSignal(_rlSource);

		@synchronized(self)
		{
			CFBagApplyFunction(_runloops, (CFBagApplierFunction) CFRunLoopWakeUp, NULL);
		}
	}
}

-(void)_performFromRunLoop
{
	[self retain];
	[self _dropRunLoopSignal];
	[self _doCallback];
	[self release];
}

-(void)_doCallback
{
	__block NSMutableArray* pathArray;
	__block FSEventStreamEventFlags* flagArray;
	__block FSEventStreamEventId* idArray;
	char** cpathArray = NULL;
	NSArray<NSDictionary*>* dicts = NULL;

	[self retain];

	// Take the queued events; the read handler queues new ones meanwhile.
	dispatch_sync(g_fsEventsQueue, ^{
		pathArray = _pathArray;
		flagArray = _flagArray;
		idArray = _idArray;
		_pathArray = [[NSMutableArray alloc] init];
		_flagArray = NULL;
		_idArray = NULL;
		_deliveryArmed = false;
		_lastDelivery = CFAbsoluteTimeGetCurrent();
	});

	const NSUInteger count = [pathArray count];
	void* ptrToPass = pathArray;

	if (count > 0 && !__atomic_load_n(&_invalidated, __ATOMIC_ACQUIRE))
	{
		if (!(_flags & kFSEventStreamCreateFlagUseCFTypes))
		{
			@autoreleasepool
			{
				cpathArray = (char**) malloc(sizeof(char*) * count);
				for (NSUInteger i = 0; i < count; i++)
					cpathArray[i] = strdup([[pathArray objectAtIndex: i] UTF8String]);
			}
			ptrToPass = cpathArray;
		}
		else if (_flags & kFSEventStreamCreateFlagUseExtendedData)
		{
			NSDictionary** dd = (NSDictionary**) malloc(sizeof(NSDictionary*) * count);

			for (NSUInteger i = 0; i < count; i++)
			{
				dd[i] = @{
					((NSString*) kFSEventStreamEventExtendedDataPathKey) : [pathArray objectAtIndex: i]
				};
			}

			dicts = [[NSArray alloc] initWithObjects:dd
											count: count];
			free(dd);
			ptrToPass = dicts;
		}

		_callback((ConstFSEventStreamRef) self, _context.info,
			count,
			ptrToPass,
			flagArray, idArray);
	}

	if (cpathArray != NULL)
	{
		for (NSUInteger i = 0; i < count; i++)
			free(cpathArray[i]);
		free(cpathArray);
	}

	if (dicts)
		[dicts release];

	free(flagArray);
	free(idArray);
	[pathArray release];
	[self release];
}

@end
