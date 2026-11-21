/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _QTTimeRange_H_
#define _QTTimeRange_H_

#import <Foundation/Foundation.h>
#import <QTKit/QTKitDefines.h>
#import <QTKit/QTTime.h>

typedef struct QTTimeRange {
    QTTime time;
    QTTime duration;
} QTTimeRange;

QTKIT_EXTERN QTTimeRange QTMakeTimeRange(QTTime time, QTTime duration);
QTKIT_EXTERN QTTime QTTimeRangeEnd(QTTimeRange range);
QTKIT_EXTERN BOOL QTTimeInTimeRange(QTTime time, QTTimeRange range);
QTKIT_EXTERN BOOL QTEqualTimeRanges(QTTimeRange range1, QTTimeRange range2);
QTKIT_EXTERN QTTimeRange QTIntersectionTimeRange(QTTimeRange range1, QTTimeRange range2);
QTKIT_EXTERN QTTimeRange QTUnionTimeRange(QTTimeRange range1, QTTimeRange range2);
QTKIT_EXTERN NSString *QTStringFromTimeRange(QTTimeRange range);
QTKIT_EXTERN QTTimeRange QTTimeRangeFromString(NSString *string);

#endif
