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

@implementation CKOperationGroup

@synthesize operationGroupID = _operationGroupID;
@synthesize defaultConfiguration = _defaultConfiguration;
@synthesize name = _name;
@synthesize quantity = _quantity;
@synthesize expectedSendSize = _expectedSendSize;
@synthesize expectedReceiveSize = _expectedReceiveSize;

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)init
{
    if ((self = [super init])) {
        // 16 hexadecimal digits.
        _operationGroupID = [NSString stringWithFormat:@"%016llX", (unsigned long long)arc4random() << 32 | arc4random()];
        _defaultConfiguration = [[CKOperationConfiguration alloc] init];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    NSString *groupID = [coder decodeObjectOfClass:[NSString class] forKey:@"OperationGroupID"];
    if (groupID == nil)
        return nil;
    if ((self = [self init])) {
        _operationGroupID = [groupID copy];
        _name = [[coder decodeObjectOfClass:[NSString class] forKey:@"Name"] copy];
        _quantity = (NSUInteger)[coder decodeIntegerForKey:@"Quantity"];
        _expectedSendSize = (CKOperationGroupTransferSize)[coder decodeIntegerForKey:@"ExpectedSendSize"];
        _expectedReceiveSize = (CKOperationGroupTransferSize)[coder decodeIntegerForKey:@"ExpectedReceiveSize"];
        _defaultConfiguration.qualityOfService = (NSQualityOfService)[coder decodeIntegerForKey:@"QualityOfService"];
        _defaultConfiguration.allowsCellularAccess = [coder decodeBoolForKey:@"AllowsCellularAccess"];
        _defaultConfiguration.longLived = [coder decodeBoolForKey:@"LongLived"];
        _defaultConfiguration.timeoutIntervalForRequest = [coder decodeDoubleForKey:@"TimeoutForRequest"];
        _defaultConfiguration.timeoutIntervalForResource = [coder decodeDoubleForKey:@"TimeoutForResource"];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_operationGroupID forKey:@"OperationGroupID"];
    [coder encodeObject:_name forKey:@"Name"];
    [coder encodeInteger:(NSInteger)_quantity forKey:@"Quantity"];
    [coder encodeInteger:_expectedSendSize forKey:@"ExpectedSendSize"];
    [coder encodeInteger:_expectedReceiveSize forKey:@"ExpectedReceiveSize"];
    [coder encodeInteger:_defaultConfiguration.qualityOfService forKey:@"QualityOfService"];
    [coder encodeBool:_defaultConfiguration.allowsCellularAccess forKey:@"AllowsCellularAccess"];
    [coder encodeBool:_defaultConfiguration.longLived forKey:@"LongLived"];
    [coder encodeDouble:_defaultConfiguration.timeoutIntervalForRequest forKey:@"TimeoutForRequest"];
    [coder encodeDouble:_defaultConfiguration.timeoutIntervalForResource forKey:@"TimeoutForResource"];
}
@end
