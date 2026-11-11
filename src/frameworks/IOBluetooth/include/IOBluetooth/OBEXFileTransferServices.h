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

// Keys of the dictionaries describing folder listing entries and transfer progress.
FOUNDATION_EXPORT NSString * const kFTSListingNameKey;
FOUNDATION_EXPORT NSString * const kFTSListingTypeKey;
FOUNDATION_EXPORT NSString * const kFTSListingSizeKey;
FOUNDATION_EXPORT NSString * const kFTSProgressBytesTransferredKey;
FOUNDATION_EXPORT NSString * const kFTSProgressBytesTotalKey;
FOUNDATION_EXPORT NSString * const kFTSProgressPercentageKey;
FOUNDATION_EXPORT NSString * const kFTSProgressPrecentageKey; // sic, as in Apple's SDK
FOUNDATION_EXPORT NSString * const kFTSProgressEstimatedTimeKey;
FOUNDATION_EXPORT NSString * const kFTSProgressTimeElapsedKey;
FOUNDATION_EXPORT NSString * const kFTSProgressTransferRateKey;

@interface OBEXFileTransferServices : NSObject

@end
