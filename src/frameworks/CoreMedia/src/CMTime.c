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

#include <CoreMedia/CMTime.h>

const CMTime kCMTimeInvalid = {
    .value = 0,
    .timescale = 0,
    .flags = 0,
    .epoch = 0
};

const CMTime kCMTimeIndefinite = {
    .value = 0,
    .timescale = 0,
    .flags = kCMTimeFlags_Valid | kCMTimeFlags_Indefinite,
    .epoch = 0
};

const CMTime kCMTimePositiveInfinity = {
    .value = 0,
    .timescale = 0,
    .flags = kCMTimeFlags_Valid | kCMTimeFlags_PositiveInfinity,
    .epoch = 0
};

const CMTime kCMTimeNegativeInfinity = {
    .value = 0,
    .timescale = 0,
    .flags = kCMTimeFlags_Valid | kCMTimeFlags_NegativeInfinity,
    .epoch = 0
};

const CMTime kCMTimeZero = {
    .value = 0,
    .timescale = 1,
    .flags = kCMTimeFlags_Valid,
    .epoch = 0
};
