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


#include <MobileAsset/MobileAsset.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* ASEnsureDataVault(void)
{
    if (verbose) puts("STUB: ASEnsureDataVault called");
    return NULL;
}

void* ASNonUserInitiatedDownloadsAllowed(void)
{
    if (verbose) puts("STUB: ASNonUserInitiatedDownloadsAllowed called");
    return NULL;
}

void* ASServerURLForAssetType(void)
{
    if (verbose) puts("STUB: ASServerURLForAssetType called");
    return NULL;
}

void* ASSetAssetServerURL(void)
{
    if (verbose) puts("STUB: ASSetAssetServerURL called");
    return NULL;
}

void* ASSetAssetServerURLForAssetType(void)
{
    if (verbose) puts("STUB: ASSetAssetServerURLForAssetType called");
    return NULL;
}

void* ASSetDefaultAssetServerURLForAssetType(void)
{
    if (verbose) puts("STUB: ASSetDefaultAssetServerURLForAssetType called");
    return NULL;
}

void* ASSetLogLevel(void)
{
    if (verbose) puts("STUB: ASSetLogLevel called");
    return NULL;
}

void* MACleanV1Repository(void)
{
    if (verbose) puts("STUB: MACleanV1Repository called");
    return NULL;
}

void* MAGetPallasAudience(void)
{
    if (verbose) puts("STUB: MAGetPallasAudience called");
    return NULL;
}

void* MAMigrateAssets(void)
{
    if (verbose) puts("STUB: MAMigrateAssets called");
    return NULL;
}

void* MAPurgeAll(void)
{
    if (verbose) puts("STUB: MAPurgeAll called");
    return NULL;
}

void* MASendRepairState(void)
{
    if (verbose) puts("STUB: MASendRepairState called");
    return NULL;
}

void* MASetPallasAudience(void)
{
    if (verbose) puts("STUB: MASetPallasAudience called");
    return NULL;
}

void* MASetPallasUrl(void)
{
    if (verbose) puts("STUB: MASetPallasUrl called");
    return NULL;
}

void* MASetServerUrlOverride(void)
{
    if (verbose) puts("STUB: MASetServerUrlOverride called");
    return NULL;
}

void* MobileAssetQueryCreate(void)
{
    if (verbose) puts("STUB: MobileAssetQueryCreate called");
    return NULL;
}

void* MobileAssetQueryCreateArrayOfKnownAssets(void)
{
    if (verbose) puts("STUB: MobileAssetQueryCreateArrayOfKnownAssets called");
    return NULL;
}

void* MobileAssetQueryGetMatchingAsset(void)
{
    if (verbose) puts("STUB: MobileAssetQueryGetMatchingAsset called");
    return NULL;
}

void* MobileAssetQueryGetTypeID(void)
{
    if (verbose) puts("STUB: MobileAssetQueryGetTypeID called");
    return NULL;
}

void* MobileAssetQueryRefreshAssetsAgainstLocalCache(void)
{
    if (verbose) puts("STUB: MobileAssetQueryRefreshAssetsAgainstLocalCache called");
    return NULL;
}

void* MobileAssetQueryRefreshKnownAssets(void)
{
    if (verbose) puts("STUB: MobileAssetQueryRefreshKnownAssets called");
    return NULL;
}

void* _ForceMetadataUpdate(void)
{
    if (verbose) puts("STUB: _ForceMetadataUpdate called");
    return NULL;
}

void* _MACleanV1Repository(void)
{
    if (verbose) puts("STUB: _MACleanV1Repository called");
    return NULL;
}

void* _MAClientSendRepairState(void)
{
    if (verbose) puts("STUB: _MAClientSendRepairState called");
    return NULL;
}

void* _MAMigrateAssets(void)
{
    if (verbose) puts("STUB: _MAMigrateAssets called");
    return NULL;
}

void* _MAclientSendCoverageDump(void)
{
    if (verbose) puts("STUB: _MAclientSendCoverageDump called");
    return NULL;
}

void* _MAclientSendQueryNSUrlState(void)
{
    if (verbose) puts("STUB: _MAclientSendQueryNSUrlState called");
    return NULL;
}

void* _MAclientSendRepairStateSync(void)
{
    if (verbose) puts("STUB: _MAclientSendRepairStateSync called");
    return NULL;
}

void* _MAclientSendServerUrlOverride(void)
{
    if (verbose) puts("STUB: _MAclientSendServerUrlOverride called");
    return NULL;
}

void* _MAensureExtension(void)
{
    if (verbose) puts("STUB: _MAensureExtension called");
    return NULL;
}

void* _MAsendCancelDownload(void)
{
    if (verbose) puts("STUB: _MAsendCancelDownload called");
    return NULL;
}

void* _MAsendConfigDownload(void)
{
    if (verbose) puts("STUB: _MAsendConfigDownload called");
    return NULL;
}

void* _MAsendDownloadAsset(void)
{
    if (verbose) puts("STUB: _MAsendDownloadAsset called");
    return NULL;
}

void* _MAsendDownloadMetaData(void)
{
    if (verbose) puts("STUB: _MAsendDownloadMetaData called");
    return NULL;
}

void* _MAsendDumpClientAccess(void)
{
    if (verbose) puts("STUB: _MAsendDumpClientAccess called");
    return NULL;
}

void* _MAsendGarbageCollect(void)
{
    if (verbose) puts("STUB: _MAsendGarbageCollect called");
    return NULL;
}

void* _MAsendGetPallasAudience(void)
{
    if (verbose) puts("STUB: _MAsendGetPallasAudience called");
    return NULL;
}

void* _MAsendPurgeAsset(void)
{
    if (verbose) puts("STUB: _MAsendPurgeAsset called");
    return NULL;
}

void* _MAsendQueryMetaData(void)
{
    if (verbose) puts("STUB: _MAsendQueryMetaData called");
    return NULL;
}

void* _MAsendSetPallasAudience(void)
{
    if (verbose) puts("STUB: _MAsendSetPallasAudience called");
    return NULL;
}

void* _MAsendSetPallasUrl(void)
{
    if (verbose) puts("STUB: _MAsendSetPallasUrl called");
    return NULL;
}

void* _MAsendUpdateClientAccess(void)
{
    if (verbose) puts("STUB: _MAsendUpdateClientAccess called");
    return NULL;
}

void* _MobileAssetGarbageCollectionBehaviorForAttributes(void)
{
    if (verbose) puts("STUB: _MobileAssetGarbageCollectionBehaviorForAttributes called");
    return NULL;
}

void* _MobileAssetLog(void)
{
    if (verbose) puts("STUB: _MobileAssetLog called");
    return NULL;
}

void* __DiscardDownloadQueueToken(void)
{
    if (verbose) puts("STUB: __DiscardDownloadQueueToken called");
    return NULL;
}

void* __ForceMigration(void)
{
    if (verbose) puts("STUB: __ForceMigration called");
    return NULL;
}

void* __RetryQueuedDownloads(void)
{
    if (verbose) puts("STUB: __RetryQueuedDownloads called");
    return NULL;
}

void* __StartBackgroundedDownloads(void)
{
    if (verbose) puts("STUB: __StartBackgroundedDownloads called");
    return NULL;
}

void* __TriggerAssetMigration(void)
{
    if (verbose) puts("STUB: __TriggerAssetMigration called");
    return NULL;
}

void* _getClientCallbackQueue(void)
{
    if (verbose) puts("STUB: _getClientCallbackQueue called");
    return NULL;
}

void* addObjectToMessage(void)
{
    if (verbose) puts("STUB: addObjectToMessage called");
    return NULL;
}
