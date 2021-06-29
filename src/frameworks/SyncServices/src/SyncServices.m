/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#include <SyncServices/SyncServices.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* ISDArrayAsGlobalIds(void)
{
    if (verbose) puts("STUB: ISDArrayAsGlobalIds called");
    return NULL;
}

void* ISDAsGlobalId(void)
{
    if (verbose) puts("STUB: ISDAsGlobalId called");
    return NULL;
}

void* ISDCleanupExtraLogFiles(void)
{
    if (verbose) puts("STUB: ISDCleanupExtraLogFiles called");
    return NULL;
}

void* ISDCleanupLogFiles(void)
{
    if (verbose) puts("STUB: ISDCleanupLogFiles called");
    return NULL;
}

void* ISDDefaultDoublePrecision(void)
{
    if (verbose) puts("STUB: ISDDefaultDoublePrecision called");
    return NULL;
}

void* ISDEnableAllFacilities(void)
{
    if (verbose) puts("STUB: ISDEnableAllFacilities called");
    return NULL;
}

void* ISDEnableFacility(void)
{
    if (verbose) puts("STUB: ISDEnableFacility called");
    return NULL;
}

void* ISDGetHostUUIDString(void)
{
    if (verbose) puts("STUB: ISDGetHostUUIDString called");
    return NULL;
}

void* ISDInitializeLogging(void)
{
    if (verbose) puts("STUB: ISDInitializeLogging called");
    return NULL;
}

void* ISDIsAppleInternalUser(void)
{
    if (verbose) puts("STUB: ISDIsAppleInternalUser called");
    return NULL;
}

void* ISDMessageTrace(void)
{
    if (verbose) puts("STUB: ISDMessageTrace called");
    return NULL;
}

void* ISDRemoveOrMoveAside(void)
{
    if (verbose) puts("STUB: ISDRemoveOrMoveAside called");
    return NULL;
}

void* ISDRollLogsInDirectory(void)
{
    if (verbose) puts("STUB: ISDRollLogsInDirectory called");
    return NULL;
}

void* ISDSetLogFileDirectory(void)
{
    if (verbose) puts("STUB: ISDSetLogFileDirectory called");
    return NULL;
}

void* ISDStringFromSyncMode(void)
{
    if (verbose) puts("STUB: ISDStringFromSyncMode called");
    return NULL;
}

void* ISDSyncingWithOtherClients(void)
{
    if (verbose) puts("STUB: ISDSyncingWithOtherClients called");
    return NULL;
}

void* _ISDActiveClientCheckIntervalDefault(void)
{
    if (verbose) puts("STUB: _ISDActiveClientCheckIntervalDefault called");
    return NULL;
}

void* _ISDAirbagAnimationInterval(void)
{
    if (verbose) puts("STUB: _ISDAirbagAnimationInterval called");
    return NULL;
}

void* _ISDAirbagDataclassBlacklist(void)
{
    if (verbose) puts("STUB: _ISDAirbagDataclassBlacklist called");
    return NULL;
}

void* _ISDAirbagIsEnabled(void)
{
    if (verbose) puts("STUB: _ISDAirbagIsEnabled called");
    return NULL;
}

void* _ISDAirbagMinimumRecordsInTruthForDataclass(void)
{
    if (verbose) puts("STUB: _ISDAirbagMinimumRecordsInTruthForDataclass called");
    return NULL;
}

void* _ISDAirbagSetAnimationInterval(void)
{
    if (verbose) puts("STUB: _ISDAirbagSetAnimationInterval called");
    return NULL;
}

void* _ISDAirbagSetEnabled(void)
{
    if (verbose) puts("STUB: _ISDAirbagSetEnabled called");
    return NULL;
}

void* _ISDAirbagSetThreshold(void)
{
    if (verbose) puts("STUB: _ISDAirbagSetThreshold called");
    return NULL;
}

void* _ISDAirbagThreshold(void)
{
    if (verbose) puts("STUB: _ISDAirbagThreshold called");
    return NULL;
}

void* _ISDAirbagTimeout(void)
{
    if (verbose) puts("STUB: _ISDAirbagTimeout called");
    return NULL;
}

void* _ISDAllocUUID(void)
{
    if (verbose) puts("STUB: _ISDAllocUUID called");
    return NULL;
}

void* _ISDAllocUUIDString(void)
{
    if (verbose) puts("STUB: _ISDAllocUUIDString called");
    return NULL;
}

void* _ISDAllowMultipleAppClients(void)
{
    if (verbose) puts("STUB: _ISDAllowMultipleAppClients called");
    return NULL;
}

void* _ISDAllowUnresolvedReferences(void)
{
    if (verbose) puts("STUB: _ISDAllowUnresolvedReferences called");
    return NULL;
}

void* _ISDAlwaysShowAirbagIsEnabled(void)
{
    if (verbose) puts("STUB: _ISDAlwaysShowAirbagIsEnabled called");
    return NULL;
}

void* _ISDAlwaysShowAirbagSetEnabled(void)
{
    if (verbose) puts("STUB: _ISDAlwaysShowAirbagSetEnabled called");
    return NULL;
}

void* _ISDAreDataWrappersDisabled(void)
{
    if (verbose) puts("STUB: _ISDAreDataWrappersDisabled called");
    return NULL;
}

void* _ISDArrayOfDictionariesFromQueryCallback(void)
{
    if (verbose) puts("STUB: _ISDArrayOfDictionariesFromQueryCallback called");
    return NULL;
}

void* _ISDBackdoorListenerPort(void)
{
    if (verbose) puts("STUB: _ISDBackdoorListenerPort called");
    return NULL;
}

void* _ISDBrandString(void)
{
    if (verbose) puts("STUB: _ISDBrandString called");
    return NULL;
}

void* _ISDChangeSourceVacuumThreshold(void)
{
    if (verbose) puts("STUB: _ISDChangeSourceVacuumThreshold called");
    return NULL;
}

void* _ISDCleanStringForFileSystem(void)
{
    if (verbose) puts("STUB: _ISDCleanStringForFileSystem called");
    return NULL;
}

void* _ISDCleanupCallHistory(void)
{
    if (verbose) puts("STUB: _ISDCleanupCallHistory called");
    return NULL;
}

void* _ISDCleanupExtraBeehiveBackupFiles(void)
{
    if (verbose) puts("STUB: _ISDCleanupExtraBeehiveBackupFiles called");
    return NULL;
}

void* _ISDClientNameSymlinkPathForClient(void)
{
    if (verbose) puts("STUB: _ISDClientNameSymlinkPathForClient called");
    return NULL;
}

void* _ISDClientReceiveTimeout(void)
{
    if (verbose) puts("STUB: _ISDClientReceiveTimeout called");
    return NULL;
}

void* _ISDClientSendTimeout(void)
{
    if (verbose) puts("STUB: _ISDClientSendTimeout called");
    return NULL;
}

void* _ISDClientSyncAlertHandlerBootstrapName(void)
{
    if (verbose) puts("STUB: _ISDClientSyncAlertHandlerBootstrapName called");
    return NULL;
}

void* _ISDComputeClosureOfEntityNames(void)
{
    if (verbose) puts("STUB: _ISDComputeClosureOfEntityNames called");
    return NULL;
}

void* _ISDComputeConservativeClosureOfEntityNames(void)
{
    if (verbose) puts("STUB: _ISDComputeConservativeClosureOfEntityNames called");
    return NULL;
}

void* _ISDComputeFileSystemNodeSize(void)
{
    if (verbose) puts("STUB: _ISDComputeFileSystemNodeSize called");
    return NULL;
}

void* _ISDConnectRetryCount(void)
{
    if (verbose) puts("STUB: _ISDConnectRetryCount called");
    return NULL;
}

void* _ISDConnectTimeout(void)
{
    if (verbose) puts("STUB: _ISDConnectTimeout called");
    return NULL;
}

void* _ISDCopyUUIDAsData(void)
{
    if (verbose) puts("STUB: _ISDCopyUUIDAsData called");
    return NULL;
}

void* _ISDCopyUUIDAsString(void)
{
    if (verbose) puts("STUB: _ISDCopyUUIDAsString called");
    return NULL;
}

void* _ISDCreateUUID(void)
{
    if (verbose) puts("STUB: _ISDCreateUUID called");
    return NULL;
}

void* _ISDDataDirectoryPathForClient(void)
{
    if (verbose) puts("STUB: _ISDDataDirectoryPathForClient called");
    return NULL;
}

void* _ISDDataWrapperThresholdSize(void)
{
    if (verbose) puts("STUB: _ISDDataWrapperThresholdSize called");
    return NULL;
}

void* _ISDDatabaseAccessStatus(void)
{
    if (verbose) puts("STUB: _ISDDatabaseAccessStatus called");
    return NULL;
}

void* _ISDDefaultDataDirectoryPath(void)
{
    if (verbose) puts("STUB: _ISDDefaultDataDirectoryPath called");
    return NULL;
}

void* _ISDDefaultLogDirectoryPath(void)
{
    if (verbose) puts("STUB: _ISDDefaultLogDirectoryPath called");
    return NULL;
}

void* _ISDDefaultSyncServicesDirectoryPath(void)
{
    if (verbose) puts("STUB: _ISDDefaultSyncServicesDirectoryPath called");
    return NULL;
}

void* _ISDDefaultSyncServicesLogDirectoryPath(void)
{
    if (verbose) puts("STUB: _ISDDefaultSyncServicesLogDirectoryPath called");
    return NULL;
}

void* _ISDDisableUIHelpers(void)
{
    if (verbose) puts("STUB: _ISDDisableUIHelpers called");
    return NULL;
}

void* _ISDDisabledServer(void)
{
    if (verbose) puts("STUB: _ISDDisabledServer called");
    return NULL;
}

void* _ISDDomainString(void)
{
    if (verbose) puts("STUB: _ISDDomainString called");
    return NULL;
}

void* _ISDEnsureDirectoryPathExists(void)
{
    if (verbose) puts("STUB: _ISDEnsureDirectoryPathExists called");
    return NULL;
}

void* _ISDEntityNamesToDataclassNames(void)
{
    if (verbose) puts("STUB: _ISDEntityNamesToDataclassNames called");
    return NULL;
}

void* _ISDFileSystemSafeString(void)
{
    if (verbose) puts("STUB: _ISDFileSystemSafeString called");
    return NULL;
}

void* _ISDFileSystemSafeStringUsingHash(void)
{
    if (verbose) puts("STUB: _ISDFileSystemSafeStringUsingHash called");
    return NULL;
}

void* _ISDFileSystemSafeStringUsingHex(void)
{
    if (verbose) puts("STUB: _ISDFileSystemSafeStringUsingHex called");
    return NULL;
}

void* _ISDGetToplevelDataDirectory(void)
{
    if (verbose) puts("STUB: _ISDGetToplevelDataDirectory called");
    return NULL;
}

void* _ISDGetTruthDatabaseDirectoryForSegmentNamed(void)
{
    if (verbose) puts("STUB: _ISDGetTruthDatabaseDirectoryForSegmentNamed called");
    return NULL;
}

void* _ISDHexDataDirectoryPathForClient(void)
{
    if (verbose) puts("STUB: _ISDHexDataDirectoryPathForClient called");
    return NULL;
}

void* _ISDHomeDirectoryIsOnSharedNetworkDrive(void)
{
    if (verbose) puts("STUB: _ISDHomeDirectoryIsOnSharedNetworkDrive called");
    return NULL;
}

void* _ISDInitializeLogging(void)
{
    if (verbose) puts("STUB: _ISDInitializeLogging called");
    return NULL;
}

void* _ISDIsContactSyncingAllowed(void)
{
    if (verbose) puts("STUB: _ISDIsContactSyncingAllowed called");
    return NULL;
}

void* _ISDIsDatabaseAtPathCorrupted(void)
{
    if (verbose) puts("STUB: _ISDIsDatabaseAtPathCorrupted called");
    return NULL;
}

void* _ISDIsSyncingEnabled(void)
{
    if (verbose) puts("STUB: _ISDIsSyncingEnabled called");
    return NULL;
}

void* _ISDLimitAlwaysShowAirbagDataclasses(void)
{
    if (verbose) puts("STUB: _ISDLimitAlwaysShowAirbagDataclasses called");
    return NULL;
}

void* _ISDLog(void)
{
    if (verbose) puts("STUB: _ISDLog called");
    return NULL;
}

void* _ISDLogDirectoryPathForClient(void)
{
    if (verbose) puts("STUB: _ISDLogDirectoryPathForClient called");
    return NULL;
}

void* _ISDLogLevelToString(void)
{
    if (verbose) puts("STUB: _ISDLogLevelToString called");
    return NULL;
}

void* _ISDLogSql(void)
{
    if (verbose) puts("STUB: _ISDLogSql called");
    return NULL;
}

void* _ISDMaxSessionCreationTime(void)
{
    if (verbose) puts("STUB: _ISDMaxSessionCreationTime called");
    return NULL;
}

void* _ISDMessageTrace(void)
{
    if (verbose) puts("STUB: _ISDMessageTrace called");
    return NULL;
}

void* _ISDNewActiveClientCheckDate(void)
{
    if (verbose) puts("STUB: _ISDNewActiveClientCheckDate called");
    return NULL;
}

void* _ISDNewPurgeAndVacuumDate(void)
{
    if (verbose) puts("STUB: _ISDNewPurgeAndVacuumDate called");
    return NULL;
}

void* _ISDNumberOfBeehiveBackupFilesToKeep(void)
{
    if (verbose) puts("STUB: _ISDNumberOfBeehiveBackupFilesToKeep called");
    return NULL;
}

void* _ISDNumberOfLogFiles(void)
{
    if (verbose) puts("STUB: _ISDNumberOfLogFiles called");
    return NULL;
}

void* _ISDPotentialLogFileNames(void)
{
    if (verbose) puts("STUB: _ISDPotentialLogFileNames called");
    return NULL;
}

void* _ISDPruneFilePath(void)
{
    if (verbose) puts("STUB: _ISDPruneFilePath called");
    return NULL;
}

void* _ISDPurgeAndVacuumIntervalDefault(void)
{
    if (verbose) puts("STUB: _ISDPurgeAndVacuumIntervalDefault called");
    return NULL;
}

void* _ISDSPathForSQLiteDatabase(void)
{
    if (verbose) puts("STUB: _ISDSPathForSQLiteDatabase called");
    return NULL;
}

void* _ISDSQLiteBlowOut(void)
{
    if (verbose) puts("STUB: _ISDSQLiteBlowOut called");
    return NULL;
}

void* _ISDSQLiteTraceSQL(void)
{
    if (verbose) puts("STUB: _ISDSQLiteTraceSQL called");
    return NULL;
}

void* _ISDSQliteFullIOErrCorruptionHandler(void)
{
    if (verbose) puts("STUB: _ISDSQliteFullIOErrCorruptionHandler called");
    return NULL;
}

void* _ISDSaveClientDataBeforeMingler(void)
{
    if (verbose) puts("STUB: _ISDSaveClientDataBeforeMingler called");
    return NULL;
}

void* _ISDSaveDataDirectorySnapshot(void)
{
    if (verbose) puts("STUB: _ISDSaveDataDirectorySnapshot called");
    return NULL;
}

void* _ISDSaveDirectoryPathInDirectory(void)
{
    if (verbose) puts("STUB: _ISDSaveDirectoryPathInDirectory called");
    return NULL;
}

void* _ISDSaveSyncServicesDirectoryForException(void)
{
    if (verbose) puts("STUB: _ISDSaveSyncServicesDirectoryForException called");
    return NULL;
}

void* _ISDServerBootstrapName(void)
{
    if (verbose) puts("STUB: _ISDServerBootstrapName called");
    return NULL;
}

void* _ISDServerPath(void)
{
    if (verbose) puts("STUB: _ISDServerPath called");
    return NULL;
}

void* _ISDSetActiveClientCheckIntervalDefault(void)
{
    if (verbose) puts("STUB: _ISDSetActiveClientCheckIntervalDefault called");
    return NULL;
}

void* _ISDSetAllowUnresolvedReferences(void)
{
    if (verbose) puts("STUB: _ISDSetAllowUnresolvedReferences called");
    return NULL;
}

void* _ISDSetAreDataWrappersDisabled(void)
{
    if (verbose) puts("STUB: _ISDSetAreDataWrappersDisabled called");
    return NULL;
}

void* _ISDSetDisabledServer(void)
{
    if (verbose) puts("STUB: _ISDSetDisabledServer called");
    return NULL;
}

void* _ISDSetNumberOfLogFiles(void)
{
    if (verbose) puts("STUB: _ISDSetNumberOfLogFiles called");
    return NULL;
}

void* _ISDSetShouldCompressClientState(void)
{
    if (verbose) puts("STUB: _ISDSetShouldCompressClientState called");
    return NULL;
}

void* _ISDSetShouldEnableBackdoorListener(void)
{
    if (verbose) puts("STUB: _ISDSetShouldEnableBackdoorListener called");
    return NULL;
}

void* _ISDSetShouldEnableDebugMenu(void)
{
    if (verbose) puts("STUB: _ISDSetShouldEnableDebugMenu called");
    return NULL;
}

void* _ISDSetShouldEnableGrowlNotifications(void)
{
    if (verbose) puts("STUB: _ISDSetShouldEnableGrowlNotifications called");
    return NULL;
}

void* _ISDSetShouldExcludeSyncingPreferences(void)
{
    if (verbose) puts("STUB: _ISDSetShouldExcludeSyncingPreferences called");
    return NULL;
}

void* _ISDSetShouldSaveAdditionalInfoWhenMingleFails(void)
{
    if (verbose) puts("STUB: _ISDSetShouldSaveAdditionalInfoWhenMingleFails called");
    return NULL;
}

void* _ISDSetShouldSaveCallHistory(void)
{
    if (verbose) puts("STUB: _ISDSetShouldSaveCallHistory called");
    return NULL;
}

void* _ISDSetShouldSaveCallHistoryForEveryCall(void)
{
    if (verbose) puts("STUB: _ISDSetShouldSaveCallHistoryForEveryCall called");
    return NULL;
}

void* _ISDSetShouldSaveClientState(void)
{
    if (verbose) puts("STUB: _ISDSetShouldSaveClientState called");
    return NULL;
}

void* _ISDSetShouldSavePushedChanges(void)
{
    if (verbose) puts("STUB: _ISDSetShouldSavePushedChanges called");
    return NULL;
}

void* _ISDSetShouldUseSmartExceptionsInSyncServer(void)
{
    if (verbose) puts("STUB: _ISDSetShouldUseSmartExceptionsInSyncServer called");
    return NULL;
}

void* _ISDSetShowAnchorMismatchDialog(void)
{
    if (verbose) puts("STUB: _ISDSetShowAnchorMismatchDialog called");
    return NULL;
}

void* _ISDSetShowDetailOnLoad(void)
{
    if (verbose) puts("STUB: _ISDSetShowDetailOnLoad called");
    return NULL;
}

void* _ISDSetSyncingEnabled(void)
{
    if (verbose) puts("STUB: _ISDSetSyncingEnabled called");
    return NULL;
}

void* _ISDShouldCleanupAllCallHistory(void)
{
    if (verbose) puts("STUB: _ISDShouldCleanupAllCallHistory called");
    return NULL;
}

void* _ISDShouldCompressClientState(void)
{
    if (verbose) puts("STUB: _ISDShouldCompressClientState called");
    return NULL;
}

void* _ISDShouldDisableUIHelpers(void)
{
    if (verbose) puts("STUB: _ISDShouldDisableUIHelpers called");
    return NULL;
}

void* _ISDShouldEnableBackdoorListener(void)
{
    if (verbose) puts("STUB: _ISDShouldEnableBackdoorListener called");
    return NULL;
}

void* _ISDShouldEnableDebugMenu(void)
{
    if (verbose) puts("STUB: _ISDShouldEnableDebugMenu called");
    return NULL;
}

void* _ISDShouldEnableGrowlNotifications(void)
{
    if (verbose) puts("STUB: _ISDShouldEnableGrowlNotifications called");
    return NULL;
}

void* _ISDShouldExcludeSyncingPreferences(void)
{
    if (verbose) puts("STUB: _ISDShouldExcludeSyncingPreferences called");
    return NULL;
}

void* _ISDShouldLog(void)
{
    if (verbose) puts("STUB: _ISDShouldLog called");
    return NULL;
}

void* _ISDShouldLogEverything(void)
{
    if (verbose) puts("STUB: _ISDShouldLogEverything called");
    return NULL;
}

void* _ISDShouldLogKey(void)
{
    if (verbose) puts("STUB: _ISDShouldLogKey called");
    return NULL;
}

void* _ISDShouldLogPersonalInfo(void)
{
    if (verbose) puts("STUB: _ISDShouldLogPersonalInfo called");
    return NULL;
}

void* _ISDShouldRequireLowercaseTypes(void)
{
    if (verbose) puts("STUB: _ISDShouldRequireLowercaseTypes called");
    return NULL;
}

void* _ISDShouldSaveAdditionalInfoWhenMingleFails(void)
{
    if (verbose) puts("STUB: _ISDShouldSaveAdditionalInfoWhenMingleFails called");
    return NULL;
}

void* _ISDShouldSaveCallHistory(void)
{
    if (verbose) puts("STUB: _ISDShouldSaveCallHistory called");
    return NULL;
}

void* _ISDShouldSaveCallHistoryForEveryCall(void)
{
    if (verbose) puts("STUB: _ISDShouldSaveCallHistoryForEveryCall called");
    return NULL;
}

void* _ISDShouldSaveClientState(void)
{
    if (verbose) puts("STUB: _ISDShouldSaveClientState called");
    return NULL;
}

void* _ISDShouldSavePushedChanges(void)
{
    if (verbose) puts("STUB: _ISDShouldSavePushedChanges called");
    return NULL;
}

void* _ISDShouldUseSmartExceptionsInSyncServer(void)
{
    if (verbose) puts("STUB: _ISDShouldUseSmartExceptionsInSyncServer called");
    return NULL;
}

void* _ISDShouldValidateClients(void)
{
    if (verbose) puts("STUB: _ISDShouldValidateClients called");
    return NULL;
}

void* _ISDShouldValidatePushedChangesAndRecords(void)
{
    if (verbose) puts("STUB: _ISDShouldValidatePushedChangesAndRecords called");
    return NULL;
}

void* _ISDShouldValidateTypesInSchemas(void)
{
    if (verbose) puts("STUB: _ISDShouldValidateTypesInSchemas called");
    return NULL;
}

void* _ISDShowAnchorMismatchDialog(void)
{
    if (verbose) puts("STUB: _ISDShowAnchorMismatchDialog called");
    return NULL;
}

void* _ISDShowDetailOnLoad(void)
{
    if (verbose) puts("STUB: _ISDShowDetailOnLoad called");
    return NULL;
}

void* _ISDSqliteCacheSize(void)
{
    if (verbose) puts("STUB: _ISDSqliteCacheSize called");
    return NULL;
}

void* _ISDStringToLogLevel(void)
{
    if (verbose) puts("STUB: _ISDStringToLogLevel called");
    return NULL;
}

void* _ISDSyncAlertReceiveTimeout(void)
{
    if (verbose) puts("STUB: _ISDSyncAlertReceiveTimeout called");
    return NULL;
}

void* _ISDSyncAlertSendTimeout(void)
{
    if (verbose) puts("STUB: _ISDSyncAlertSendTimeout called");
    return NULL;
}

void* _ISDSyncDirectoryFileSystemSupportsHardLinks(void)
{
    if (verbose) puts("STUB: _ISDSyncDirectoryFileSystemSupportsHardLinks called");
    return NULL;
}

void* _ISDSyncUIDaemonReceiveTimeout(void)
{
    if (verbose) puts("STUB: _ISDSyncUIDaemonReceiveTimeout called");
    return NULL;
}

void* _ISDSyncUIDaemonSendTimeout(void)
{
    if (verbose) puts("STUB: _ISDSyncUIDaemonSendTimeout called");
    return NULL;
}

void* _ISDSyncingEntitiesAllowed(void)
{
    if (verbose) puts("STUB: _ISDSyncingEntitiesAllowed called");
    return NULL;
}

void* _ISDSyncsAfterHysteresis(void)
{
    if (verbose) puts("STUB: _ISDSyncsAfterHysteresis called");
    return NULL;
}

void* _ISDUseExtendedFiltering(void)
{
    if (verbose) puts("STUB: _ISDUseExtendedFiltering called");
    return NULL;
}

void* _ISDVLog(void)
{
    if (verbose) puts("STUB: _ISDVLog called");
    return NULL;
}

void* _ISD_UTNameNumberSetup(void)
{
    if (verbose) puts("STUB: _ISD_UTNameNumberSetup called");
    return NULL;
}

void* _ISD_UTNameNumberTearDown(void)
{
    if (verbose) puts("STUB: _ISD_UTNameNumberTearDown called");
    return NULL;
}

void* _ISDfileIsLocked(void)
{
    if (verbose) puts("STUB: _ISDfileIsLocked called");
    return NULL;
}

void* _ISyncAssertFailed(void)
{
    if (verbose) puts("STUB: _ISyncAssertFailed called");
    return NULL;
}

void* _ISyncPreconditionFailed(void)
{
    if (verbose) puts("STUB: _ISyncPreconditionFailed called");
    return NULL;
}

void* _ISyncPrivateInstance(void)
{
    if (verbose) puts("STUB: _ISyncPrivateInstance called");
    return NULL;
}

void* _WarnAboutUsingISyncDataClassImageKey(void)
{
    if (verbose) puts("STUB: _WarnAboutUsingISyncDataClassImageKey called");
    return NULL;
}

void* __ISDDefaultSyncServicesDirectoryPath(void)
{
    if (verbose) puts("STUB: __ISDDefaultSyncServicesDirectoryPath called");
    return NULL;
}

void* __ISDDefaultSyncServicesLogDirectoryPath(void)
{
    if (verbose) puts("STUB: __ISDDefaultSyncServicesLogDirectoryPath called");
    return NULL;
}

void* __ISyncThouShallNotSubclass(void)
{
    if (verbose) puts("STUB: __ISyncThouShallNotSubclass called");
    return NULL;
}

void* _dataclassNamesForEntities(void)
{
    if (verbose) puts("STUB: _dataclassNamesForEntities called");
    return NULL;
}

void* _executeSQL(void)
{
    if (verbose) puts("STUB: _executeSQL called");
    return NULL;
}

void* _executeStatement(void)
{
    if (verbose) puts("STUB: _executeStatement called");
    return NULL;
}

void* _insertIdReferences(void)
{
    if (verbose) puts("STUB: _insertIdReferences called");
    return NULL;
}

void* _isRunningInServer(void)
{
    if (verbose) puts("STUB: _isRunningInServer called");
    return NULL;
}

void* _setIsRunningInServer(void)
{
    if (verbose) puts("STUB: _setIsRunningInServer called");
    return NULL;
}

void* beginExclusiveTransaction(void)
{
    if (verbose) puts("STUB: beginExclusiveTransaction called");
    return NULL;
}

void* beginImmediateTransaction(void)
{
    if (verbose) puts("STUB: beginImmediateTransaction called");
    return NULL;
}

void* beginTransaction(void)
{
    if (verbose) puts("STUB: beginTransaction called");
    return NULL;
}

void* checkin_mach_service(void)
{
    if (verbose) puts("STUB: checkin_mach_service called");
    return NULL;
}

void* commitTransaction(void)
{
    if (verbose) puts("STUB: commitTransaction called");
    return NULL;
}

void* conflictsCount(void)
{
    if (verbose) puts("STUB: conflictsCount called");
    return NULL;
}

void* createCalendarRequestPort(void)
{
    if (verbose) puts("STUB: createCalendarRequestPort called");
    return NULL;
}

void* databaseGotCorrupted(void)
{
    if (verbose) puts("STUB: databaseGotCorrupted called");
    return NULL;
}

void* databasePath(void)
{
    if (verbose) puts("STUB: databasePath called");
    return NULL;
}

void* getColumnDescriptionWithSql(void)
{
    if (verbose) puts("STUB: getColumnDescriptionWithSql called");
    return NULL;
}

void* googleFound(void)
{
    if (verbose) puts("STUB: googleFound called");
    return NULL;
}

void* integrityResult(void)
{
    if (verbose) puts("STUB: integrityResult called");
    return NULL;
}

void* isd_addRecordIdToArray(void)
{
    if (verbose) puts("STUB: isd_addRecordIdToArray called");
    return NULL;
}

void* isd_clientAsNumber(void)
{
    if (verbose) puts("STUB: isd_clientAsNumber called");
    return NULL;
}

void* isd_clientName(void)
{
    if (verbose) puts("STUB: isd_clientName called");
    return NULL;
}

void* isd_clientNumber(void)
{
    if (verbose) puts("STUB: isd_clientNumber called");
    return NULL;
}

void* isd_entityAsNumber(void)
{
    if (verbose) puts("STUB: isd_entityAsNumber called");
    return NULL;
}

void* isd_entityName(void)
{
    if (verbose) puts("STUB: isd_entityName called");
    return NULL;
}

void* isd_entityNumber(void)
{
    if (verbose) puts("STUB: isd_entityNumber called");
    return NULL;
}

void* isd_mapArray(void)
{
    if (verbose) puts("STUB: isd_mapArray called");
    return NULL;
}

void* rollbackTransaction(void)
{
    if (verbose) puts("STUB: rollbackTransaction called");
    return NULL;
}

void* sqliteExecute(void)
{
    if (verbose) puts("STUB: sqliteExecute called");
    return NULL;
}

void* sqliteExecuteWithCallback(void)
{
    if (verbose) puts("STUB: sqliteExecuteWithCallback called");
    return NULL;
}

void* sqliteExecuteWithCallbackAndOptions(void)
{
    if (verbose) puts("STUB: sqliteExecuteWithCallbackAndOptions called");
    return NULL;
}

void* syncServerPreference(void)
{
    if (verbose) puts("STUB: syncServerPreference called");
    return NULL;
}
