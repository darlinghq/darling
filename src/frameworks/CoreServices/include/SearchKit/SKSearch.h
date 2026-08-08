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

#ifndef SKSEARCH_H_
#define SKSEARCH_H_

#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SKSearchType : unsigned int {
    kSKSearchRanked = 0,
    kSKSearchBooleanRanked = 1,
    kSKSearchRequiredRanked = 2,
    kSKSearchPrefixRanked = 3,
} SKSearchType;

typedef enum SKSearchOptions : UInt32 {
    kSKSearchOptionDefault = 0,
    kSKSearchOptionNoRelevanceScores = 1L << 0,
    kSKSearchOptionSpaceMeansOR = 1L << 1,
    kSKSearchOptionFindSimilar = 1L << 2,
} SKSearchOptions;

#ifdef __cplusplus
};
#endif

#endif
