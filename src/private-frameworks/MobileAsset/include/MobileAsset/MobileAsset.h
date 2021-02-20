/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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


#ifndef _MobileAsset_H_
#define _MobileAsset_H_

#import <Foundation/Foundation.h>

#import <MobileAsset/MAProgressHandler.h>
#import <MobileAsset/MAAsset.h>
#import <MobileAsset/MAAssetQuery.h>
#import <MobileAsset/ASAsset.h>
#import <MobileAsset/ASAssetQuery.h>
#import <MobileAsset/MAXpcManager.h>
#import <MobileAsset/MAAbsoluteAssetId.h>
#import <MobileAsset/MADownloadOptions.h>
#import <MobileAsset/MAMsuDownloadOptions.h>
#import <MobileAsset/MADownloadConfig.h>
#import <MobileAsset/MAProgressNotification.h>
#import <MobileAsset/MAXpcConnection.h>

void* ASEnsureDataVault(void);
void* ASNonUserInitiatedDownloadsAllowed(void);
void* ASServerURLForAssetType(void);
void* ASSetAssetServerURL(void);
void* ASSetAssetServerURLForAssetType(void);
void* ASSetDefaultAssetServerURLForAssetType(void);
void* ASSetLogLevel(void);
void* MACleanV1Repository(void);
void* MAGetPallasAudience(void);
void* MAMigrateAssets(void);
void* MAPurgeAll(void);
void* MASendRepairState(void);
void* MASetPallasAudience(void);
void* MASetPallasUrl(void);
void* MASetServerUrlOverride(void);
void* MobileAssetQueryCreate(void);
void* MobileAssetQueryCreateArrayOfKnownAssets(void);
void* MobileAssetQueryGetMatchingAsset(void);
void* MobileAssetQueryGetTypeID(void);
void* MobileAssetQueryRefreshAssetsAgainstLocalCache(void);
void* MobileAssetQueryRefreshKnownAssets(void);
void* _ForceMetadataUpdate(void);
void* _MACleanV1Repository(void);
void* _MAClientSendRepairState(void);
void* _MAMigrateAssets(void);
void* _MAclientSendCoverageDump(void);
void* _MAclientSendQueryNSUrlState(void);
void* _MAclientSendRepairStateSync(void);
void* _MAclientSendServerUrlOverride(void);
void* _MAensureExtension(void);
void* _MAsendCancelDownload(void);
void* _MAsendConfigDownload(void);
void* _MAsendDownloadAsset(void);
void* _MAsendDownloadMetaData(void);
void* _MAsendDumpClientAccess(void);
void* _MAsendGarbageCollect(void);
void* _MAsendGetPallasAudience(void);
void* _MAsendPurgeAsset(void);
void* _MAsendQueryMetaData(void);
void* _MAsendSetPallasAudience(void);
void* _MAsendSetPallasUrl(void);
void* _MAsendUpdateClientAccess(void);
void* _MobileAssetGarbageCollectionBehaviorForAttributes(void);
void* _MobileAssetLog(void);
void* __DiscardDownloadQueueToken(void);
void* __ForceMigration(void);
void* __RetryQueuedDownloads(void);
void* __StartBackgroundedDownloads(void);
void* __TriggerAssetMigration(void);
void* _getClientCallbackQueue(void);
void* addObjectToMessage(void);

#endif
