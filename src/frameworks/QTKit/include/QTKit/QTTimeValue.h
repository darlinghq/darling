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
#include <QTKit/QTTime.h>
#include <QTKit/QTTimeRange.h>

@interface QTTimeValue : NSObject

@end

@interface NSValue (NSValueQTTimeExtensions)
+ (NSValue *)valueWithQTTime:(QTTime)time;
- (QTTime)QTTimeValue;
+ (NSValue *)valueWithQTTimeRange:(QTTimeRange)range;
- (QTTimeRange)QTTimeRangeValue;
@end

@interface NSCoder (NSCoderQTTimeExtensions)
- (void)encodeQTTime:(QTTime)time forKey:(NSString *)key;
- (QTTime)decodeQTTimeForKey:(NSString *)key;
- (void)encodeQTTimeRange:(QTTimeRange)range forKey:(NSString *)key;
- (QTTimeRange)decodeQTTimeRangeForKey:(NSString *)key;
@end
