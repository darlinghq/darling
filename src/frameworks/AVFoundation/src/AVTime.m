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

#import <AVFoundation/AVTime.h>

@implementation NSValue (NSValueAVFoundationExtensions)

+ (NSValue *)valueWithCMTime:(CMTime)time
{
    return [NSValue valueWithBytes:&time objCType:@encode(CMTime)];
}

- (CMTime)CMTimeValue
{
    CMTime time;
    [self getValue:&time];
    return time;
}

+ (NSValue *)valueWithCMTimeRange:(CMTimeRange)timeRange
{
    return [NSValue valueWithBytes:&timeRange objCType:@encode(CMTimeRange)];
}

- (CMTimeRange)CMTimeRangeValue
{
    CMTimeRange range;
    [self getValue:&range];
    return range;
}

@end
