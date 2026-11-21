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

#ifndef QTCMTime_h
#define QTCMTime_h

// Conversions between QTKit's time and CoreMedia's, for the QTKit classes that sit on AVFoundation.
// Not part of QTKit's interface.

#import <QTKit/QTTime.h>
#import <QTKit/QTTimeRange.h>
#import <CoreMedia/CMTime.h>
#import <CoreMedia/CMTimeRange.h>

static inline QTTime QTTimeFromCMTime(CMTime time)
{
    if (!(time.flags & kCMTimeFlags_Valid) || (time.flags & (kCMTimeFlags_Indefinite | kCMTimeFlags_PositiveInfinity | kCMTimeFlags_NegativeInfinity)))
        return QTIndefiniteTime;
    return QTMakeTime(time.value, (long)time.timescale);
}

static inline CMTime CMTimeFromQTTime(QTTime time)
{
    if ((time.flags & kQTTimeIsIndefinite) || time.timeScale <= 0 || time.timeScale > INT32_MAX)
        return kCMTimeIndefinite;
    return CMTimeMake(time.timeValue, (int32_t)time.timeScale);
}

#endif
