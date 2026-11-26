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

#ifndef QT_TIMELINE_H
#define QT_TIMELINE_H

#include <stdint.h>

/* Movie time state, in movie time units: a time scale, a duration, the
 * active segment [begin, end] and the current position. Private to
 * QuickTime.framework. Every function returns 0 on success and -1 on
 * invalid input, and leaves the timeline unchanged on failure. */
struct qt_timeline {
    int32_t scale;
    int32_t duration;
    int32_t begin;
    int32_t end;
    int32_t position;
};

__attribute__((visibility("hidden")))
int qt_timeline_init(struct qt_timeline *t, int32_t scale, int32_t duration);

/* Convert every time to a new scale, rounding down. Fails when the duration
 * does not fit a TimeValue or when a nonempty duration or active segment
 * would collapse to zero. */
__attribute__((visibility("hidden")))
int qt_timeline_rescale(struct qt_timeline *t, int32_t scale);

#endif
