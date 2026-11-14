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

#ifndef _QTTime_H_
#define _QTTime_H_

#include <Foundation/Foundation.h>
#include <QTKit/QTKitDefines.h>

// Rational time: timeValue / timeScale seconds. QTTime is 24 bytes on x86_64
// (long is 64-bit) and 16 bytes on i386.
typedef struct QTTime {
    long long timeValue;
    long timeScale;
    long flags;
} QTTime;

enum {
    kQTTimeIsIndefinite = 1 << 0
};

QTKIT_EXTERN const QTTime QTZeroTime;
QTKIT_EXTERN const QTTime QTIndefiniteTime;

#endif
