/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

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

#ifndef _SHAZAMINSIGHTS_H_
#define _SHAZAMINSIGHTS_H_

#import <Foundation/Foundation.h>

#import <ShazamInsights/CLLocation+Geohash.h>
#import <ShazamInsights/SHAffinityGroup.h>
#import <ShazamInsights/SHAffinityGroupQuery.h>
#import <ShazamInsights/SHCDNDataFetcher.h>
#import <ShazamInsights/SHClusterController.h>
#import <ShazamInsights/SHClusterImporter.h>
#import <ShazamInsights/SHClusterImporterUtils.h>
#import <ShazamInsights/SHClusterJSONLReader.h>
#import <ShazamInsights/SHClusterLoader.h>
#import <ShazamInsights/SHClusterLoaderRequest.h>
#import <ShazamInsights/SHClusterMetadata.h>
#import <ShazamInsights/SHClusterMetadataQuery.h>
#import <ShazamInsights/SHClusterQuery.h>
#import <ShazamInsights/SHClusterSQLiteDataStore.h>
#import <ShazamInsights/SHClusterStatementRunner.h>
#import <ShazamInsights/SHClusterStatus.h>
#import <ShazamInsights/SHDataFetcher.h>
#import <ShazamInsights/SHDataFetcherFileInfo.h>
#import <ShazamInsights/SHFileChecksum.h>
#import <ShazamInsights/SHInsightsConfiguration.h>
#import <ShazamInsights/SHInsightsError.h>
#import <ShazamInsights/SHInsightsNetworkRequester.h>
#import <ShazamInsights/SHJSONLClusterImporter.h>
#import <ShazamInsights/SHJSONLDataDetokenizerDelegate.h>
#import <ShazamInsights/SHLocalDataFetcher.h>
#import <ShazamInsights/SHMediaItemPropertyCollection.h>
#import <ShazamInsights/SHNetworkRequester.h>
#import <ShazamInsights/SHRegion.h>
#import <ShazamInsights/SHSQLiteClusterImporter.h>
#import <ShazamInsights/SHSQLiteUtils.h>
#import <ShazamInsights/SHStubbedNetworkRequester.h>
#import <ShazamInsights/SHTimeAndPlaceAffinityGroup.h>
#import <ShazamInsights/SHTimeAndPlaceController.h>
#import <ShazamInsights/SHTimeAndPlaceServerResponseParser.h>


void *sh_columnToText(void);
void *sh_databaseErrorToNSError(void);
void *sh_CaseDiacriticInsensitiveCollation(void);


extern extern const void* ShazamInsightsVersionString;
extern extern const void* ShazamInsightsVersionNumber;

#endif

