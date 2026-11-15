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

#import "CKInternal.h"

@implementation CKOperation

@synthesize group = _group;
@synthesize operationID = _operationID;
@synthesize longLivedOperationWasPersistedBlock = _longLivedOperationWasPersistedBlock;
@synthesize usesBackgroundSession = _usesBackgroundSession;

- (instancetype)init
{
    if ((self = [super init]))
        _operationID = [NSString stringWithFormat:@"%016llX", (unsigned long long)arc4random() << 32 | arc4random()];
    return self;
}

- (CKOperationConfiguration *)configuration
{
    if (_configuration == nil)
        _configuration = [[CKOperationConfiguration alloc] init];
    return _configuration;
}

- (void)setConfiguration:(CKOperationConfiguration *)configuration
{
    _configuration = [configuration copy];
    [super setQualityOfService:self.configuration.qualityOfService];
}

- (NSQualityOfService)qualityOfService
{
    return self.configuration.qualityOfService;
}

- (void)setQualityOfService:(NSQualityOfService)qualityOfService
{
    self.configuration.qualityOfService = qualityOfService;
    [super setQualityOfService:qualityOfService];
}

- (CKContainer *)container { return self.configuration.container; }
- (void)setContainer:(CKContainer *)container { self.configuration.container = container; }
- (BOOL)allowsCellularAccess { return self.configuration.allowsCellularAccess; }
- (void)setAllowsCellularAccess:(BOOL)allows { self.configuration.allowsCellularAccess = allows; }
- (BOOL)isLongLived { return self.configuration.longLived; }
- (void)setLongLived:(BOOL)longLived { self.configuration.longLived = longLived; }
- (NSTimeInterval)timeoutIntervalForRequest { return self.configuration.timeoutIntervalForRequest; }
- (void)setTimeoutIntervalForRequest:(NSTimeInterval)interval { self.configuration.timeoutIntervalForRequest = interval; }
- (NSTimeInterval)timeoutIntervalForResource { return self.configuration.timeoutIntervalForResource; }
- (void)setTimeoutIntervalForResource:(NSTimeInterval)interval { self.configuration.timeoutIntervalForResource = interval; }

- (BOOL)isAsynchronous
{
    return YES;
}

- (BOOL)isConcurrent
{
    return YES;
}

- (BOOL)isExecuting
{
    return _executing;
}

- (BOOL)isFinished
{
    return _finished;
}

// There is no iCloud account to authenticate a request with, so every operation fails with
// CKErrorNotAuthenticated, or with CKErrorOperationCancelled when it was cancelled before it started.
// The outcome reaches the completion blocks of the subclass through -_performWithError:.
- (void)start
{
    if (_executing || _finished)
        return;
    [self willChangeValueForKey:@"isExecuting"];
    _executing = YES;
    [self didChangeValueForKey:@"isExecuting"];

    NSError *error = [self isCancelled] ? CKCancelledError() : CKNotAuthenticatedError();
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        [self _performWithError:error];
        [self willChangeValueForKey:@"isExecuting"];
        [self willChangeValueForKey:@"isFinished"];
        self->_executing = NO;
        self->_finished = YES;
        [self didChangeValueForKey:@"isExecuting"];
        [self didChangeValueForKey:@"isFinished"];
    });
}

// Subclasses call their completion blocks with no results and the error.
- (void)_performWithError:(NSError *)error
{
}
@end
