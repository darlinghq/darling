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

@implementation CKOperationConfiguration

@synthesize container = _container;
@synthesize qualityOfService = _qualityOfService;
@synthesize allowsCellularAccess = _allowsCellularAccess;
@synthesize longLived = _longLived;
@synthesize timeoutIntervalForRequest = _timeoutIntervalForRequest;
@synthesize timeoutIntervalForResource = _timeoutIntervalForResource;

- (instancetype)init
{
    if ((self = [super init])) {
        _qualityOfService = NSQualityOfServiceDefault;
        _allowsCellularAccess = YES;
        _timeoutIntervalForRequest = 60;
        _timeoutIntervalForResource = -1;
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    CKOperationConfiguration *copy = [[[self class] allocWithZone:zone] init];
    copy->_container = _container;
    copy->_qualityOfService = _qualityOfService;
    copy->_allowsCellularAccess = _allowsCellularAccess;
    copy->_longLived = _longLived;
    copy->_timeoutIntervalForRequest = _timeoutIntervalForRequest;
    copy->_timeoutIntervalForResource = _timeoutIntervalForResource;
    return copy;
}
@end
