/*
 This file is part of Darling.

 Copyright (C) 2026 Darling developers

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

#import <BackgroundAssets/BackgroundAssets.h>

// GUESS: the documentation gives the symbols but not their string values; each string is the symbol's own name.
NSString * const BAErrorDomain = @"BAErrorDomain";
NSErrorDomain const BAManagedErrorDomain = @"BAManagedErrorDomain";
NSErrorUserInfoKey const BAAssetPackIdentifierErrorKey = @"BAAssetPackIdentifierErrorKey";
NSErrorUserInfoKey const BAFailuresErrorKey = @"BAFailuresErrorKey";
NSErrorUserInfoKey const BASuccessesErrorKey = @"BASuccessesErrorKey";

// GUESS: the documentation gives no values for the priorities, only that default lies between minimum and maximum.
const BADownloaderPriority BADownloaderPriorityMin = 0;
const BADownloaderPriority BADownloaderPriorityDefault = 1;
const BADownloaderPriority BADownloaderPriorityMax = 2;
