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

#include "qt_timeline.h"

static int valid(const struct qt_timeline *t)
{
    return t->scale > 0 && t->duration >= 0 && t->begin >= 0 &&
           t->begin <= t->end && t->end <= t->duration &&
           t->position >= t->begin && t->position <= t->end;
}

int qt_timeline_init(struct qt_timeline *t, int32_t scale, int32_t duration)
{
    if (scale <= 0 || duration < 0)
        return -1;
    t->scale = scale;
    t->duration = duration;
    t->begin = 0;
    t->end = duration;
    t->position = 0;
    return 0;
}

/* value * to / from, rounded down; value >= 0 and from, to > 0 so the
 * product fits in 62 bits. */
static uint64_t convert(int32_t value, int32_t from, int32_t to)
{
    return (uint64_t)value * (uint64_t)to / (uint64_t)from;
}

int qt_timeline_rescale(struct qt_timeline *t, int32_t scale)
{
    uint64_t duration, begin, end, position;

    if (scale <= 0 || !valid(t))
        return -1;
    if (scale == t->scale)
        return 0;

    duration = convert(t->duration, t->scale, scale);
    begin = convert(t->begin, t->scale, scale);
    end = convert(t->end, t->scale, scale);
    position = convert(t->position, t->scale, scale);

    /* Rounding down keeps begin <= position <= end <= duration. */
    if (duration > INT32_MAX ||
        (t->duration > 0 && duration == 0) ||
        (t->end > t->begin && begin == end))
        return -1;

    t->scale = scale;
    t->duration = (int32_t)duration;
    t->begin = (int32_t)begin;
    t->end = (int32_t)end;
    t->position = (int32_t)position;
    return 0;
}
