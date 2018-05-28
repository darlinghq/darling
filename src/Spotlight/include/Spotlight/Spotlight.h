/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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


#ifndef _Spotlight_H_
#define _Spotlight_H_

#import <Foundation/Foundation.h>

#import <Spotlight/CKDAAPPurchaseHistoryObserver.h>
#import <Spotlight/NSPasteboardWriting.h>
#import <Spotlight/PRSBagFetchSuccessDelegate.h>
#import <Spotlight/PRSBaseResult.h>
#import <Spotlight/PRSGenericResult.h>
#import <Spotlight/PRSMapsResult.h>
#import <Spotlight/PRSPredictionSupport.h>
#import <Spotlight/PRSSearchQueryHandler.h>
#import <Spotlight/PRSSessionController.h>
#import <Spotlight/PRSSimpleResult.h>
#import <Spotlight/QLSeamlessOpenerDelegate.h>
#import <Spotlight/SFFeedbackListener.h>
#import <Spotlight/SPKCoreParsecInterfaceProtocol.h>
#import <Spotlight/SPKQueryDelegate.h>
#import <Spotlight/SPShortcuts.h>
#import <Spotlight/PRSDictionaryResult.h>
#import <Spotlight/SPLogging.h>
#import <Spotlight/SPLoggingSearchSession.h>
#import <Spotlight/SPDefaults.h>
#import <Spotlight/SPAppDefaults.h>
#import <Spotlight/SPSearchQueryContext.h>
#import <Spotlight/SPClientSession.h>
#import <Spotlight/SPKCalculatorQuery.h>
#import <Spotlight/TestingFeedbackListener.h>
#import <Spotlight/QueryController.h>
#import <Spotlight/SPKCoreParsecInterface.h>
#import <Spotlight/PRSPattern.h>
#import <Spotlight/SPStatistics.h>
#import <Spotlight/SPURLResult.h>
#import <Spotlight/SPURLQuery.h>
#import <Spotlight/SPKParsecPattern.h>
#import <Spotlight/SPKParsecQuery.h>
#import <Spotlight/SPWebHistoryResult.h>
#import <Spotlight/SPWebHistoryQuery.h>
#import <Spotlight/SPApplicationQueryResult.h>
#import <Spotlight/SPApplicationQuery.h>
#import <Spotlight/SPFolderQuery.h>
#import <Spotlight/SPMetadataQuery.h>
#import <Spotlight/SPQueryTask.h>
#import <Spotlight/SPKDictionaryQuery.h>
#import <Spotlight/SPCoreSpotlightResult.h>
#import <Spotlight/SPCoreSpotlightQuery.h>
#import <Spotlight/SPMetadataPattern.h>
#import <Spotlight/SPGroupHeadingResult.h>
#import <Spotlight/SPMetadataGroupHeadingResult.h>
#import <Spotlight/SPParsecGroupHeadingResult.h>
#import <Spotlight/PRSResult.h>
#import <Spotlight/SPShortcutPattern.h>
#import <Spotlight/PRSParsecResult.h>
#import <Spotlight/PRSParsecSimpleResult.h>
#import <Spotlight/PRSParsecGenericResult.h>
#import <Spotlight/PRSParsecMapsResult.h>
#import <Spotlight/_SPResultsArrays.h>
#import <Spotlight/PRSCalculatorResult.h>
#import <Spotlight/PRSCalculationResult.h>
#import <Spotlight/PRSConversion.h>
#import <Spotlight/PRSConversionResult.h>
#import <Spotlight/PRSUnitConversionResult.h>
#import <Spotlight/PRSCurrencyConversionResult.h>
#import <Spotlight/SPKResponse.h>
#import <Spotlight/PRSPurchaseHistoryManager.h>
#import <Spotlight/SPKQuery.h>
#import <Spotlight/SPMetadataResult.h>
#import <Spotlight/SPFusedResult.h>
#import <Spotlight/SPAlbumResult.h>

void* SPKStringFromResponseKind(void);
void* SPLogInit(void);
void* SPRoundToNSignificantDigits(void);
void* bootDriveIsFast(void);
void* enumerateQueryResultsUsingBlock(void);
void* mt_log_spotlight_category_orderings(void);
void* mt_log_spotlight_search_session(void);

#endif
